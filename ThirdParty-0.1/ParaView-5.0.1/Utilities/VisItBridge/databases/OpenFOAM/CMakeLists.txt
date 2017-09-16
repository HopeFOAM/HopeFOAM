
set(SOURCES
  avtOpenFOAMFileFormat.C
  avtOpenFOAMOptions.C
  OpenFOAMCommonPluginInfo.C
  OpenFOAMEnginePluginInfo.C
  OpenFOAMMDServerPluginInfo.C
  OpenFOAMPluginInfo.C
  visit_vtkOpenFOAMReader.cxx
  visit_vtkPOpenFOAMReader.cxx
  )

ADD_VISIT_READER(VisItOpenFOAMReader "1.0"
  VISIT_READER_TYPE "MTMD"
  VISIT_READER_NAME "avtOpenFOAMFileFormat"
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
