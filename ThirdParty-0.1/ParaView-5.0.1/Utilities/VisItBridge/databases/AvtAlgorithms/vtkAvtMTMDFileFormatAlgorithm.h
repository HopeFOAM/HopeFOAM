/*=========================================================================

   Program: ParaView
   Module:    vtkAvtMTMDFileFormatAlgorithm.h

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

#ifndef _vtkAvtMTMDFileFormatAlgorithm_h
#define _vtkAvtMTMDFileFormatAlgorithm_h

#include "vtkIOVisItBridgeModule.h" //for export macro

#include "vtkAvtSTMDFileFormatAlgorithm.h"

//BTX
class avtDatabaseMetaData;
class avtVariableCache;
//ETX

class VTKIOVISITBRIDGE_EXPORT vtkAvtMTMDFileFormatAlgorithm : public vtkAvtSTMDFileFormatAlgorithm
{
public:
  static vtkAvtMTMDFileFormatAlgorithm *New();
  vtkTypeMacro(vtkAvtMTMDFileFormatAlgorithm,vtkAvtSTMDFileFormatAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkAvtMTMDFileFormatAlgorithm();
  ~vtkAvtMTMDFileFormatAlgorithm();

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkAvtMTMDFileFormatAlgorithm(const vtkAvtMTMDFileFormatAlgorithm&);
  void operator = (const vtkAvtMTMDFileFormatAlgorithm&);
};
#endif
