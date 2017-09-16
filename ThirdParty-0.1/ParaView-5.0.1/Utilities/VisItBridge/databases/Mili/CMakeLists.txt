
set(SOURCES
  avtMiliFileFormat.C
  )
ADD_VISIT_READER(VisItMiliReader "1.0"
  VISIT_READER_TYPE "MTMD"
  VISIT_READER_NAME "avtMiliFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
