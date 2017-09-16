
set(SOURCES
  avtNek5000FileFormat.C
  avtNek5000Options.C
  Nek5000CommonPluginInfo.C
  Nek5000EnginePluginInfo.C
  Nek5000MDServerPluginInfo.C
  Nek5000PluginInfo.C
  )
ADD_VISIT_READER(VisItNek5000Reader "1.0"
  VISIT_READER_TYPE "MTMD"
  VISIT_READER_NAME "avtNek5000FileFormat"
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
