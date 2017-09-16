
SET(SOURCES
avtSAMRAIFileFormat.C
avtSAMRAIFileFormat.h
AutoArray.h
SAMRAIEnginePluginInfo.C
SAMRAIPluginInfo.C
SAMRAIPluginInfo.h
SAMRAICommonPluginInfo.C
SAMRAIMDServerPluginInfo.C
)

ADD_VISIT_READER(VisItSAMRAIReader "1.0"  
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtSAMRAIFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )