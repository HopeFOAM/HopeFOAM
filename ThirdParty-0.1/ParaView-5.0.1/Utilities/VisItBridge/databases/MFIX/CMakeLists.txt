

set(SOURCES
  avtMFIXFileFormat.C
  avtMFIXFileFormat.h
  avtMFIXOptions.C
  avtMFIXFileFormat.C
  MFIXPluginInfo.C
  MFIXEnginePluginInfo.C
  MFIXCommonPluginInfo.C
  MFIXMDServerPluginInfo.C
  )


ADD_VISIT_READER(VisItMFIXReader "1.0"
  VISIT_READER_TYPE "MTMD"
  VISIT_READER_NAME "avtMFIXFileFormat"
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
