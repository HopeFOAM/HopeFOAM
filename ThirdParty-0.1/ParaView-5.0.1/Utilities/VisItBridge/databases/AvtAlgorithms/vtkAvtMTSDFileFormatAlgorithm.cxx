/*=========================================================================

   Program: ParaView
   Module:    vtkAvtMTSDFileFormatAlgorithm.cxx

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
#include "vtkAvtMTSDFileFormatAlgorithm.h"
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

#include "avtMTSDFileFormat.h"
#include "avtDomainNesting.h"
#include "avtDatabaseMetaData.h"
#include "avtVariableCache.h"
#include "avtScalarMetaData.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtMTSDFileFormatAlgorithm);
//-----------------------------------------------------------------------------
vtkAvtMTSDFileFormatAlgorithm::vtkAvtMTSDFileFormatAlgorithm()
{
  this->OutputType = VTK_MULTIBLOCK_DATA_SET;
}

//-----------------------------------------------------------------------------
vtkAvtMTSDFileFormatAlgorithm::~vtkAvtMTSDFileFormatAlgorithm()
{
}

//-----------------------------------------------------------------------------
int vtkAvtMTSDFileFormatAlgorithm::RequestData(vtkInformation *request,
        vtkInformationVector **inputVector, vtkInformationVector *outputVector)
  {
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  unsigned int TimeIndex = this->GetCurrentTimeStep(outInfo);

  if (!this->InitializeAVTReader( TimeIndex ))
    {
    return 0;
    }
  this->CreateAVTDataSelections();

  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!output)
    {
    vtkErrorMacro("Was unable to determine output type");
    return 0;
    }
  this->FillMultiBlock(output,TimeIndex);
  this->CleanupAVTReader();
  return 1;
}

//-----------------------------------------------------------------------------
void vtkAvtMTSDFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


