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
    Foam::dgSubList

Description
    Store the data of part of the field

SourceFiles
    SubListI.H

Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef dgSubList_H
#define dgSubList_H

#include "SubList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class dgSubList Declaration
\*---------------------------------------------------------------------------*/

template<class T>
class dgSubList
:
    public UList<T>
{

public:

    // Static Member Functions

        //- Return a null SubList
        inline static const dgSubList<T>& null();


    // Constructors

        //- Construct from UList and sub-list size
        inline dgSubList();

        //- Construct from UList, sub-list size and start index
        inline dgSubList
        (
            const UList<T>& list,
            const label subSize,
            const label startIndex
        );


    // Member operators

        //- Allow cast to a const List<T>&
        inline operator const Foam::List<T>&() const;

        //- Assignment of all entries to the given sub-list
        inline void operator=(const dgSubList<T>&);

        //- Assignment of all entries to the given list
        inline void operator=(const UList<T>&);

        //- Assignment of all entries to the given value
        inline void operator=(const T&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "dgSubListI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

