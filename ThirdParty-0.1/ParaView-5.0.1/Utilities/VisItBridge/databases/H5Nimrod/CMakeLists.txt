
set(SOURCES
  avtH5NimrodFileFormat.C
  H5utils.C
  )
ADD_VISIT_READER(VisItH5NimrodReader "1.0"
  VISIT_READER_TYPE "MTSD"
  VISIT_READER_NAME "avtH5NimrodFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
