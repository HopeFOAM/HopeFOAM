SET(SOURCES
avtChomboFileFormat.C
avtChomboFileFormat.h
avtChomboOptions.C
avtChomboOptions.h
ChomboPluginInfo.h
ChomboPluginInfo.C
ChomboEnginePluginInfo.C
ChomboMDServerPluginInfo.C
ChomboCommonPluginInfo.C
)

ADD_VISIT_READER(VisItChomboReader "1.0" 
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtChomboFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )