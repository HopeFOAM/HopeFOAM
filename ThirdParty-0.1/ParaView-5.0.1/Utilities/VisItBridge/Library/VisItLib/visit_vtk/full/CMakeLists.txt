#*****************************************************************************
#
# Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#   Cyrus Harrison, Tue Mar  9 08:28:53 PST 2010
#   Define __USE_ISOC99 if we are using the boost interval lib to make sure
#   a rounding scheme is available.
#
#****************************************************************************/
project(visit_vtk)

SET(VISIT_VTK_SOURCES
InitVTK.C
vtkAxisDepthSort.C
vtkBinaryPartitionVolumeFromVolume.C
vtkCSGCell.C
vtkCSGGrid.C
vtkConnectedTubeFilter.C
vtkDataSetRemoveGhostCells.C
vtkLineoutFilter.C
vtkLinesFromOriginalCells.C
vtkMultiSplitter.C
vtkOBJWriter.C
vtkOnionPeelFilter.C
vtkPPMWriter.C
vtkPolyDataOnionPeelFilter.C
vtkRGBWriter.C
vtkRectilinearGridFacelistFilter.C
vtkRectilinearLinesNoDataFilter.C
vtkSlicer.C
vtkStructuredGridFacelistFilter.C
vtkSurfaceFilter.C
vtkSurfaceFromVolume.C
vtkTensorReduceFilter.C
vtkUniqueFeatureEdges.C
vtkUnstructuredGridRelevantPointsFilter.C
vtkVectorGlyph.C
vtkVectorReduceFilter.C
vtkVertexFilter.C
vtkVisItCellDataToPointData.C
vtkVisItClipper.C
vtkVisItContourFilter.C
vtkVisItCutter.C
vtkVisItExtractGrid.C
vtkVisItExtractRectilinearGrid.C
vtkVisItFeatureEdges.C
vtkVisItGlyph3D.C
vtkVisItInterpolatedVelocityField.C
vtkVisItPolyDataNormals.C
vtkVisItProbeFilter.C
vtkVisItRectilinearGrid.C
vtkVisItSTLWriter.C
vtkVisItScalarTree.C
vtkVisItSplitter.C
vtkVisItStructuredGrid.C
vtkVisItStructuredGridNormals.C
#vtkVisItTIFFWriter.C
vtkVisItTensorGlyph.C
vtkVisItTubeFilter.C
vtkVolumeFromVolume.C
vtkVolumeFromCSGVolume.C
)

INCLUDE_DIRECTORIES(
${VISIT_BINARY_DIR}
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${BOOST_INCLUDE_DIR}
)
#robert maynard: todo, find a better solution
if(HAVE_BILIB)
  add_definitions(-D __USE_ISOC99)
endif(HAVE_BILIB)

#fix the include directories to have the parser first no matter what
get_directory_property(tmp_include_dirs INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES
  "${VISIT_SOURCE_DIR}/parser;${tmp_include_dirs}")

VTK_ADD_LIBRARY(visit_vtk ${VISIT_VTK_SOURCES})

target_link_libraries(visit_vtk
  LINK_PUBLIC
    visitcommon
    lightweight_visit_vtk
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
  LINK_PRIVATE
    vtkFiltersExtraction
    vtkFiltersFlowPaths
    vtkImagingHybrid
    vtkInteractionStyle
    vtkIOLegacy
    vtkRendering${VTK_RENDERING_BACKEND}
  )

VISIT_INSTALL_TARGETS(visit_vtk)
