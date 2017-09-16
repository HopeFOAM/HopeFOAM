
set(SOURCES
  avtTSurfFileFormat.C
  )
ADD_VISIT_READER(VisItTSurfReader "1.0"
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtTSurfFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
