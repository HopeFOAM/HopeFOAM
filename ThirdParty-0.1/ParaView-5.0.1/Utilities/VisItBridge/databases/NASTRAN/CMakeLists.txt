
set(SOURCES
  avtNASTRANFileFormat.C
  avtNASTRANFileFormat.h
  avtNASTRANOptions.C
  avtNASTRANOptions.h
  NASTRANPluginInfo.h
  NASTRANPluginInfo.C
  NASTRANEnginePluginInfo.C
  NASTRANMDServerPluginInfo.C
  NASTRANCommonPluginInfo.C
)

ADD_VISIT_READER(VisItNASTRANReader "1.0" 
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtNASTRANFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
