
SET(SOURCES
avtProteinDataBankFileFormat.C
avtProteinDataBankFileFormat.h
avtProteinDataBankOptions.C
avtProteinDataBankOptions.h
ProteinDataBankPluginInfo.h
ProteinDataBankPluginInfo.C
ProteinDataBankEnginePluginInfo.C
ProteinDataBankMDServerPluginInfo.C
ProteinDataBankCommonPluginInfo.C
)

ADD_VISIT_READER(VisItProteinDataBankReader "1.0" 
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtProteinDataBankFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )

