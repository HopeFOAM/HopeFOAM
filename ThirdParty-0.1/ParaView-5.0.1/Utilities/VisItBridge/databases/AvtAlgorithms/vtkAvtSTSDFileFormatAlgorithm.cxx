/*=========================================================================

   Program: ParaView
   Module:    vtkAvtSTSDFileFormatAlgorithm.cxx

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
#include "vtkAvtSTSDFileFormatAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDataArraySelection.h"

#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"
#include "vtkUnstructuredGrid.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"

#include "vtkUnstructuredGridRelevantPointsFilter.h"
#include "vtkCleanPolyData.h"
#include "vtkCSGGrid.h"

#include "avtSTSDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtSTSDFileFormatAlgorithm);
//-----------------------------------------------------------------------------
vtkAvtSTSDFileFormatAlgorithm::vtkAvtSTSDFileFormatAlgorithm()
{
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
}

//-----------------------------------------------------------------------------
vtkAvtSTSDFileFormatAlgorithm::~vtkAvtSTSDFileFormatAlgorithm()
{
}

//-----------------------------------------------------------------------------
int vtkAvtSTSDFileFormatAlgorithm::RequestDataObject(vtkInformation *,
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));

  if ( output && output->GetDataObjectType() == this->OutputType )
    {
    return 1;
    }
  else if ( !output || output->GetDataObjectType() != this->OutputType )
    {
    output = vtkMultiBlockDataSet::New();
    }
  info->Set(vtkDataObject::DATA_OBJECT(), output);
  output->Delete();

  return 1;
  }

//-----------------------------------------------------------------------------
int vtkAvtSTSDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  if (!this->InitializeAVTReader())
    {
    return 0;
    }
  this->CreateAVTDataSelections();

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    vtkErrorMacro("Was unable to determine output type");
    return 0;
    }

  this->FillMultiBlock(output, 0);
  this->CleanupAVTReader();
  return 1;
}

void vtkAvtSTSDFileFormatAlgorithm::FillMultiBlock(vtkMultiBlockDataSet *output, const int &timestep)
{
  int size = this->MetaData->GetNumMeshes();
  if ( this->MeshArraySelection )
    {
    //we don't want NULL blocks to be displayed, so get the
    //actual number of meshes the user wants
    size = this->MeshArraySelection->GetNumberOfArraysEnabled();
    }
  output->SetNumberOfBlocks( size );

  std::string name;
  int blockIndex = 0;
  for ( int i=0; i < this->MetaData->GetNumMeshes(); ++i)
    {
    const avtMeshMetaData meshMetaData = this->MetaData->GetMeshes( i );
    name = meshMetaData.name;

    //before we get the mesh see if the user wanted to load this mesh
    if (this->MeshArraySelection &&
      !this->MeshArraySelection->ArrayIsEnabled( name.c_str() ) )
      {
      continue;
      }

    vtkDataSet *data=NULL;
    CATCH_VISIT_EXCEPTIONS(data,
      this->AvtFile->GetMesh(timestep, 0, name.c_str()) );

    if ( data )
      {
      this->AssignProperties( data, name, timestep, 0);

       //clean the mesh of all points that are not part of a cell
      if ( meshMetaData.meshType == AVT_UNSTRUCTURED_MESH)
        {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(data);
        vtkUnstructuredGridRelevantPointsFilter *clean =
            vtkUnstructuredGridRelevantPointsFilter::New();
        clean->SetInputData( ugrid );
        clean->Update();
        output->SetBlock(blockIndex,clean->GetOutput());
        clean->Delete();
        }
      else if(meshMetaData.meshType == AVT_SURFACE_MESH)
        {
        vtkCleanPolyData *clean = vtkCleanPolyData::New();
        clean->SetInputData( data );
        clean->ToleranceIsAbsoluteOn();
        clean->SetAbsoluteTolerance(0.0);
        clean->ConvertStripsToPolysOff();
        clean->ConvertPolysToLinesOff();
        clean->ConvertLinesToPointsOff();
        clean->Update();
        output->SetBlock(blockIndex,clean->GetOutput());
        clean->Delete();
        }
      else if(meshMetaData.meshType == AVT_CSG_MESH)
        {
        //basic uniform csg support
        int blockIndex = i;
        int csgRegion = 0;
        this->MetaData->ConvertCSGDomainToBlockAndRegion(name.c_str(),
                                                &blockIndex, &csgRegion);
        vtkCSGGrid *csgGrid = vtkCSGGrid::SafeDownCast(data);
        const double *bounds = csgGrid->GetBounds();
        vtkDataSet *csgResult = csgGrid->DiscretizeSpace( csgRegion, 0.01,
            bounds[0], bounds[1], bounds[2],
            bounds[3], bounds[4], bounds[5]);
        if ( csgResult )
          {
          output->SetBlock(i,csgResult );
          csgResult->Delete();
          }
        }
      else
        {
        output->SetBlock(blockIndex,data);
        }
      data->Delete();
      output->GetMetaData(blockIndex)->Set(vtkCompositeDataSet::NAME(),name.c_str());
      ++blockIndex;
      }
    }


}

//-----------------------------------------------------------------------------
void vtkAvtSTSDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


