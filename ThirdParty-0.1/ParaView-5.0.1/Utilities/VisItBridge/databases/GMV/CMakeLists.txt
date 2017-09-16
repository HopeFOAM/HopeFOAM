
set(SOURCES
  avtGMVFileFormat.C
  avtGMVFileFormat.h
  GMVEnginePluginInfo.C
  GMVMDServerPluginInfo.C
  GMVCommonPluginInfo.C
  GMVPluginInfo.C
  gmvMaterialEncoder.C
  gmvPolyhedralSplit.C
  gmvread.c
)


ADD_VISIT_READER(VisItGMVReader "1.0"
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtGMVFileFormat"
  SERVER_SOURCES ${SOURCES}
  )
