/*=========================================================================

   Program: ParaView
   Module:    vtkAvtFileFormatAlgorithm.cxx

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
#include "vtkAvtFileFormatAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkSmartPointer.h"
#include "vtkCompositeDataSet.h"

#include "vtkCallbackCommand.h"
#include "vtkDataArraySelection.h"

#include "vtkDataSet.h"

#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"

#include "avtDatabaseMetaData.h"
#include "avtDomainNesting.h"
#include "avtFileFormat.h"
#include "avtIntervalTree.h"
#include "avtMaterial.h"
#include "avtMaterialMetaData.h"
#include "avtScalarMetaData.h"
#include "avtSpatialBoxSelection.h"
#include "avtVariableCache.h"
#include "avtVectorMetaData.h"
#include "TimingsManager.h"

#include "limits.h"

vtkStandardNewMacro(vtkAvtFileFormatAlgorithm);

//-----------------------------------------------------------------------------
vtkAvtFileFormatAlgorithm::vtkAvtFileFormatAlgorithm()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);

  this->AvtFile = NULL;
  this->MetaData = NULL;
  this->Cache = NULL;

  this->PointDataArraySelection = vtkDataArraySelection::New();
  this->CellDataArraySelection = vtkDataArraySelection::New();
  this->MeshArraySelection = vtkDataArraySelection::New();
  this->MaterialArraySelection = vtkDataArraySelection::New();

  // Setup the selection callback to modify this object when an array
  // selection is changed.
  this->SelectionObserver = vtkCallbackCommand::New();
  this->SelectionObserver->SetCallback(&
    vtkAvtFileFormatAlgorithm::SelectionModifiedCallback);
  this->SelectionObserver->SetClientData(this);
  this->PointDataArraySelection->AddObserver(vtkCommand::ModifiedEvent,
                                             this->SelectionObserver);
  this->CellDataArraySelection->AddObserver(vtkCommand::ModifiedEvent,
                                            this->SelectionObserver);
  this->MeshArraySelection->AddObserver(vtkCommand::ModifiedEvent,
                                            this->SelectionObserver);
  this->MaterialArraySelection->AddObserver(vtkCommand::ModifiedEvent,
                                            this->SelectionObserver);

  //visit has this horrible singelton timer that is called in all algorithms
  //we need to initiailize it, and than disable it
  if ( !visitTimer )
    {
    TimingsManager::Initialize("");
    visitTimer->Disable();
    }
}

//-----------------------------------------------------------------------------
vtkAvtFileFormatAlgorithm::~vtkAvtFileFormatAlgorithm()
{
  this->CleanupAVTReader();

  this->CellDataArraySelection->RemoveObserver(this->SelectionObserver);
  this->PointDataArraySelection->RemoveObserver(this->SelectionObserver);
  this->MeshArraySelection->RemoveObserver(this->SelectionObserver);
  this->MaterialArraySelection->RemoveObserver(this->SelectionObserver);

  this->SelectionObserver->Delete();
  this->CellDataArraySelection->Delete();
  this->PointDataArraySelection->Delete();
  this->MeshArraySelection->Delete();
  this->MaterialArraySelection->Delete();
}

//-----------------------------------------------------------------------------
bool vtkAvtFileFormatAlgorithm::InitializeAVTReader( const int &timestep )
{
  return false;
}

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::CleanupAVTReader()
{
  if ( this->AvtFile )
    {
    this->AvtFile->FreeUpResources();
    delete this->AvtFile;
    this->AvtFile = NULL;
    }

  if ( this->MetaData )
    {
    delete this->MetaData;
    this->MetaData = NULL;
    }

  if ( this->Cache )
    {
    delete this->Cache;
    this->Cache = NULL;
    }
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::ProcessRequest(vtkInformation* request,
                                         vtkInformationVector** inputVector,
                                         vtkInformationVector* outputVector)
{
  // TODO (berk)
  // This should be either in RequestInformation or RequestTemporalInformation

  // // generate the needed data for each time step
  // // to handle domain level piece loading
  // if(request->Has(
  // vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT_INFORMATION()))
  //   {
  //   vtkInformation *outInfo = outputVector->GetInformationObject(0);
  //   this->SetupBlockBoundsInformation(outInfo);
  //   }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}


//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::RequestInformation(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  if (!this->InitializeAVTReader())
    {
    return 0;
    }
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if ( this->MetaData->GetNumMeshes() > 0 )
    {
    int maxPieces = (this->MetaData->GetMeshes(0).numBlocks > 1)?
      -1:1;
    //only MD classes have blocks inside a mesh, and therefore
    //we can use that to determine if we support reading on each processor
    //outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(),
    //maxPieces);
    }

  //Set up ghost levels

  //setup user selection of meshes to load
  this->SetupMeshSelections();
  //setup user selection of arrays to load
  this->SetupDataArraySelections();

  //setup the materials that are on all the meshes
  this->SetupMaterialSelections();

  //setup the timestep and cylce info
  this->SetupTemporalInformation(outInfo);

  return 1;
}


//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::RequestData(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::RequestUpdateExtent(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::FillOutputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataObject");
  return 1;
}


//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::AssignProperties( vtkDataSet *data,
    const vtkStdString &meshName, const int &timestep, const int &domain)
{
  int size = this->MetaData->GetNumScalars();
  for ( int i=0; i < size; ++i)
    {
    const avtScalarMetaData scalarMeta = this->MetaData->GetScalars(i);
    if ( meshName != scalarMeta.meshName )
      {
      //this mesh doesn't have this scalar property, go to next
      continue;
      }

    std::string name = scalarMeta.name;

    //now check against what arrays the user has selected to load
    bool selected = false;
    if (scalarMeta.centering == AVT_ZONECENT)
      {
      //cell array
      selected = this->GetCellArrayStatus(name.c_str());
      }
    else if (scalarMeta.centering == AVT_NODECENT)
      {
      //point array
      selected = this->GetPointArrayStatus(name.c_str());
      }
    if (!selected)
      {
      //don't add the array since the user hasn't selected it
      continue;
      }

    //some readers will throw exceptions when they can't find
    //the file containing properties, so we have to ignore that property
    vtkDataArray *scalar = NULL;
    CATCH_VISIT_EXCEPTIONS(scalar,
      this->AvtFile->GetVar(timestep,domain,name.c_str()));
    if ( !scalar )
      {
      //it seems that we had a bad array for this domain
      continue;
      }

    //update the vtkDataArray to have the name, since GetVar doesn't require
    //placing a name on the returned array
    scalar->SetName( name.c_str() );

    //based on the centering we go determine if this is cell or point based
    switch(scalarMeta.centering)
      {
      case AVT_ZONECENT:
        //cell property
        data->GetCellData()->AddArray( scalar );
        break;
      case AVT_NODECENT:
        //point based
        data->GetPointData()->AddArray( scalar );
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
      default:
        break;
      }
    scalar->Delete();
    }

  //now do vector properties
  size = this->MetaData->GetNumVectors();
  for ( int i=0; i < size; ++i)
    {
    const avtVectorMetaData vectorMeta = this->MetaData->GetVectors(i);
    if ( meshName != vectorMeta.meshName )
      {
      //this mesh doesn't have this vector property, go to next
      continue;
      }
    std::string name = vectorMeta.name;

    //now check agianst what arrays the user has selected to load
    bool selected = false;
    if (vectorMeta.centering == AVT_ZONECENT)
      {
      //cell array
      selected = this->GetCellArrayStatus(name.c_str());
      }
    else if (vectorMeta.centering == AVT_NODECENT)
      {
      //point array
      selected = this->GetPointArrayStatus(name.c_str());
      }
    if (!selected)
      {
      //don't add the array since the user hasn't selected it
      continue;
      }

    vtkDataArray *vector = NULL;
    CATCH_VISIT_EXCEPTIONS(vector,
      this->AvtFile->GetVectorVar(timestep,domain,name.c_str()));
    if ( !vector )
      {
      //it seems that we had a bad array for this domain
      continue;
      }

    //update the vtkDataArray to have the name, since GetVar doesn't require
    //placing a name on the returned array
    vector->SetName( name.c_str() );

    //based on the centering we go determine if this is cell or point based
    switch(vectorMeta.centering)
      {
      case AVT_ZONECENT:
        //cell property
        data->GetCellData()->AddArray( vector );
        break;
      case AVT_NODECENT:
        //point based
        data->GetPointData()->AddArray( vector );
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
        break;
      }
    vector->Delete();
    }

  //now call the materials
  this->AssignMaterials( data, meshName, timestep, domain);
}

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::AssignMaterials( vtkDataSet *data,
  const vtkStdString &meshName, const int &timestep, const int &domain )
{
  //now we check for materials
  int size = this->MetaData->GetNumMaterials();
  void_ref_ptr vr;
  avtMaterial *material = NULL;
  for ( int i=0; i < size; ++i)
    {
    const avtMaterialMetaData* materialMetaData = this->MetaData->GetMaterial(i);
    if ( meshName != materialMetaData->meshName )
      {
      continue;
      }

    std::string name = materialMetaData->name;
    //lets first try and see if the data has been cached
    //get the aux data from the cache for the material
    vr = this->Cache->GetVoidRef(name.c_str(),
                    AUXILIARY_DATA_MATERIAL, timestep, domain);
    material = reinterpret_cast<avtMaterial*>(*vr);
    if ( !material)
      {
      //data wasn't cached! time to ask dataset itself
      DestructorFunction df;
      void* ref = this->AvtFile->GetAuxiliaryData(name.c_str(),timestep,domain,
        AUXILIARY_DATA_MATERIAL,NULL,df);
      if ( !ref )
        {
        continue;
        }

      //add the material to the cache
      vr = void_ref_ptr(ref,df);
      this->Cache->CacheVoidRef(name.c_str(),AUXILIARY_DATA_MATERIAL, timestep, domain, vr );
      material = reinterpret_cast<avtMaterial*>(*vr);
      if ( !material)
        {
        continue;
        }
      }

    //decompose the material class into a collection of float arrays
    //that we will than push into vtkFloatArrays and place on the dataset
    int numCells = material->GetNZones();
    int mats = material->GetNMaterials();
    float** materials = new float*[mats];
    for ( int i=0; i < mats; ++i)
      {
      materials[i] = new float[numCells];
      for ( int j=0; j < numCells; ++j)
        {
        materials[i][j] = -1.0;
        }
      }

    const int *matlist = material->GetMatlist();
    const int *mixMat = material->GetMixMat();
    const int *mixNext = material->GetMixNext();
    const float *mixValues = material->GetMixVF();
    for ( int i=0; i < numCells; ++i)
      {
      if ( matlist[i] >= 0 )
        {
        //this material is pure
        materials[matlist[i]][i] = 1.0;
        }
      else
        {
        float sum = 0.0;
        int lookupIndex = (matlist[i]+1) * -1;
        while ( sum < 1.0 )
          {
          materials[mixMat[lookupIndex]][i] = mixValues[lookupIndex];
          sum += mixValues[lookupIndex];
          if ( mixNext[lookupIndex] == 0 )
            {
            //just in case a material doesn't sum up to 100
            break;
            }
          lookupIndex = mixNext[lookupIndex]-1;
          }
        }
      }

    //we now have all our arrays loaded with the material mixtures
    //time to pass them to vtk
    stringVector mNames = materialMetaData->materialNames;
    for ( int i=0; i < mNames.size(); ++i)
      {
      //TODO: change this so that we check selection enabled before we
      //decompose the avtMaterial class
      if ( this->MaterialArraySelection->ArrayIsEnabled(mNames.at(i).c_str()) )
        {
        vtkFloatArray* tempMaterial = vtkFloatArray::New();
        tempMaterial->SetName( mNames.at(i).c_str() );
        tempMaterial->SetArray(materials[i],numCells,0);
        data->GetCellData()->AddArray( tempMaterial );
        tempMaterial->Delete();
        }
      }
    }
}

//-----------------------------------------------------------------------------
/*
void vtkAvtFileFormatAlgorithm::SetupBlockBoundsInformation(
  vtkInformation *outInfo)
{
  //this allows the VisIt Readers to support individual
  //domain and block loading
  vtkSmartPointer<vtkMultiBlockDataSet> metadata =
      vtkSmartPointer<vtkMultiBlockDataSet>::New();

  unsigned int index = 0; //converting the multiblock to a flat index

  int size = this->MetaData->GetNumMeshes();
  int timeStep = this->GetCurrentTimeStep(outInfo);
  for ( int i=0; i < size; ++i)
    {
    const avtMeshMetaData *meshMetaData = this->MetaData->GetMesh(i);

    int numBlocks = meshMetaData->numBlocks;

    //setup the block that represents this mesh
    vtkMultiBlockDataSet* childDS = vtkMultiBlockDataSet::New();
    childDS->SetNumberOfBlocks(numBlocks);
    metadata->SetBlock(i,childDS);
    childDS->FastDelete();

    //setup the bounding box for each domain in this block
    for ( int dom=0; dom < numBlocks; ++dom )
      {

      //create the block for this domain
      childDS->SetBlock(dom,NULL);
      vtkInformation* piece_metadata = childDS->GetMetaData(dom);

      double bounds[6] = {0.0,0.0,0.0,0.0,0.0,0.0};
      bool valid =
        this->GetDataSpatialExtents(meshMetaData->name.c_str(),
        timeStep, dom, bounds);
      if ( valid )
        {
        piece_metadata->Set(
        vtkStreamingDemandDrivenPipeline::BOUNDS(),bounds,6);
        }
      ++index;
      }
    }

  outInfo->Set(vtkCompositeDataPipeline::COMPOSITE_DATA_META_DATA(),
            metadata);
}
*/

//-----------------------------------------------------------------------------
bool vtkAvtFileFormatAlgorithm::GetDataSpatialExtents(const char* meshName,
    const int &timestep, const int &domain, double bounds[6])
{
    void_ref_ptr vr = this->Cache->GetVoidRef(meshName,
                  AUXILIARY_DATA_SPATIAL_EXTENTS, timestep, domain);
    if (!(*vr))
      {
      //the specfic domain failed, try the global size for the timestep
      void_ref_ptr vr = this->Cache->GetVoidRef(meshName,
        AUXILIARY_DATA_SPATIAL_EXTENTS, timestep, -1);
      }
    if (!(*vr))
      {
      //the specfic timestep failed, try the gloabl extent
      void_ref_ptr vr = this->Cache->GetVoidRef(meshName,
        AUXILIARY_DATA_SPATIAL_EXTENTS, -1, -1);
      }
    if (!(*vr))
      {
      //everything failed we don't have information!
      return false;
      }
    avtIntervalTree *tree = NULL;
    tree = reinterpret_cast<avtIntervalTree*>(*vr);
    if ( tree )
      {
      tree->GetExtents(bounds);
      return true;
      }
  }

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::CreateAVTDataSelections()
{
  //by default the box selection is a box from FLT MIN to FLT MAX so
  //we will be asking the reader to load everything in.
  avtSpatialBoxSelection* selectWholeMesh = new avtSpatialBoxSelection();

  std::vector<avtDataSelection_p> selections;
  selections.push_back(selectWholeMesh);
  this->SelectionResults.resize (selections.size ());

  this->AvtFile->RegisterDataSelections(selections,&this->SelectionResults);
}

