## Copyright 2003 Sandia Coporation
## Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
## the U.S. Government retains certain rights in this software.
##
## This source code is released under the New BSD License.
#

# These source files are actually compiled in the Core IceT source
# directory.  They are separated in a separate directory for convenience.
ADD_CUSTOM_TARGET(strategies_target ALL
  DEPENDS IceTCore
  )
