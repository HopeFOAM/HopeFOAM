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

\*---------------------------------------------------------------------------*/

#include "dgTreeUnit.H"

using std::shared_ptr;
using std::make_shared;

template<class Type>
Foam::dgTreeUnit<Type>::dgTreeUnit()
:
    father_(NULL),
    nxt_(NULL),
    sub_(1)
{
    sub_[0] = NULL;
}

template<class Type>
Foam::dgTreeUnit<Type>::dgTreeUnit(const shared_ptr<Type> value)
:   
    value_(value),
    father_(NULL),
    nxt_(NULL),
    sub_(1)
{
    sub_[0] = NULL;
}

template<class Type>
Foam::dgTreeUnit<Type>::dgTreeUnit(const shared_ptr<Type> value, const label size)
:
    value_(value),
    father_(NULL),
    nxt_(NULL),
    sub_(size)
{
    forAll(sub_, unitI)
    {
        sub_[unitI] = NULL;
    }
}

template<class Type>
void Foam::dgTreeUnit<Type>::setValue(const shared_ptr<Type> value)
{
    value_ = value;
}

template<class Type>
void Foam::dgTreeUnit<Type>::setFather(const shared_ptr<dgTreeUnit<Type>> father)
{
    father_ = father;
}

template<class Type>
void Foam::dgTreeUnit<Type>::setNxt(const shared_ptr<dgTreeUnit<Type>> nxt)
{
    nxt_ = nxt;
}

template<class Type>
void Foam::dgTreeUnit<Type>::setSubSize(const label size)
{
    sub_.setSize(size);
}

template<class Type>
void Foam::dgTreeUnit<Type>::setSub(const label unitI, const shared_ptr<dgTreeUnit<Type>> subI){
    if(sub_.size() <= unitI){
        FatalErrorIn("dgTreeUnit::setSub(const label unitI, const Type* subI)")
                        << "the size of sub_ is less than "
                        <<unitI<< " in dgTreeUnit"<<endl
                        << abort(FatalError);
    }
    sub_[unitI] = subI;
}

template<class Type>
void Foam::dgTreeUnit<Type>::refine()
{
    const List<Type>& subValues = value().refine();
    const label subSize = subValues.size();
    sub_.setSize(subSize);
    forAll(subValues, valueI){
        sub_[valueI] = new dgTreeUnit(subValues[valueI]);
        sub_[valueI]->setFather(this);
    }

    for(int i=0 ; i<subSize-1 ; i++){
        sub_[i]->setNxt(sub_[i+1]);
    }
}

template<class Type>
const Foam::label Foam::dgTreeUnit<Type>::leafSize() const
{
    label num=0;
    Type* tmp = this;
    while(tmp != NULL){
        if(tmp->sub()[0] != NULL){
            tmp = tmp->sub()[0];
        }
        else{
            num++;
            if(tmp->next() != NULL){
                tmp = tmp->next();
            }
            else{
                while(tmp->next() == NULL){
                    tmp = tmp->father();
                    if(tmp == NULL){
                        return num;
                    }
                }
                tmp = tmp->next();
            }
        }
    }
    return num;
}