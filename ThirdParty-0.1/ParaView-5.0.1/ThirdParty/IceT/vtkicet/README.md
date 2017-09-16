> *****************************************************************************
>
> README and installation instructions for IceT
>
> Author: Kenneth Moreland (kmorel@sandia.gov)
>
> Copyright 2003 Sandia Coporation
>
> Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
> the U.S. Government retains certain rights in this software.
>
> This source code is released under the [New BSD License](bsd).
>
> *****************************************************************************

Welcome to the IceT build process.  IceT uses [CMake](cmake) to automatically
tailor itself to your system, so compiling should be relatively painless.
Before building IceT you will need to install CMake on your system.  You
can get CMake from [here](cmake-download).

Once CMake is installed and the IceT source is extracted, run the
interactive CMake configuration tool.  On UNIX, run `ccmake`.  On Win32, run
the CMake program on the desktop or in the start menu.  Note that when
using the interactive configuration tool, you will need to *configure*
several times before you can generate the build files.  This is because as
more information is retrieved, futher options are revealed.

After CMake generates build files, compile the applications as applicable
for your system.

IceT is released under the [New BSD License](bsd).  Any contributions to IceT will
also be considered to fall under this license, and it is the responsibility
of the authors to secure the necessary permissions before contributing.

[bsd]: http://opensource.org/licenses/BSD-3-Clause
[cmake]: http://www.cmake.org/
[cmake-download]: http://www.cmake.org/download/
