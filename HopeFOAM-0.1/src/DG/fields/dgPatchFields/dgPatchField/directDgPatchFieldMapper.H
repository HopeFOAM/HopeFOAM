/*--------------------------------------------------------------------------------------
|     __  ______  ____  ______ |                                                       |
|    / / / / __ \/ __ \/ ____/ | HopeFOAM: High Order Parallel Extensible CFD Software |
|   / /_/ / / / / /_/ / __/    |                                                       |
|  / __  / /_/ / ____/ /___    |                                                       |
| /_/ /_/\____/_/   /_____/    | Copyright(c) 2017-2017 The Exercise Group, AMS, China.|
|                              |                                                       |
----------------------------------------------------------------------------------------

License
    This file is part of HopeFOAM, which is developed by Exercise Group, Innovation 
    Institute for Defence Science and Technology, the Academy of Military Science (AMS), China.

    HopeFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    HopeFOAM is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HopeFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::directDgPatchFieldMapper

Description
    direct dgPatchFieldMapper

\*---------------------------------------------------------------------------*/

#ifndef directDgPatchFieldMapper_H
#define directDgPatchFieldMapper_H

#include "dgPatchFieldMapper.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class directDgPatchFieldMapper Declaration
\*---------------------------------------------------------------------------*/

class directDgPatchFieldMapper
:
    public dgPatchFieldMapper
{

    //- Addressing from new back to old
    const labelUList& directAddressing_;

    //- Does map contain any unmapped values
    bool hasUnmapped_;


public:

    // Constructors

        //- Construct given addressing
        directDgPatchFieldMapper(const labelUList& directAddressing)
        :
            directAddressing_(directAddressing),
            hasUnmapped_(false)
        {
            if (directAddressing_.size() && min(directAddressing_) < 0)
            {
                hasUnmapped_ = true;
            }
        }

    //- Destructor
    virtual ~directDgPatchFieldMapper()
    {}


    // Member Functions

        label size() const
        {
            return directAddressing_.size();
        }

        bool direct() const
        {
            return true;
        }

        bool hasUnmapped() const
        {
            return hasUnmapped_;
        }

        const labelUList& directAddressing() const
        {
            return directAddressing_;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
