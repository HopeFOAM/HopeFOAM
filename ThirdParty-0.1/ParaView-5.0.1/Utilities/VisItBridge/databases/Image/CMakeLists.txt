
set(SOURCES
  vtkStimulateReader.C
  avtImageFileFormat.C
  avtImageOptions.C
  avtImagePluginWriter.C
  ImageMDServerPluginInfo.C
  ImagePluginInfo.C
  ImageCommonPluginInfo.C
  ImageEnginePluginInfo.C
)

ADD_VISIT_READER(VisItImageReader "1.0" 
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtImageFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
