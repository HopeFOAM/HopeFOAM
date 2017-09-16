SET(SOURCES
avtSiloWriter.C
avtSiloFileFormat.C
avtSiloMBObjectCache.C
avtSiloOptions.C
SiloPluginInfo.C
SiloCommonPluginInfo.C
SiloEnginePluginInfo.C
SiloMDServerPluginInfo.C
)

if(HAVE_LIBSILO)
  ADD_VISIT_READER(VisItSiloReader "1.0"
    VISIT_READER_TYPE "STMD"
    VISIT_READER_NAME "avtSiloFileFormat"
    VISIT_READER_USES_OPTIONS ON
    SERVER_SOURCES ${SOURCES}
    )
endif(HAVE_LIBSILO)