//-----------------------------------------------------------------------------
unsigned int vtkAvtFileFormatAlgorithm::GetCurrentTimeStep(vtkInformation *outInfo)
{
  int tsLength =
    outInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  double* steps =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

  unsigned int TimeIndex = 0;
  // Check if a particular time was requested by the pipeline.
  // This overrides the ivar.
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()) && tsLength>0)
    {
    // Get the requested time step. We only supprt requests of a single time
    // step in this reader right now
    double requestedTimeStep =
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());

    // find the first time value larger than requested time value
    // this logic could be improved
    while (TimeIndex < tsLength-1 && steps[TimeIndex] < requestedTimeStep)
      {
      TimeIndex++;
      }
    }
  return TimeIndex;
}

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetupTemporalInformation(
  vtkInformation *outInfo)
{

  int numTimeValues;
  double timeRange[2];
  std::vector< double > timesteps;
  std::vector< int > cycles;

  try
    {
    this->AvtFile->FormatGetTimes( timesteps );
    this->AvtFile->FormatGetCycles( cycles );
    }
  catch(...)
    {
    //unable to get time or cycles
    return;
    }

  bool hasTime = timesteps.size() > 0;
  bool hasCycles = cycles.size() > 0;
  bool hasTimeAndCycles = hasTime && hasCycles;

  //in some case the times and cycles have all zero values,
  //or everything but the first time step have are zero values
  //This is caused by a file reader that generates the time value
  //once the reader moves to that timestep.
  //That kind of behavior is not possible currently in ParaView. Instead
  //we will force the reader to generate the time values for each timestep
  //by cycling through everytime step but not requesting any data.
  bool needs_manual_query = false;
  if(hasTime)
    {
    std::size_t last_spot = timesteps.size()-1;
    needs_manual_query = (timesteps[0] == timesteps[last_spot]);
    needs_manual_query |= ( timesteps.size() > 2 &&
                            timesteps[last_spot-1] == timesteps[last_spot] &&
                            timesteps[last_spot] == 0 );
    }
  if(hasTime && needs_manual_query)
    {
    //FormatGetTimes expect the timesteps vector that is passed
    //in has an empty size. If you use the timesteps variable
    //readers will push_back values causing an infinte loop or
    //duplicate time steps
    std::vector< double > newTimesSteps;

    //we have hit a timestep range that needs to be cycled
    //we use a const size variable so
    const std::size_t size = timesteps.size();
    for(int i=0; i < size;++i)
      {
      this->ActivateTimestep(i);
      //Nek and other readers don't update the time info intill you
      //call gettimes.
      this->AvtFile->FormatGetTimes(newTimesSteps);
      }

    //use the updated timestep values
    timesteps = newTimesSteps;
    }


  //need to figure out the use case of when cycles and timesteps don't match
  if (hasTimeAndCycles && timesteps.size()==cycles.size() )
    {
    //presume that timesteps and cycles are just duplicates of each other
    numTimeValues = static_cast<int>(timesteps.size());
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      &timesteps[0],numTimeValues);
    timeRange[0] = timesteps[0];
    timeRange[1] = timesteps[numTimeValues-1];
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      timeRange, 2);
    }
  else if( hasTime )
    {
    numTimeValues = static_cast<int>(timesteps.size());

    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      &timesteps[0],numTimeValues);
    timeRange[0] = timesteps[0];
    timeRange[1] = timesteps[numTimeValues-1];
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      timeRange, 2);
    }
  else if( hasCycles )
    {
    //convert the cycles over to time steps now
    for ( unsigned int i=0; i < cycles.size(); ++i)
      {
      timesteps.push_back( static_cast<double>(cycles[i]) );
      }

    numTimeValues = static_cast<int>(timesteps.size());

    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      &timesteps[0],numTimeValues);
    timeRange[0] = timesteps[0];
    timeRange[1] = timesteps[numTimeValues-1];
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      timeRange, 2);
    }
}

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetupDataArraySelections( )
{
  if (!this->MetaData)
    {
    return;
    }
  //go through the meta data and get all the scalar and vector property names
  //add them to the point & cell selection arrays for user control if they don't already exist
  //by default all properties are disabled
  int size = this->MetaData->GetNumScalars();
  std::string name;
  for ( int i=0; i < size; ++i)
    {
    const avtScalarMetaData scalarMetaData = this->MetaData->GetScalars(i);
    name = scalarMetaData.name;
    switch(scalarMetaData.centering)
      {
      case AVT_ZONECENT:
        //cell property
        if (!this->CellDataArraySelection->ArrayExists(name.c_str()))
          {
          this->CellDataArraySelection->DisableArray(name.c_str());
          }
        break;
      case AVT_NODECENT:
        //point based
        if (!this->PointDataArraySelection->ArrayExists(name.c_str()))
          {
          this->PointDataArraySelection->DisableArray(name.c_str());
          }
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
        break;
      }

    }

  size = this->MetaData->GetNumVectors();
  for ( int i=0; i < size; ++i)
    {
    const avtVectorMetaData vectorMetaData = this->MetaData->GetVectors(i);
    name = vectorMetaData.name;
    switch(vectorMetaData.centering)
      {
      case AVT_ZONECENT:
        //cell property
        if (!this->CellDataArraySelection->ArrayExists(name.c_str()))
          {
          this->CellDataArraySelection->DisableArray(name.c_str());
          }
        break;
      case AVT_NODECENT:
        //point based
        if (!this->PointDataArraySelection->ArrayExists(name.c_str()))
          {
          this->PointDataArraySelection->DisableArray(name.c_str());
          }
        break;
      case AVT_NO_VARIABLE:
      case AVT_UNKNOWN_CENT:
        break;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetupMeshSelections( )
{
  if (!this->MetaData)
    {
    return;
    }
  //go through the meta data and get all the mesh names
  //by default all meshes are disabled but the first one
  int size = this->MetaData->GetNumMeshes();
  std::string name;
  for ( int i=0; i < size; ++i)
    {
    const avtMeshMetaData *meshMetaData = this->MetaData->GetMesh(i);
    name = meshMetaData->name;
    if ( i == 0 && !this->MeshArraySelection->ArrayExists(name.c_str()))
      {
      this->MeshArraySelection->EnableArray(name.c_str());
      }
    else if (!this->MeshArraySelection->ArrayExists(name.c_str()))
      {
      this->MeshArraySelection->DisableArray(name.c_str());
      }
    }
}


//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetupMaterialSelections()
{
  if (!this->MetaData)
    {
    return;
    }
  //go through the meta data and get all the material names
  int size = this->MetaData->GetNumMaterials();
  std::string name;
  for ( int i=0; i < size; ++i)
    {
    const avtMaterialMetaData* matMetaData = this->MetaData->GetMaterial(i);
    //we are going to decompose the material into a separate array for each
    //component in the material collection.
    stringVector materials = matMetaData->materialNames;
    for ( int j=0; j < materials.size(); ++j )
      {
      name = materials.at(j);
      if (!this->MaterialArraySelection->ArrayExists(name.c_str()))
        {
        this->MaterialArraySelection->DisableArray(name.c_str());
        }
      }
    }
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetNumberOfPointArrays()
{
  return this->PointDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtFileFormatAlgorithm::GetPointArrayName(int index)
{
  return this->PointDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetPointArrayStatus(const char* name)
{
  return this->PointDataArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetPointArrayStatus(const char* name, int status)
{
  if(status)
    {
    this->PointDataArraySelection->EnableArray(name);
    }
  else
    {
    this->PointDataArraySelection->DisableArray(name);
    }
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetNumberOfCellArrays()
{
  return this->CellDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtFileFormatAlgorithm::GetCellArrayName(int index)
{
  return this->CellDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetCellArrayStatus(const char* name)
{
  return this->CellDataArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetCellArrayStatus(const char* name, int status)
{
  if(status)
    {
    this->CellDataArraySelection->EnableArray(name);
    }
  else
    {
    this->CellDataArraySelection->DisableArray(name);
    }
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetNumberOfMeshArrays()
{
  return this->MeshArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtFileFormatAlgorithm::GetMeshArrayName(int index)
{
  return this->MeshArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetMeshArrayStatus(const char* name)
{
  return this->MeshArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetMeshArrayStatus(const char* name, int status)
{
  if(status)
    {
    this->MeshArraySelection->EnableArray(name);
    }
  else
    {
    this->MeshArraySelection->DisableArray(name);
    }
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetNumberOfMaterialArrays()
{
  return this->MaterialArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
const char* vtkAvtFileFormatAlgorithm::GetMaterialArrayName(int index)
{
  return this->MaterialArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkAvtFileFormatAlgorithm::GetMaterialArrayStatus(const char* name)
{
  return this->MaterialArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SetMaterialArrayStatus(const char* name, int status)
{
  if(status)
    {
    this->MaterialArraySelection->EnableArray(name);
    }
  else
    {
    this->MaterialArraySelection->DisableArray(name);
    }
}

//----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::SelectionModifiedCallback(vtkObject*, unsigned long,
                                             void* clientdata, void*)
{
  static_cast<vtkAvtFileFormatAlgorithm*>(clientdata)->Modified();
}


//-----------------------------------------------------------------------------
void vtkAvtFileFormatAlgorithm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
   os << indent << "Output Type: " << this->OutputType << "\n";
}
