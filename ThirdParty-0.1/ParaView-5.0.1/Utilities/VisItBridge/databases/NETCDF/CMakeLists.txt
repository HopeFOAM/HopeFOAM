PROJECT(VisItNETCDF)

#############
## WARNING ##
#############
#VisIt interface framework is really weird. This is a quick first
#pass at figuring out a semi easy way to emulate it in ParaView world

#macro has to return true/false ( users needs to write it)
#this macro is called from the CanReadFile method of each class
set(INTERFACE_MACRO_CALL "ValidNETCDFFile(fname)")
set(INTERFACE_MACRO_FILE "ValidNETCDFFile") #needs to be a .h


SET(SOURCES
avtBasicNETCDFFileFormat.C
avtADAPTFileFormat.C
avtLODIFileFormat.C
avtLODIParticleFileFormat.C
avtFVCOM_STSDFileFormat.C
avtFVCOMParticleFileFormat.C
avtFVCOM_MTSDFileFormat.C
avtFVCOM_MTMDFileFormat.C
avtCCSMFileFormat.C
avtBOUTFileFormat.C
avtNETCDFReaderBase.C
avtBasicNETCDFReader.C
avtCCSMReader.C
avtFVCOMReader.C
NETCDFFileObject.C
)

#needed by cmake to properly generate the correct readers in the interface collection
#for each reader in the interface we need: name, type, and name of header it is defined in
set(READER_NAMES
avtADAPTFileFormat
avtLODIFileFormat
avtLODIParticleFileFormat
avtFVCOM_STSDFileFormat
avtFVCOM_MTMDFileFormat
avtFVCOM_MTSDFileFormat
avtFVCOMParticleFileFormat
avtCCSM_MTSD_FileFormat
avtCCSM_STSD_FileFormat
avtBOUTFileFormat
)

set(READER_TYPES
STSD
MTSD
MTSD
STSD
MTMD
MTSD
MTSD
MTSD
STSD
MTMD
)

set(READER_INCLUDES
avtADAPTFileFormat
avtLODIFileFormat
avtLODIParticleFileFormat
avtFVCOM_STSDFileFormat
avtFVCOM_MTMDFileFormat
avtFVCOM_MTSDFileFormat
avtFVCOMParticleFileFormat
avtCCSMFileFormat
avtCCSMFileFormat
avtBOUTFileFormat
)

ADD_VISIT_INTERFACE_READER(VisItNETCDF "1.0"
  VISIT_READER_NAMES ${READER_NAMES}
  VISIT_READER_TYPES ${READER_TYPES}
  VISIT_READER_INCLUDES ${READER_INCLUDES}
  VISIT_INTERFACE_CALL ${INTERFACE_MACRO_CALL}
  VISIT_INTERFACE_FILE ${INTERFACE_MACRO_FILE}
  VISIT_INTERFACE_EXEMPT_CLASSES ${INTERFACE_EXEMPT_CLASSES}
  SERVER_SOURCES ${SOURCES}
  )
