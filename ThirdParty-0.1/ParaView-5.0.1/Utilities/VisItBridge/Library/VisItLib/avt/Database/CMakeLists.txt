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
#
#****************************************************************************/
PROJECT(AVT_DATABASE)

SET(DATABASE_SOURCES
Database/avtDatabase.C
Database/avtDatabaseFactory.C
Database/avtDatasetCollection.C
Database/avtDatasetDatabase.C
Database/avtFileDescriptorManager.C
Database/avtGenericDatabase.C
Database/avtIOInformation.C
#Database/avtPolygonToTrianglesTesselator.C
Database/avtPolyhedralSplit.C
Database/avtSILGenerator.C
Database/avtSourceFromDatabase.C
Database/avtVariableCache.C
Database/avtTransformManager.C
)

SET(FORMATS_SOURCES
Formats/avtFileFormat.C
Formats/avtFileFormatInterface.C
Formats/avtMTSDFileFormat.C
Formats/avtMTSDFileFormatInterface.C
Formats/avtMTMDFileFormat.C
Formats/avtMTMDFileFormatInterface.C
Formats/avtSTMDFileFormat.C
Formats/avtSTMDFileFormatInterface.C
Formats/avtSTSDFileFormat.C
Formats/avtSTSDFileFormatInterface.C
)

SET(GHOST_SOURCES
Ghost/StructuredBoundary.C
Ghost/avtDomainBoundaries.C
Ghost/avtIsenburgSGG.C
Ghost/avtStreamingGhostGenerator.C
Ghost/avtStructuredDomainBoundaries.C
Ghost/avtUnstructuredDomainBoundaries.C
Ghost/avtNekDomainBoundaries.C
Ghost/avtStructuredDomainNesting.C
Ghost/avtUnstructuredPointBoundaries.C
)

SET(AVTDATABASE_SOURCES
${DATABASE_SOURCES}
${FORMATS_SOURCES}
${GHOST_SOURCES}
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${AVT_DATABASE_SOURCE_DIR}/Database
${AVT_DATABASE_SOURCE_DIR}/Formats
${AVT_DATABASE_SOURCE_DIR}/Ghost
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${AVT_DBATTS_SOURCE_DIR}/MetaData
${AVT_DBATTS_SOURCE_DIR}/SIL
${AVT_PIPELINE_SOURCE_DIR}/AbstractFilters
${AVT_PIPELINE_SOURCE_DIR}/Data
${AVT_PIPELINE_SOURCE_DIR}/Pipeline
${AVT_PIPELINE_SOURCE_DIR}/Sinks
${AVT_PIPELINE_SOURCE_DIR}/Sources
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VTK_INCLUDE_DIRS}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${VTK_LIBRARY_DIRS})
#LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${VTK_LIBRARY_DIRS} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR})


IF(NOT VISIT_PARALLEL)

#********************************* SERIAL ************************************
    VTK_ADD_LIBRARY(avtdatabase_ser ${AVTDATABASE_SOURCES})
    target_link_libraries(avtdatabase_ser
                         LINK_PUBLIC
                            avtdbatts
                            avtpipeline_ser
                            visit_vtk
                         LINK_PRIVATE
                            ${AVTMIR_SER}
                            )
    VISIT_INSTALL_TARGETS(avtdatabase_ser)

ELSE()

#********************************* PARALLEL **********************************
    ADD_PARALLEL_LIBRARY(avtdatabase_par ${AVTDATABASE_SOURCES})
    target_link_libraries(avtdatabase_par
                         LINK_PUBLIC
                            avtdbatts
                            avtpipeline_par
                            visit_vtk
                        LINK_PRIVATE
                            ${AVTMIR_PAR}
                            )
    VISIT_INSTALL_TARGETS(avtdatabase_par)

ENDIF()
