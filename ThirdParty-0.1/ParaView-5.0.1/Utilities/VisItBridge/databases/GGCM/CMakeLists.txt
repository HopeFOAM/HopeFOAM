
set(SOURCES
  avtGGCMFileFormat.C
  avtGGCMOptions.C
  GGCMCommonPluginInfo.C
  GGCMEnginePluginInfo.C
  GGCMMDServerPluginInfo.C
  GGCMPluginInfo.C
  GGCMPluginInfo.h
  ggcm_grid.C
  ggcm_grid_gse.C
  ggcm_field_matrix.C
  ggcm_mhd_gse.C
  ggcm_read_mhd.C
  matrix.C
)

ADD_VISIT_READER(VisItGGCMReader "1.0" 
  VISIT_READER_TYPE "MTSD"
  VISIT_READER_NAME "avtGGCMFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
