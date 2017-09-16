
set(SOURCES
  avtFluentFileFormat.h
  avtFluentFileFormat.C
  )

ADD_VISIT_READER(VisItFluentReader "1.0"  
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtFluentFileFormat"    
  SERVER_SOURCES ${SOURCES}  
  )