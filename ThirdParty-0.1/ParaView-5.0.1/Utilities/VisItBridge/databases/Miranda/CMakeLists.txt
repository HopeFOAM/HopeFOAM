
set(SOURCES
  avtMirandaFileFormat.C
  avtMirandaFileFormat.h
  avtMirandaOptions.C
  avtMirandaOptions.h
  MirandaPluginInfo.h
  MirandaPluginInfo.C
  MirandaEnginePluginInfo.C
  MirandaMDServerPluginInfo.C
  MirandaCommonPluginInfo.C
)

ADD_VISIT_READER(VisItMirandaReader "1.0" 
  VISIT_READER_TYPE "MTMD"
  VISIT_READER_NAME "avtMirandaFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
