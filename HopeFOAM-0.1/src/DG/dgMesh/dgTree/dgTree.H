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
    Foam::dgHpTree

Description
    A template class, the information is organized into a tree structure

SourceFiles
    dgTree.C

Author
    Feng Yongquan (yqfeng0418@163.com)

\*---------------------------------------------------------------------------*/

#ifndef dgTree_H
#define dgTree_H


#include "dgTreeUnit.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace Foam
{

struct dgTreeCore{

    dgTreeCore()
    {}

    ClassName("dgTree");

    struct iteratorEnd{
        //- Construct null
        iteratorEnd()
        {}
    };

    inline static iteratorEnd cend()
    {
        return iteratorEnd();
    }

    inline static iteratorEnd end()
    {
        return iteratorEnd();
    }
};


/*---------------------------------------------------------------------------*\
                          Class dgHpPolyMesh Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class dgTree
:
    public dgTreeCore
{

private:
    //The list saving the information of dgPolyMesh which is organized by cells
    //List<dgPolyCell> dgCells_;

    //The basis of forming the tree structure
    List<shared_ptr<dgTreeUnit<Type>>> baseLst_;

    //The point to the label of the list
    //label lstTp_;

    //label leafLstTp_;

    //The size of leaf nodes
    label leafSize_;

    //The size of all nodes
    label size_;

public:

    class treeIterator;
    class leafIterator;

    dgTree();
    //Construct from given size of list and list which is the information of this tree
    dgTree(const label, const List<shared_ptr<dgTreeUnit<Type>>>&);

    ~dgTree();

    void initial(const label, List<shared_ptr<dgTreeUnit<Type>>>&);


    //Refine the node of the cell
    //Given the node needing to refine
    void refine(shared_ptr<dgTreeUnit<Type>>);


    inline List<shared_ptr<dgTreeUnit<Type>>>& baseLst() {return baseLst_;}

    inline const List<shared_ptr<dgTreeUnit<Type>>>& baseLst() const {return baseLst_;}

    inline const label leafSize() const {return leafSize_;}

    inline const label size() const {return size_;}

    // The iterator base for dgTree
    class iteratorBase
    {
    private:
    //- Private data
        
        //- Pointer to the dgTree<Type> for which this is an iterator
        const dgTree<Type>* treePtr_;

        //- Current element
        shared_ptr<dgTreeUnit<Type>> entryPtr_;

        //- Current index
        label index_;

        //- The index needed iterator covered, if null, cover all
        labelList partIndex_;

    protected:
        //constructors
            
            //Construct null
            inline iteratorBase();

            //Construct from dgTree, moving to its 'begin' position
            inline explicit iteratorBase
            (
                const dgTree<Type>* curTree
            );

            inline explicit iteratorBase
            (
                const dgTree<Type>* curTree,
                const labelList& partIndex
            );

        // Protected Member functions
            
            //- Increment to the next position
            inline void increment();

            //- Return non-const access to referenced object
            inline shared_ptr<dgTreeUnit<Type>> object();

            //- Return const access to referenced object
            inline const shared_ptr<dgTreeUnit<Type>> cobject() const;

    public:

        //Member operators
            
            //Access


            //- Compare entryPtr 
            inline bool operator==(const iteratorEnd& unused) const;
            inline bool operator!=(const iteratorEnd& unused) const;

    };


    //- A iterator for all node in the tree
    class treeIterator
    :
        public iteratorBase
    {  
        friend class dgTree;

        // Private member functions
            //- Construct from dgTree, moving to its 'begin' position
            inline explicit treeIterator
            (
                const dgTree<Type>* curTree
            );

            inline explicit treeIterator
            (
                const dgTree<Type>* curTree,
                const labelList& partIndex
            );

    public:
        //Constructors
        
            //- Construct null()
            inline treeIterator();

            inline treeIterator(const iteratorEnd& unused);

        //Member operators
            inline  shared_ptr<dgTreeUnit<Type>> operator()();
            inline const shared_ptr<dgTreeUnit<Type>> operator()() const;
            inline treeIterator& operator++();
            inline treeIterator& operator++(int);

    };
    inline treeIterator begin() const;

    inline treeIterator begin(const labelList& partIndex) const;

    // A iterator for all leaf node in the tree
    class leafIterator
    :
        public iteratorBase
    {  
        friend class dgTree;

        // Private member functions
            //- Construct from dgTree, moving to its 'begin' leaf position
            inline explicit leafIterator
            (
                const dgTree<Type>* curTree
            );

            inline explicit leafIterator
            (
                const dgTree<Type>* curTree,
                const labelList& partIndex
            );

    public:
        //Constructors
        
            //- Construct null()
            inline leafIterator();

            inline leafIterator(const iteratorEnd& unused);

        //Member operators
            inline  shared_ptr<dgTreeUnit<Type>> operator()();
            inline const shared_ptr<dgTreeUnit<Type>> operator()() const;
            inline leafIterator& operator++();
            inline leafIterator& operator++(int);

    };

    inline leafIterator leafBegin() const;

    inline leafIterator leafBegin(const labelList& partIndex) const;

};


}
#ifdef NoRepository
    #include "dgTree.C"
#endif


#endif
