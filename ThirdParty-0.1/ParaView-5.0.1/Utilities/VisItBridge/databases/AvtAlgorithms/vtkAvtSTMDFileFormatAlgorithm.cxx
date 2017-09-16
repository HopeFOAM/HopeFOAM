/*=========================================================================

   Program: ParaView
   Module:    vtkAvtSTMDFileFormatAlgorithm.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "vtkAvtSTMDFileFormatAlgorithm.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkDataArraySelection.h"

#include "vtkAMRBox.h"
#include "vtkAMRInformation.h"
#include "vtkOverlappingAMR.h"
#include "vtkHierarchicalBoxDataSet.h"
#include "vtkParallelAMRUtilities.h"
#include "vtkMultiProcessController.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"
#include "vtkUnstructuredGrid.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"

#include "vtkCompositeDataPipeline.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkUnstructuredGridRelevantPointsFilter.h"
#include "vtkCleanPolyData.h"
#include "vtkCSGGrid.h"

#include "avtMeshMetaData.h"
#include "avtSTMDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"
#include <set>

struct vtkAvtSTMDFileFormatAlgorithm::vtkAvtSTMDFileFormatAlgorithmInternal
{
  unsigned int MinDataset;
  unsigned int MaxDataset;
  bool HasUpdateRestriction;
  std::set<int> UpdateIndices;
  vtkAvtSTMDFileFormatAlgorithmInternal():
    MinDataset(0),
    MaxDataset(0),
    HasUpdateRestriction(false),
    UpdateIndices()
    {}
};

vtkStandardNewMacro(vtkAvtSTMDFileFormatAlgorithm);

//-----------------------------------------------------------------------------
vtkAvtSTMDFileFormatAlgorithm::vtkAvtSTMDFileFormatAlgorithm()
{
  this->UpdatePiece = 0;
  this->UpdateNumPieces = 0;
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
  this->Internal =
    new vtkAvtSTMDFileFormatAlgorithm::vtkAvtSTMDFileFormatAlgorithmInternal();
}

//-----------------------------------------------------------------------------
vtkAvtSTMDFileFormatAlgorithm::~vtkAvtSTMDFileFormatAlgorithm()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::RequestDataObject(vtkInformation *,
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }


  int size = this->MetaData->GetNumMeshes();
  if ( size <= 0 )
    {
    vtkErrorMacro("Unable to find any meshes");
    return 0;
    }

  //determine if this is an AMR mesh
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
  const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
  if ( size == 1 &&  meshMetaData.meshType == AVT_AMR_MESH)
    {
    //verify the mesh is correct
    if ( this->ValidAMR( &meshMetaData ) )
      {
      this->OutputType = VTK_OVERLAPPING_AMR;
      }
    }

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkCompositeDataSet *output = vtkCompositeDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));

  if ( output && output->GetDataObjectType() == this->OutputType )
    {
    return 1;
    }
  else if ( !output || output->GetDataObjectType() != this->OutputType )
    {
    switch( this->OutputType )
      {
      case VTK_OVERLAPPING_AMR:
        output = vtkOverlappingAMR::New();
        break;
      case VTK_HIERARCHICAL_BOX_DATA_SET:
        output = vtkHierarchicalBoxDataSet::New();
        break;
      case VTK_MULTIBLOCK_DATA_SET:
      default:
        output = vtkMultiBlockDataSet::New();
        break;
      }
    info->Set(vtkDataObject::DATA_OBJECT(), output);
    output->Delete();
    }

  return 1;
  }

//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }
  this->CreateAVTDataSelections();


  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  this->UpdatePiece =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  this->UpdateNumPieces =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

  if ( this->OutputType == VTK_HIERARCHICAL_BOX_DATA_SET
    && this->MeshArraySelection
    && this->MeshArraySelection->GetNumberOfArraysEnabled() == 1)
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
    vtkHierarchicalBoxDataSet *output = vtkHierarchicalBoxDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    this->FillAMR( output, &meshMetaData, 0, 0 );
    }

  else if( this->OutputType == VTK_OVERLAPPING_AMR &&
           this->MeshArraySelection &&
           this->MeshArraySelection->GetNumberOfArraysEnabled()==1 )
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( 0 );
    vtkOverlappingAMR *output = vtkOverlappingAMR::
       SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    this->FillAMR( output, &meshMetaData, 0, 0 );
    }

  else if ( this->OutputType == VTK_MULTIBLOCK_DATA_SET )
    {
    vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::
      SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    int size = this->MetaData->GetNumMeshes();
    if ( this->MeshArraySelection )
      {
      //we don't want NULL blocks to be displayed, so get the
      //actual number of meshes the user wants
      size = this->MeshArraySelection->GetNumberOfArraysEnabled();
      }
    output->SetNumberOfBlocks( size );

    vtkMultiBlockDataSet* tempData = NULL;
    int blockIndex=0;
    for ( int i=0; i < this->MetaData->GetNumMeshes(); ++i)
      {
      const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( i );
      std::string name = meshMetaData.name;

      //before we get the mesh see if the user wanted to load this mesh
      if (this->MeshArraySelection &&
        !this->MeshArraySelection->ArrayIsEnabled( name.c_str() ) )
        {
        continue;
        }

      switch(meshMetaData.meshType)
        {
        case AVT_CSG_MESH:
        case AVT_AMR_MESH:
        case AVT_RECTILINEAR_MESH:
        case AVT_CURVILINEAR_MESH:
        case AVT_UNSTRUCTURED_MESH:
        case AVT_POINT_MESH:
        case AVT_SURFACE_MESH:
        default:
          tempData = vtkMultiBlockDataSet::New();
          this->FillBlock( tempData, &meshMetaData, 0 );
          output->SetBlock(blockIndex,tempData);
          tempData->Delete();
          tempData = NULL;
          break;
        }
      output->GetMetaData(blockIndex)->Set(vtkCompositeDataSet::NAME(),name.c_str());
      ++blockIndex;
      }
    }

  this->CleanupAVTReader();
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAvtSTMDFileFormatAlgorithm::FillAMR(
  vtkOverlappingAMR *outputAMR, const avtMeshMetaData *meshMetaData,
  const int &timestep, const int &domain)
{
  //we first need to determine if this AMR can be safely converted to a
  //ParaView AMR. What this means is that every dataset needs to have regular spacing
  bool valid  = this->ValidAMR( meshMetaData );
  if ( !valid )
    {
    return 0;
    }

  vtkOverlappingAMR *ghostedAMR = vtkOverlappingAMR::New();

  this->GetDomainRange(meshMetaData);

  //number of levels in the AMR
  int numGroups = meshMetaData->numGroups;

  //TODO: if the cache doesn't have the results we can ask the file format itself
  //determine the ratio for each level
  void_ref_ptr vr = this->Cache->GetVoidRef(meshMetaData->name.c_str(),
                    AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                    0, -1);
  if (!(*vr))
    {
    vr = this->Cache->GetVoidRef("any_mesh",
          AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
          0, -1);
    }

  if (!(*vr))
    {
    vtkErrorMacro("Unable to find cache for dataset");
    return 0;
    }

  //determine the number of grids on each level of the AMR
  //and initialize the amr
  intVector gids = meshMetaData->groupIds;
  int *numDataSets = new int[ numGroups ];
  for ( int i=0; i < numGroups; ++i)
    {
    numDataSets[i] = 0; //clear the array
    }
  //count the grids at each level
  for ( int i=0; i < gids.size(); ++i )
    {
    ++numDataSets[gids.at(i)];
    }

  ghostedAMR->Initialize(numGroups, numDataSets);

  avtDomainNesting *domainNesting = reinterpret_cast<avtDomainNesting*>(*vr);
  for ( int i=1; i < numGroups; ++i) //don't need a ratio for level 0
    {
    intVector ratios = domainNesting->GetRatiosForLevel(i,domain);
    //Visit returns the ratio for each dimension and if it is a multiply or divide
    //Currently we just presume the same ratio for each dimension
    //TODO: verify this logic
    //  This log is probably incorrect -Leo
    // if ( ratios[0] >= 2 )
    //   {
    //   ghostedAMR->SetRefinementRatio(i, ratios[0] );
    //   }
    }

  //assign the info the the AMR, and create the uniform grids
  std::string name = meshMetaData->name;

  //take one pass through level 0 to compute the origin and grid description
  int gridDescription = -1;
  double globalOrigin[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
  int meshIndex=0;
  for (int j=0; j < numDataSets[0]; ++j)
    {
    vtkRectilinearGrid *rgrid = NULL;
    //get the rgrid from the VisIt reader
    //so we have the origin/spacing/dims
    CATCH_VISIT_EXCEPTIONS(rgrid,
                           vtkRectilinearGrid::SafeDownCast(
                             this->AvtFile->GetMesh(timestep, meshIndex, name.c_str())));
    ++meshIndex;
    if ( !rgrid )
      {
      //downcast failed or an exception was thrown
      continue;
      }

    double origin[3];
    origin[0] = rgrid->GetXCoordinates()->GetTuple1(0);
    origin[1] = rgrid->GetYCoordinates()->GetTuple1(0);
    origin[2] = rgrid->GetZCoordinates()->GetTuple1(0);

    int dims[3];
    rgrid->GetDimensions( dims );
    rgrid->Delete();

    //update the global origin
    for(int d=0; d<3; d++)
      {
      if(origin[d]< globalOrigin[d] )
        {
        globalOrigin[d] = origin[d];
        }
      }

    //use the first data set to compute the grid description
    if(gridDescription<0)
      {
      int ext[6] = {0,dims[0]-1,0,dims[1]-1,0,dims[2]-1};
      int ext1[6];
      gridDescription = vtkStructuredData::SetExtent(ext,ext1);
      }
    }

  ghostedAMR->SetOrigin(globalOrigin);
  ghostedAMR->SetGridDescription(gridDescription);

  meshIndex=0;
  for ( int i=0; i < numGroups; ++i)
    {
    for (int j=0; j < numDataSets[i]; ++j)
      {
      vtkRectilinearGrid *rgrid = NULL;
      //get the rgrid from the VisIt reader
      //so we have the origin/spacing/dims
      CATCH_VISIT_EXCEPTIONS(rgrid,
                             vtkRectilinearGrid::SafeDownCast(
                               this->AvtFile->GetMesh(timestep, meshIndex, name.c_str())));
      if ( !rgrid )
        {
        //downcast failed or an exception was thrown
        vtkWarningMacro("GetMesh failed");
        continue;
        }

      double origin[3];
      origin[0] = rgrid->GetXCoordinates()->GetTuple1(0);
      origin[1] = rgrid->GetYCoordinates()->GetTuple1(0);
      origin[2] = rgrid->GetZCoordinates()->GetTuple1(0);


      double spacing[3];
      spacing[0] = ( rgrid->GetXCoordinates()->GetNumberOfTuples() > 2 ) ?
        fabs( rgrid->GetXCoordinates()->GetTuple1(1) -
              rgrid->GetXCoordinates()->GetTuple1(0)): 1;

      spacing[1] = ( rgrid->GetYCoordinates()->GetNumberOfTuples() > 2 ) ?
        fabs( rgrid->GetYCoordinates()->GetTuple1(1) -
              rgrid->GetYCoordinates()->GetTuple1(0)): 1;

      spacing[2] = ( rgrid->GetZCoordinates()->GetNumberOfTuples() > 2 ) ?
        fabs( rgrid->GetZCoordinates()->GetTuple1(1) -
              rgrid->GetZCoordinates()->GetTuple1(0)): 1;

      int dims[3];
      rgrid->GetDimensions( dims );

      //don't need the rgrid anymoe
      rgrid->Delete();
      rgrid = NULL;

      vtkAMRBox box(origin, dims, spacing, globalOrigin, ghostedAMR->GetGridDescription());
      ghostedAMR->SetSpacing(i, spacing);
      ghostedAMR->SetAMRBox(i,j,box);

      //only load grids inside the domainRange for this processor
      if (this->ShouldReadDataSet(meshIndex) )
        {
        //create the dataset
        vtkUniformGrid *grid = vtkUniformGrid::New();
        grid->SetOrigin( origin );
        grid->SetSpacing( spacing );
        grid->SetDimensions( dims );
        this->AssignProperties( grid, name, timestep, meshIndex);
        ghostedAMR->SetDataSet(i,j,grid);
        grid->Delete();
        }
      ++meshIndex;
      }
    }


  vtkParallelAMRUtilities::StripGhostLayers(
      ghostedAMR,outputAMR,vtkMultiProcessController::GetGlobalController());
  ghostedAMR->Delete();
  vtkParallelAMRUtilities::BlankCells(outputAMR, vtkMultiProcessController::GetGlobalController());
  delete[] numDataSets;
  return 1;

}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::FillBlock(
  vtkMultiBlockDataSet *block, const avtMeshMetaData *meshMetaData,
  const int &timestep )
{
  if ( meshMetaData->meshType == AVT_CSG_MESH )
    {
    //CSG meshes do not act like any other block
    //so it has a seperate method.
    this->FillBlockWithCSG(block, meshMetaData, timestep );
    return;
    }

  std::string name = meshMetaData->name;

  //block names
  stringVector blockNames = meshMetaData->blockNames;
  int numBlockNames = blockNames.size();

  //set the number of pieces in the block
  block->SetNumberOfBlocks( meshMetaData->numBlocks );

  this->GetDomainRange(meshMetaData);
  for ( int i=this->Internal->MinDataset; i < this->Internal->MaxDataset; ++i )
    {
    if (!this->ShouldReadDataSet(i))
      {
      continue;
      }
    vtkDataSet *data=NULL;
    CATCH_VISIT_EXCEPTIONS(data,
      this->AvtFile->GetMesh(timestep, i, name.c_str()) );
    if ( data )
      {
      this->AssignProperties(data,name,timestep,i);

      //clean the mesh of all points that are not part of a cell
      if ( meshMetaData->meshType == AVT_UNSTRUCTURED_MESH)
        {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(data);
        vtkUnstructuredGridRelevantPointsFilter *clean =
            vtkUnstructuredGridRelevantPointsFilter::New();
        clean->SetInputData( ugrid );
        clean->Update();
        block->SetBlock(i,clean->GetOutput());
        clean->Delete();
        }
      else if(meshMetaData->meshType == AVT_SURFACE_MESH)
        {
        vtkCleanPolyData *clean = vtkCleanPolyData::New();
        clean->SetInputData( data );
        clean->ToleranceIsAbsoluteOn();
        clean->SetAbsoluteTolerance(0.0);
        clean->ConvertStripsToPolysOff();
        clean->ConvertPolysToLinesOff();
        clean->ConvertLinesToPointsOff();
        clean->Update();
        block->SetBlock(i,clean->GetOutput());
        clean->Delete();
        }
      else
        {
        block->SetBlock(i,data);
        }
      if ( i < numBlockNames)
        {
        block->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(),
                                 blockNames.at(i).c_str());
        }
      data->Delete();
      }
    }
}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::FillBlockWithCSG(
  vtkMultiBlockDataSet *block, const avtMeshMetaData *meshMetaData,
  const int &timestep )
{
  //this still does not support multi-block csg meshes

  std::string meshName = meshMetaData->name;

  //block names
  stringVector blockNames = meshMetaData->blockNames;
  int numBlockNames = blockNames.size();

  this->GetDomainRange(meshMetaData);
  for ( int i=this->Internal->MinDataset; i < this->Internal->MaxDataset; ++i )
    {
    if (!this->ShouldReadDataSet(i))
      {
      continue;
      }
    //basic uniform csg support
    int blockIndex = i;
    int csgRegion = 0;
    this->MetaData->ConvertCSGDomainToBlockAndRegion(meshName.c_str(),
      &blockIndex, &csgRegion);

    vtkDataSet *data=NULL;
    CATCH_VISIT_EXCEPTIONS(data,
      this->AvtFile->GetMesh(timestep, i, meshName.c_str()) );
    vtkCSGGrid *csgGrid = vtkCSGGrid::SafeDownCast(data);
    if ( csgGrid )
      {
      const double *bounds = csgGrid->GetBounds();
      vtkDataSet *csgResult = csgGrid->DiscretizeSpace( csgRegion, 0.01,
        bounds[0], bounds[1], bounds[2],
        bounds[3], bounds[4], bounds[5]);
      if ( csgResult )
        {
        block->SetBlock(i, csgResult );
        csgResult->Delete();
        if ( i < numBlockNames)
          {
          block->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(),
                                 blockNames.at(i).c_str());
          }
        }
      csgGrid->Delete();
      }
    }
}
//-----------------------------------------------------------------------------
bool vtkAvtSTMDFileFormatAlgorithm::ValidAMR( const avtMeshMetaData *meshMetaData )
{

  //I can't find an easy way to determine the type of a sub mesh
  std::string name = meshMetaData->name;
  vtkRectilinearGrid *rgrid = NULL;

  for ( int i=0; i < meshMetaData->numBlocks; ++i )
    {
    //lets get the mesh for each amr box
    vtkRectilinearGrid *rgrid = NULL;
    CATCH_VISIT_EXCEPTIONS(rgrid, vtkRectilinearGrid::SafeDownCast(
      this->AvtFile->GetMesh(0, i, name.c_str()) ) );
    if ( !rgrid )
      {
      //this is not an AMR that ParaView supports
      return false;
      }

    //verify the spacing of the grid is uniform
    if (!this->IsEvenlySpacedDataArray( rgrid->GetXCoordinates()) )
      {
      rgrid->Delete();
      return false;
      }
    if (!this->IsEvenlySpacedDataArray( rgrid->GetYCoordinates()) )
      {
      rgrid->Delete();
      return false;
      }
    if (!this->IsEvenlySpacedDataArray( rgrid->GetZCoordinates()) )
      {
      rgrid->Delete();
      return false;
      }
    rgrid->Delete();
    }

  return true;
}
//-----------------------------------------------------------------------------
bool vtkAvtSTMDFileFormatAlgorithm::IsEvenlySpacedDataArray(vtkDataArray *data)
{
  if ( !data )
    {
    return false;
    }

  //if we have less than 3 values it is evenly spaced
  vtkIdType size = data->GetNumberOfTuples();
  bool valid = true;
  if ( size > 2 )
    {
    double spacing = data->GetTuple1(1)-data->GetTuple1(0);
    double tolerance = 0.000001;
    for (vtkIdType j = 2; j < data->GetNumberOfTuples() && valid; ++j )
      {
      double temp = data->GetTuple1(j) - data->GetTuple1(j-1);
      valid = ( (temp - tolerance) <= spacing ) && ( (temp + tolerance) >= spacing ) ;
      }
    }
  return valid;
}

//----------------------------------------------------------------------------
//determine which nodes will be read by this processor
void vtkAvtSTMDFileFormatAlgorithm::GetDomainRange(const avtMeshMetaData *meshMetaData)
{
  int numBlock = meshMetaData->numBlocks;
  this->Internal->MinDataset = 0;
  this->Internal->MaxDataset = numBlock;

  vtkInformation* outInfo = this->GetOutputPortInformation(0);
  if (outInfo->Has(vtkCompositeDataPipeline::UPDATE_COMPOSITE_INDICES()))
    {
    //index based data requests
    this->Internal->UpdateIndices.clear();
    int length = outInfo->Length(vtkCompositeDataPipeline::UPDATE_COMPOSITE_INDICES());
    this->Internal->HasUpdateRestriction = (length > 0);
    if (this->Internal->HasUpdateRestriction)
      {
      int* idx = outInfo->Get(vtkCompositeDataPipeline::UPDATE_COMPOSITE_INDICES());
      this->Internal->UpdateIndices = std::set<int>(idx, idx+length);
      }
    }
  if (!this->Internal->HasUpdateRestriction)
    {
    //1 == load the whole data
    if ( this->UpdateNumPieces > 1 )
      {
      //determine which domains in this mesh this processor is responsible for
      float percent = (1.0 / this->UpdateNumPieces) * numBlock;
      this->Internal->MinDataset = percent * this->UpdatePiece;
      this->Internal->MaxDataset = (percent * this->UpdatePiece) + percent;
      }
    }
}
//-----------------------------------------------------------------------------
bool vtkAvtSTMDFileFormatAlgorithm::ShouldReadDataSet(const int &index)
{
  bool shouldRead =
    (index >= this->Internal->MinDataset &&
     index < this->Internal->MaxDataset);

  if (shouldRead && this->Internal->HasUpdateRestriction)
    {
    shouldRead = (this->Internal->UpdateIndices.find(index) ==
        this->Internal->UpdateIndices.end());
    }
  return shouldRead;
}

//-----------------------------------------------------------------------------
void vtkAvtSTMDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


