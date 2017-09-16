
set(SOURCES
  avtPoint3DFileFormat.C
  )
ADD_VISIT_READER(VisItPoint3DReader "1.0"
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtPoint3DFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
