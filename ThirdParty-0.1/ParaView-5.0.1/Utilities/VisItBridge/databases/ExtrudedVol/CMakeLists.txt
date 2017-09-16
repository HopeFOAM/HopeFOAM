
set(SOURCES
  avtExtrudedVolFileFormat.C
  avtExtrudedVolWriter.C
  avtExtrudedVolOptions.C
  avtExtrudedVolOptions.h
  ExtrudedVolPluginInfo.h
  ExtrudedVolPluginInfo.C
  ExtrudedVolEnginePluginInfo.C
  ExtrudedVolMDServerPluginInfo.C
  ExtrudedVolCommonPluginInfo.C  
  )
ADD_VISIT_READER(VisItExtrudedVolReader "1.0"
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtExtrudedVolFileFormat"    
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
