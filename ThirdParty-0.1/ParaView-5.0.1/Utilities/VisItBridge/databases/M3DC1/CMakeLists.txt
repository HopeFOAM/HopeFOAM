
set(SOURCES
  avtM3DC1Field.C
  avtM3DC1FileFormat.C
  avtM3DC1FileFormat.h
  avtM3DC1Options.C
  avtM3DC1Options.h
  M3DC1PluginInfo.h
  M3DC1PluginInfo.C
  M3DC1EnginePluginInfo.C
  M3DC1MDServerPluginInfo.C
  M3DC1CommonPluginInfo.C
)

ADD_VISIT_READER(VisItM3DC1Reader "1.0" 
  VISIT_READER_TYPE "MTSD"
  VISIT_READER_NAME "avtM3DC1FileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
