
set(SOURCES
  avtFLASHFileFormat.C
  avtFLASHFileFormat.h
  avtFLASHOptions.C
  avtFLASHOptions.h
  FLASHPluginInfo.h
  FLASHPluginInfo.C
  FLASHEnginePluginInfo.C
  FLASHMDServerPluginInfo.C
  FLASHCommonPluginInfo.C
)

ADD_VISIT_READER(VisItFLASHReader "1.0" 
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtFLASHFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
