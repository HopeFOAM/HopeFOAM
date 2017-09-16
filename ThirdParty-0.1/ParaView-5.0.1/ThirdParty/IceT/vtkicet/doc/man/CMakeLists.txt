# Copyright 2009 Sandia Corporation.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.

IF (NOT ICET_INSTALL_NO_DEVELOPMENT)
  SET(manSections 3)
  FOREACH (section ${manSections})
    FILE(GLOB manFiles
      "${CMAKE_CURRENT_SOURCE_DIR}/man${section}/*.${section}"
      )
    INSTALL(FILES ${manFiles}
      DESTINATION "${ICET_INSTALL_MAN_DIR}/man${section}"
      )
  ENDFOREACH(section)
ENDIF (NOT ICET_INSTALL_NO_DEVELOPMENT)
