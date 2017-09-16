
SET(SOURCES
  avtPixieFileFormat.C
  avtPixieOptions.C
  PixieCommonPluginInfo.C
  PixieEnginePluginInfo.C
  PixieMDServerPluginInfo.C
  PixiePluginInfo.C
  )

ADD_VISIT_READER(VisItPixieReader "1.0"
  VISIT_READER_TYPE "MTSD"
  VISIT_READER_NAME "avtPixieFileFormat"
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
