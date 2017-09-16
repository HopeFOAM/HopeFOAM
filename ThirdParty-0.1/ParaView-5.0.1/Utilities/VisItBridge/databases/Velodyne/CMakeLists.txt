
set(SOURCES
  avtVelodyneFileFormat.C
  VelodyneReader.C
  )
ADD_VISIT_READER(VisItVelodyneReader "1.0"
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtVelodyneFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
