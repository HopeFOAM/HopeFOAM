/*=========================================================================

   Program: ParaView
   Module:    vtkAvtFileFormatAlgorithm.h

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

#ifndef _vtkAvtFileFormatAlgorithm_h
#define _vtkAvtFileFormatAlgorithm_h

#include <vector>
#include "vtkIOVisItBridgeModule.h" //for export macro
#include "vtkCompositeDataSetAlgorithm.h"
#include "vtkStdString.h"


class vtkDataArraySelection;
class vtkDataSet;
class vtkCallbackCommand;
class vtkInformation;

//BTX
class avtFileFormat;
class avtDatabaseMetaData;
class avtVariableCache;
class avtMeshMetaData;
//ETX

//Call a VisitMethod that returns a vtkObject
//if the call throws an exception we delete the object
//and set it to NULL
#define CATCH_VISIT_EXCEPTIONS( vtkObj,function) \
try \
  { \
  vtkObj = function; \
  } \
catch(...) \
  { \
  vtkErrorMacro("VisIt Exception caught.")\
  if ( vtkObj ) \
    { \
    vtkObj->Delete(); \
    } \
  vtkObj = NULL; \
  }

class VTKIOVISITBRIDGE_EXPORT vtkAvtFileFormatAlgorithm : public vtkCompositeDataSetAlgorithm
{
public:
  static vtkAvtFileFormatAlgorithm *New();
  vtkTypeMacro(vtkAvtFileFormatAlgorithm,vtkCompositeDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the number of point or cell arrays available in the input.
  int GetNumberOfPointArrays();
  int GetNumberOfCellArrays();
  int GetNumberOfMeshArrays();
  int GetNumberOfMaterialArrays();

  // Description:
  // Get the name of the point or cell array with the given index in
  // the input.
  const char* GetPointArrayName(int index);
  const char* GetCellArrayName(int index);
  const char* GetMeshArrayName(int index);
  const char* GetMaterialArrayName(int index);

  // Description:
  // Get/Set whether the point or cell array with the given name is to
  // be read.
  int GetPointArrayStatus(const char* name);
  int GetCellArrayStatus(const char* name);
  int GetMeshArrayStatus(const char* name);
  int GetMaterialArrayStatus(const char* name);

  void SetPointArrayStatus(const char* name, int status);
  void SetCellArrayStatus(const char* name, int status);
  void SetMeshArrayStatus(const char* name, int status);
  void SetMaterialArrayStatus(const char* name, int status);

protected:
  vtkAvtFileFormatAlgorithm();
  ~vtkAvtFileFormatAlgorithm();

  //helper method for none time aware readers
  bool InitializeAVTReader(){ return InitializeAVTReader(0);}

  //the visit reader wrapping will override the intialize method
  virtual bool InitializeAVTReader( const int &timestep );

  //the visit readers that support time will overrid the ActivateTimestep method
  virtual bool ActivateTimestep(const int &) {return false;}

  virtual void CleanupAVTReader();

  //Used to support requests for block and domain
  //level piece loading
  virtual int ProcessRequest(vtkInformation* request,
                vtkInformationVector** inputVector,
                vtkInformationVector* outputVector);
  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  //methods that setup selection arrays that the client will interact with
  void SetupDataArraySelections();
  void SetupMeshSelections();
  void SetupMaterialSelections();

  //method setups the number of timesteps that the file has
  void SetupTemporalInformation(vtkInformation *outInfo);

  //this method is used to get the current time step from the pipeline
  unsigned int GetCurrentTimeStep(vtkInformation *outInfo);

  // Callback registered with the SelectionObserver.
  static void SelectionModifiedCallback(vtkObject* caller, unsigned long eid,
                                        void* clientdata, void* calldata);

  //BTX
  void AssignProperties( vtkDataSet *data, const vtkStdString &meshName,
    const int &timestep, const int &domain );
  void AssignMaterials( vtkDataSet *data, const vtkStdString &meshName,
    const int &timestep, const int &domain );
  bool GetDataSpatialExtents(const char* meshName,
    const int &timestep, const int &domain, double bounds[6]);

  //creates a basic data selection based on bounds to pass to
  //the reader so it loads everything
  void CreateAVTDataSelections();
  //ETX


  // The array selections.
  vtkDataArraySelection* PointDataArraySelection;
  vtkDataArraySelection* CellDataArraySelection;
  vtkDataArraySelection* MeshArraySelection;
  vtkDataArraySelection* MaterialArraySelection;

  // The observer to modify this object when the array selections are
  // modified.
  vtkCallbackCommand* SelectionObserver;
  int OutputType;
  std::vector<bool> SelectionResults;

//BTX
  avtFileFormat *AvtFile;
  avtDatabaseMetaData *MetaData;
  avtVariableCache *Cache;
//ETX

private:
  vtkAvtFileFormatAlgorithm(const vtkAvtFileFormatAlgorithm&);
  void operator = (const vtkAvtFileFormatAlgorithm&);
};
#endif
