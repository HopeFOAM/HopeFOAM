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

#include "dgTree.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;

template<class Type>
Foam::dgTree<Type>::dgTree()
:
    baseLst_(0),
    size_(0),
    leafSize_(0)
{

}

template<class Type>
Foam::dgTree<Type>::dgTree(const label size, const List<shared_ptr<dgTreeUnit<Type>>>& inforLst)
:
    baseLst_(size),
    size_(size),
    leafSize_(size)

{
    forAll(inforLst, itemI)
    {
        baseLst_[itemI] = inforLst[itemI]; 
    }
}

template<class Type>
Foam::dgTree<Type>::~dgTree()
{

}


template<class Type>
void Foam::dgTree<Type>::initial(const label size, List<shared_ptr<dgTreeUnit<Type>>>& inforLst)
{
    baseLst_.setSize(size);
    size_ = size;
    leafSize_ = size;

    forAll(inforLst, itemI)
    {
        baseLst_[itemI] = inforLst[itemI]; 
    }
}


template<class Type>
void Foam::dgTree<Type>::refine(shared_ptr<dgTreeUnit<Type>> refineCell){
    refineCell->refine();
}


// * * * * * * * * * * * * * * * iterator base * * * * * * * * * * * * * * * //
template<class Type>
inline Foam::dgTree<Type>::iteratorBase::iteratorBase()
:
    treePtr_(0),
    entryPtr_(0),
    index_(0),
    partIndex_(0)
{}

template<class Type>
inline Foam::dgTree<Type>::iteratorBase::iteratorBase
(
    const dgTree<Type>* treePtr
)
:
    treePtr_(treePtr),
    entryPtr_(0),
    index_(0),
    partIndex_(0)
{
    if(treePtr_->baseLst_.size()>0){
        entryPtr_ = treePtr_->baseLst_[0];
    }
    else{
        FatalErrorIn("dgTree::iteratorBase::iteratorBase()")
                << "dgTree is not initialized"<<endl
                << abort(FatalError);
    }
}

template<class Type>
inline Foam::dgTree<Type>::iteratorBase::iteratorBase
(
    const dgTree<Type>* treePtr,
    const labelList& partIndex
)
:
    treePtr_(treePtr),
    entryPtr_(0),
    index_(0),
    partIndex_(partIndex)
{
    if(partIndex.size() == 0){
        WarningIn("dgTree<Type>::iteratorBase::iteratorBase(const dgTree<Type>*, const labelList&)")
            << "Using zero sized 'partIndex'"
            << nl << endl;
    }
    if(treePtr_->baseLst_.size()>0){
        entryPtr_ = treePtr_->baseLst_[partIndex_[0]];
    }
    else{
        FatalErrorIn("dgTree::iteratorBase::iteratorBase()")
                << "dgTree is not initialized"<<endl
                << abort(FatalError);
    }
}

template<class Type>
inline void Foam::dgTree<Type>::iteratorBase::increment()
{   
 
    if(entryPtr_->sub()[0] != NULL)
    {
        entryPtr_ = entryPtr_->sub()[0];
    }
    //else determine whether entryPtr_ has a brother node.
    else if(entryPtr_->next() != NULL)
    {
        entryPtr_ = entryPtr_->next();
    }
    //Backtrack, and find the brother node of the father node
    else
    {
        while(true)
        {
            entryPtr_ = entryPtr_->father();
            if(entryPtr_ == NULL) break;
            if(entryPtr_->next() != NULL)
            {
                entryPtr_ = entryPtr_->next();
                break;
            }
        }
        //if there is no node remaining no visiting, access the next member of the baseLst_
        if(entryPtr_ == NULL){
            index_ ++;
            if(partIndex_.size()>0){
                if(index_ < partIndex_.size()){
                    entryPtr_ = treePtr_->baseLst_[partIndex_[index_]];
                }
            }
            else{
                if(index_ < treePtr_->baseLst_.size()){
                    entryPtr_ = treePtr_->baseLst_[index_];
                }
            }
        }
    }

}

template<class Type>
inline shared_ptr<Foam::dgTreeUnit<Type>> Foam::dgTree<Type>::iteratorBase::object()
{
    return entryPtr_;
}

template<class Type>
inline const shared_ptr<Foam::dgTreeUnit<Type>> Foam::dgTree<Type>::iteratorBase::cobject() const
{
    return entryPtr_;
}

