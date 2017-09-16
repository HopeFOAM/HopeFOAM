set(PARADIS_SOURCES
  avtparaDISFileFormat.C
  avtparaDISOptions.C
  paraDISCommonPluginInfo.C
  paraDISEnginePluginInfo.C
  paraDISMDServerPluginInfo.C
  paraDISPluginInfo.C
  Dumpfile.C
  parallelParaDIS.C
  paraDIS_lib/paradis.C
  paraDIS_lib/paradis_c_interface.C
  RC_c_lib/args.c
  RC_c_lib/debugutil.c
  RC_c_lib/fileutils.c
  RC_c_lib/inventor.c
  RC_cpp_lib/RangeList.C
  RC_cpp_lib/timer.C
  RC_cpp_lib/pathutil.C
)

set(PARADIS_TECPLOT_SOURCES
  avtparaDIS_tecplotFileFormat.C
  RC_c_lib/args.c
  RC_c_lib/debugutil.c
  RC_c_lib/signals.c
  RC_c_lib/fileutils.c
  RC_c_lib/inventor.c
  RC_cpp_lib/RangeList.C
  RC_cpp_lib/timer.C
  RC_cpp_lib/pathutil.C
)

ADD_VISIT_READER(VisItParaDISReader "1.0"
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtparaDISFileFormat"
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${PARADIS_SOURCES}
  )

ADD_VISIT_READER(VisItParaDISTecplotReader "1.0"
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtparaDIS_tecplotFileFormat"
  VISIT_READER_USES_OPTIONS OFF
  SERVER_SOURCES ${PARADIS_TECPLOT_SOURCES}
  )
