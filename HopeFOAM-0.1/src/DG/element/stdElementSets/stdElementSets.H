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
    Foam::stdElementSets

Description
    Hash Table from names to all the stdElemets that will be uttilized
    during the discritesation

SourceFiles
    stdElementSets.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef stdElementSets_H
#define stdElementSets_H

#include "HashTable.H"
#include "stdElement.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
  Class stdElementSets Declaration
\*---------------------------------------------------------------------------*/

class stdElementSets
{
	//- Hash Table Data
	HashTable<stdElement*, word, string::hash> dataSets_;

public:

	// Declare name of the class and its debug switch
    ClassName("stdElementSets");

    // Constructors
    stdElementSets();

    //- Destructor
    virtual ~stdElementSets();

    // Member Functions

    	//- get stdElement from dataSets by name and order
    	stdElement* getElement(label nOrder, word cellShape);

      //-
      void eraseElement(label nOrder, word cellShape);
};

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //