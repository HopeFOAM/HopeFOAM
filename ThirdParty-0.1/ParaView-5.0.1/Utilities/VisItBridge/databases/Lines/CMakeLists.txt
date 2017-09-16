
set(SOURCES
  avtLinesFileFormat.C
  )
ADD_VISIT_READER(VisItLinesReader "1.0"
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtLinesFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
