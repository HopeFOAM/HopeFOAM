
set(SOURCES
  avtSpheralFileFormat.C
  )
ADD_VISIT_READER(VisItSpheralReader "1.0"
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtSpheralFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
