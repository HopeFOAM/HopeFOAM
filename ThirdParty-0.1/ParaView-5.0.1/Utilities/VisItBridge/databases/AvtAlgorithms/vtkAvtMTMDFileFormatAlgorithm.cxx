/*=========================================================================

   Program: ParaView
   Module:    vtkAvtMTMDFileFormatAlgorithm.cxx

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
#include "vtkAvtMTMDFileFormatAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDataArraySelection.h"

#include "vtkAMRBox.h"
#include "vtkHierarchicalBoxDataSet.h"
#include "vtkMultiBlockDataSet.h"
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

#include "avtMTMDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtMTMDFileFormatAlgorithm);
//-----------------------------------------------------------------------------
vtkAvtMTMDFileFormatAlgorithm::vtkAvtMTMDFileFormatAlgorithm()
{
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
}

//-----------------------------------------------------------------------------
vtkAvtMTMDFileFormatAlgorithm::~vtkAvtMTMDFileFormatAlgorithm()
{
}

//-----------------------------------------------------------------------------
int vtkAvtMTMDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  unsigned int TimeIndex = this->GetCurrentTimeStep(outInfo);

  if (!this->InitializeAVTReader( TimeIndex ))
    {
    return 0;
    }
  this->CreateAVTDataSelections();

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
    this->FillAMR( output, &meshMetaData, TimeIndex, 0);
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
          this->FillBlock( tempData, &meshMetaData, TimeIndex );
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
void vtkAvtMTMDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


