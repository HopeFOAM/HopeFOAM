
SET(SOURCES
avtTetradFileFormat.h
avtTetradFileFormat.C
)

ADD_VISIT_READER(VisItTetradReader "1.0"  
  VISIT_READER_TYPE "MTSD"
  VISIT_READER_NAME "avtTetradFileFormat"    
  SERVER_SOURCES ${SOURCES}  
  )
