
SET(SOURCES
avtCEAucdFileFormat.C
FichierU.C
vtkCEAucdReader.C
)
ADD_VISIT_READER(VisItCEAucdReader "1.0"
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtCEAucdFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