template<class Type>
inline bool Foam::dgTree<Type>::iteratorBase::operator==
(
    const iteratorEnd&
) const
{
    if(entryPtr_ == NULL) return true;
    return false;
}


template<class Type>
inline bool Foam::dgTree<Type>::iteratorBase::operator!=
(
    const iteratorEnd&
) const
{
    if(entryPtr_ == NULL) return false;
    return true;
    //return entryPtr_;
}



// * * * * * * * * * * * * * * * * tree iterator  * * * * * * * * * * * * * * //

template<class Type>
inline Foam::dgTree<Type>::treeIterator::treeIterator()
:
    iteratorBase()
{}


template<class Type>
inline Foam::dgTree<Type>::treeIterator::treeIterator
(
    const iteratorEnd&
)
:
    iteratorBase()
{}

template<class Type>
inline Foam::dgTree<Type>::treeIterator::treeIterator
(
    const dgTree<Type>* treePtr
)
:
    iteratorBase(treePtr)
{}


template<class Type>
inline Foam::dgTree<Type>::treeIterator::treeIterator
(
    const dgTree<Type>* treePtr,
    const labelList& partIndex
)
:
    iteratorBase(treePtr, partIndex)
{}

template<class Type>
inline shared_ptr<Foam::dgTreeUnit<Type>> Foam::dgTree<Type>::treeIterator::operator()()
{
    return this->object();
}

template<class Type>
inline const shared_ptr<Foam::dgTreeUnit<Type>> Foam::dgTree<Type>::treeIterator::operator()() const
{
    return this->object();
}

template<class Type>
inline typename Foam::dgTree<Type>::treeIterator&
Foam::dgTree<Type>::treeIterator::operator++()
{
    this->increment();
    return *this;
}

template<class Type>
inline typename Foam::dgTree<Type>::treeIterator&
Foam::dgTree<Type>::treeIterator::operator++(int)
{
    treeIterator old = *this;
    this->increment();
    return old;
}

template<class Type>
inline typename Foam::dgTree<Type>::treeIterator
Foam::dgTree<Type>::begin() const
{
    return treeIterator(this);
}

template<class Type>
inline typename Foam::dgTree<Type>::treeIterator
Foam::dgTree<Type>::begin(const labelList& partIndex) const
{
    return treeIterator(this, partIndex);
}

// * * * * * * * * * * * * * * * * tree leaf iterator  * * * * * * * * * * * * * * //

template<class Type>
inline Foam::dgTree<Type>::leafIterator::leafIterator()
:
    iteratorBase()
{}


template<class Type>
inline Foam::dgTree<Type>::leafIterator::leafIterator
(
    const iteratorEnd&
)
:
    iteratorBase()
{}

template<class Type>
inline Foam::dgTree<Type>::leafIterator::leafIterator
(
    const dgTree<Type>* treePtr
)
:
    iteratorBase(treePtr)
{}

template<class Type>
inline Foam::dgTree<Type>::leafIterator::leafIterator
(
    const dgTree<Type>* treePtr,
    const labelList& partIndex
)
:
    iteratorBase(treePtr, partIndex)
{}

template<class Type>
inline shared_ptr<Foam::dgTreeUnit<Type>> Foam::dgTree<Type>::leafIterator::operator()()
{
    return this->object();
}

template<class Type>
inline const shared_ptr<Foam::dgTreeUnit<Type>> Foam::dgTree<Type>::leafIterator::operator()() const
{
    return this->object();
}

template<class Type>
inline typename Foam::dgTree<Type>::leafIterator&
Foam::dgTree<Type>::leafIterator::operator++()
{
    while(this->object() != NULL){
        this->increment();
        if(this->object() == NULL) break;
        if(this->object()->sub()[0]==NULL) break;
    }
    return *this;
}

template<class Type>
inline typename Foam::dgTree<Type>::leafIterator&
Foam::dgTree<Type>::leafIterator::operator++(int)
{
    leafIterator old = *this;
    while(this->object() != NULL){
        this->increment();
        if(this->object() == NULL) break;
        if(this->object()->sub()[0]==NULL) break;
    }
    return old;
}

template<class Type>
inline typename Foam::dgTree<Type>::leafIterator
Foam::dgTree<Type>::leafBegin() const
{
    return leafIterator(this);
}

template<class Type>
inline typename Foam::dgTree<Type>::leafIterator
Foam::dgTree<Type>::leafBegin(const labelList& partIndex) const
{
    return leafIterator(this, partIndex);
}