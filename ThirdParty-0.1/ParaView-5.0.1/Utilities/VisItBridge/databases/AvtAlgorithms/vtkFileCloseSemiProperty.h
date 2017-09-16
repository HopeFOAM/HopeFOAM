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

#ifndef _vtkFileCloseSemiProperty_h
#define _vtkFileCloseSemiProperty_h

//.NAME vtkFileCloseSemiProperty – Manages hdf5 close_semi property.
//.SECTION Creates and manages a hdf5 close_semi property.
//
// Needs <hdf5.h>, so include this header only after you include the
// hdf5 header. Because you might need to select the right API for hdf5 (such
// as with H5_USE_16_API) the hdf5 header is not included here.
class vtkFileCloseSemiProperty
{
public:
  vtkFileCloseSemiProperty ()
  {
    this->Fapl = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fclose_degree(this->Fapl, H5F_CLOSE_SEMI);
  }
  ~vtkFileCloseSemiProperty ()
  {
    H5Pclose(this->Fapl);
  }
  operator hid_t() const
  {
    return this->Fapl;
  }


private:
  hid_t Fapl;
};

#endif
