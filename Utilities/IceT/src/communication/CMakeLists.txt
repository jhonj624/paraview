# $Id: CMakeLists.txt,v 1.3 2005/06/30 13:37:55 king Exp $
#
## Copyright 2003 Sandia Coporation
## Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
## license for use of this work by or on behalf of the U.S. Government.
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that this Notice and any statement
## of authorship are reproduced on all copies.
#

SET(ICET_MPI_SRCS
        mpi.c
)

ADD_LIBRARY(icet_mpi ${ICET_MPI_SRCS})

TARGET_LINK_LIBRARIES(icet_mpi icet)


IF(NOT ICET_INSTALL_NO_LIBRARIES)
  INSTALL_TARGETS(${ICET_INSTALL_LIB_DIR} icet_mpi)
ENDIF(NOT ICET_INSTALL_NO_LIBRARIES)
IF(NOT ICET_INSTALL_NO_DEVELOPMENT)
  INSTALL_FILES(${ICET_INSTALL_INCLUDE_DIR}/GL .h
    ${ICET_SOURCE_DIR}/src/include/GL/ice-t_mpi)
ENDIF(NOT ICET_INSTALL_NO_DEVELOPMENT)
