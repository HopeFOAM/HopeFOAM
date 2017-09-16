
set(SOURCES
  avtGadgetFileFormat.C
  )
ADD_VISIT_READER(VisItGadgetReader "1.0"
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtGadgetFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
