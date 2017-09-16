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
    Foam::dgTreeUnit

Description
    The basic unit of a tree

SourceFiles
    dgTreeUnit.C

Author
    Feng Yongquan (yqfeng0418@163.com)

\*---------------------------------------------------------------------------*/

#ifndef dgTreeUnit_H
#define dgTreeUnit_H

#include "labelList.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;


namespace Foam
{



/*---------------------------------------------------------------------------*\
                          Class dgTreeUnit Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class dgTreeUnit{
private:

    //The value in this unit
    shared_ptr<Type> value_;

    //The point to its own father node
    shared_ptr<dgTreeUnit<Type>> father_;

    //The point to its nxt node, used for the iterator
    shared_ptr<dgTreeUnit<Type>> nxt_;

    //The list points to its own sub unit
    List<shared_ptr<dgTreeUnit<Type>>> sub_;

public:
    //Construct from nothing
    dgTreeUnit();

    //Construct from given value
    dgTreeUnit(const shared_ptr<Type>);

    //Construct from given value and the number of sub units
    dgTreeUnit(const shared_ptr<Type>, const label);

    ~dgTreeUnit(){};

    //Memberfunction for access the variable
    inline const Type& value() const
    {
        return *value_;
    }

    inline Type& value()
    {
        return *value_;
    }

    inline shared_ptr<dgTreeUnit<Type>> father()
    {
        return father_;
    }

    inline shared_ptr<dgTreeUnit<Type>> next()
    {
        return nxt_;
    }

    inline List<shared_ptr<dgTreeUnit<Type>>>& sub()
    {
        return sub_;
    }

    //Member function for set value for variable
    void setValue(const shared_ptr<Type>);

    void setFather(const shared_ptr<dgTreeUnit<Type>>);

    void setNxt(const shared_ptr<dgTreeUnit<Type>>);

    void setSubSize(const label);

    void setSub(const label, const shared_ptr<dgTreeUnit<Type>>);

    //return the number of node from this node
    const label leafSize() const;

    //refine the cell, and correct the point of the sub cell,
    //next cell and father cell.
    void refine();

};


}

#ifdef NoRepository
    #include "dgTreeUnit.C"
#endif


#endif
