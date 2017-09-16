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

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/
#include "dgGaussField.H"
#include "FieldM.H"

using std::shared_ptr;
using std::make_shared;

class dgGeoMesh;

template<class Type>
class dgPatchField;
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define checkField(gf1, gf2, op)                                    \
if ((gf1).mesh() != (gf2).mesh())                                   \
{                                                                   \
    FatalErrorInFunction                                            \
        << "different mesh for fields "                             \
        << (gf1).name() << " and " << (gf2).name()                  \
        << " during operatrion " <<  op                             \
        << abort(FatalError);                                       \
}

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //
template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
    const dgGaussField<Type>& dgf
)
:
	refCount(),
	regIOobject(dgf),
	mesh_(dgf.mesh_),
    	originField_(dgf.originField_),
	dimensions_(dgf.dimensions_),
	cellField_(dgf.cellField_),
	ownerFaceField_(dgf.ownerFaceField_),
	neighborFaceField_(dgf.neighborFaceField_)
{}

	template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
	const word& newName,
    const dgGaussField<Type>& dgf
)
:
	refCount(),
	regIOobject(newName, dgf, newName == dgf.name()),
	mesh_(dgf.mesh_),
    	originField_(dgf.originField_),
	dimensions_(dgf.dimensions_),
	cellField_(dgf.cellField_),
	ownerFaceField_(dgf.ownerFaceField_),
	neighborFaceField_(dgf.neighborFaceField_)
{}

#ifndef NoConstructFromTmp
template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
    const tmp<dgGaussField<Type>>& tdgf
)
:
	regIOobject(tdgf(), tdgf.isTmp()),
	mesh_(tdgf().mesh_),
    	originField_(tdgf().originField_),
	dimensions_(tdgf().dimensions_),
	cellField_(tdgf().cellField_),
	ownerFaceField_(tdgf().ownerFaceField_),
	neighborFaceField_(tdgf().neighborFaceField_)
{}

	template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
	const word& newName,
    const tmp<dgGaussField<Type>>& tdgf
)
:
	regIOobject(newName, tdgf(), newName == tdgf().name()),
	mesh_(tdgf().mesh_),
    	originField_(tdgf().originField_),
	dimensions_(tdgf().dimensions_),
	cellField_(tdgf().cellField_),
	ownerFaceField_(tdgf().ownerFaceField_),
	neighborFaceField_(tdgf().neighborFaceField_)
{}
#endif

template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& gf
)
:
	regIOobject(gf),
	mesh_(gf.mesh()),
    originField_(gf),
	dimensions_(gf.dimensions())
{
	initField(gf);
}

#ifndef NoConstructFromTmp
template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
    const tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>& tgf
)
:
	regIOobject(tgf()),
	mesh_(tgf().mesh()),
    originField_(tgf()),
	dimensions_(tgf().dimensions())
{
	initField(tgf());
}
#endif

template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
    const word& newName,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& gf
)
:
	regIOobject(newName, gf, newName == gf.name()),
	mesh_(gf.mesh()),
    originField_(gf),
	dimensions_(gf.dimensions())
{
	initField(gf);
}

#ifndef NoConstructFromTmp
template<class Type>
Foam::dgGaussField<Type>::dgGaussField
(
    const word& newName,
    const tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>& tgf
)
:
	regIOobject(newName, tgf(), newName == tgf().name()),
	mesh_(tgf().mesh()),
    originField_(tgf()),
	dimensions_(tgf().dimensions())
{
	initField(tgf());
}
#endif

template<class Type>
bool Foam::dgGaussField<Type>::
writeData(Ostream& os) const
{
    return os.good();
}

template<class Type>
void Foam::dgGaussField<Type>::
initField(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& gf)
{
	// set cellField
	cellField_.setSize(mesh_.totalGaussCellDof());
    //interField : gf
    const dgTree<physicalCellElement>& cellPhysicalEleTree = mesh_.cellElementsTree();
    for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
        physicalCellElement& tmpCellEle = iter()->value();
        const denseMatrix<scalar>& interploateMatrix = tmpCellEle.baseFunction().gaussCellVandermonde();
        SubField<Type> subCellField(cellField_, tmpCellEle.baseFunction().gaussNDofPerCell(), tmpCellEle.gaussStart());
        SubField<Type> subGf(gf.internalField(), tmpCellEle.baseFunction().nDofPerCell(), tmpCellEle.dofStart());
        denseMatrix<Type>::MatVecMult(interploateMatrix, subGf, subCellField);
    }
	
	// set face field
	//const List<dgFace>& faceList = mesh_.dgFaceList();
    ownerFaceField_.setSize(mesh_.totalGaussFaceDof());
    neighborFaceField_.setSize(ownerFaceField_.size());
    List<Type> tempData(mesh_.maxDofPerFace());
    for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
        List<shared_ptr<physicalFaceElement>>& facePtr = iter()->value().faceElementList();
        forAll(facePtr, faceI){
            if(facePtr[faceI]->ownerEle_ == iter()){
                physicalCellElement& ownerCell = facePtr[faceI]->ownerEle_->value();
                label dofStart = ownerCell.dofStart();
                const labelList& ownerMapping = facePtr[faceI]->ownerDofMapping();
                label gaussStart = facePtr[faceI]->ownerFaceStart_;
                const denseMatrix<scalar>& faceMatrix = const_cast<stdElement&>(ownerCell.baseFunction()).gaussFaceInterpolateMatrix(facePtr[faceI]->gaussOrder_, 0);
                SubField<Type> subOwner(ownerFaceField_, facePtr[faceI]->nGaussPoints_, gaussStart);
                SubList<Type> subTemp(tempData, ownerMapping.size());
                for(int k=0; k<ownerMapping.size(); k++){
                    subTemp[k] = gf[ownerMapping[k] + dofStart];
                }
                denseMatrix<Type>::MatVecMult(faceMatrix, subTemp, subOwner);
            }
            else{
                physicalCellElement& neighborCell = facePtr[faceI]->neighborEle_->value();
                label dofStart = neighborCell.dofStart();
                const labelList& neighborMapping = facePtr[faceI]->neighborDofMapping();
                label gaussStart = facePtr[faceI]->neighbourFaceStart_;
                const denseMatrix<scalar>& faceMatrix = const_cast<stdElement&>(neighborCell.baseFunction()).gaussFaceInterpolateMatrix(facePtr[faceI]->gaussOrder_, 0);
                SubField<Type> subNeighbor(neighborFaceField_, facePtr[faceI]->nGaussPoints_, gaussStart);
                SubList<Type> subTemp(tempData, neighborMapping.size());
                for(int k=0; k<neighborMapping.size(); k++){
                    subTemp[k] = gf[neighborMapping[k] + dofStart];
                }
                denseMatrix<Type>::MatVecMult(faceMatrix, subTemp, subNeighbor);
            }
        }
    }

    const dgTree<physicalFaceElement>& facePhysicalEleTree = mesh_.faceElementsTree();

    typename Foam::GeometricField<Type, dgPatchField, dgGeoMesh>::Boundary& bData = const_cast<Foam::GeometricDofField<Type, dgPatchField, dgGeoMesh>&>(gf).boundaryFieldRef();

    forAll(bData, patchI){
        //- empty patch
        if(bData[patchI].size()==0) continue;


        const labelList& dgFaceIndex = mesh_.boundary()[patchI].dgFaceIndex();

        label start = 0;
        for(dgTree<physicalFaceElement>::leafIterator iter = facePhysicalEleTree.leafBegin(dgFaceIndex); iter != facePhysicalEleTree.end(); ++iter){
            label localID = iter()->value().faceOwnerLocalID_;
            shared_ptr<physicalFaceElement> faceElePtr = iter()->value().ownerEle_->value().faceElementList()[localID];
            label dofNum = faceElePtr->ownerEle_->value().baseFunction().nDofPerFace();
            const denseMatrix<scalar>& faceInterpolateMatrix = const_cast<stdElement&>(faceElePtr->ownerEle_->value().baseFunction()).gaussFaceInterpolateMatrix(faceElePtr->gaussOrder_, 0);
            label gaussStart = faceElePtr->neighbourFaceStart_;

            SubField<Type> subNeighbor(neighborFaceField_, faceElePtr->nGaussPoints_, gaussStart);
            SubList<Type> subData(bData[patchI], dofNum, start);
            denseMatrix<Type>::MatVecMult(faceInterpolateMatrix, subData, subNeighbor);

            start += dofNum;
        }

    }

}

//****************dgGaussField operator******************************//
template<class Type>
Foam::tmp<Foam::dgGaussField<Type>>
Foam::dgGaussField<Type>::T() const
{
    tmp<dgGaussField<Type>> result
    (
      new  dgGaussField<Type>
    (
         GeometricDofField<Type, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                this->name() + ".T()",
                this->instance(),
                this->db()
            ),
            this->mesh(),
            this->dimensions()
        )
        )
    );

    Foam::T(result.ref().cellFieldRef(),cellField());
    Foam::T(result.ref().ownerFaceFieldRef(), ownerFaceField());
    Foam::T(result.ref().neighborFaceFieldRef(), neighborFaceField());

    return result;
}


template<class Type>
Foam::tmp
<
    Foam::dgGaussField
    <
        typename Foam::dgGaussField<Type>::cmptType
    >
>
Foam::dgGaussField<Type>::component
(
    const direction d
) const
{
    tmp<dgGaussField<cmptType>> Component
    (
    new dgGaussField<cmptType>
    (
         GeometricDofField<cmptType, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                this->name() + ".component(" + Foam::name(d) + ')',
                this->instance(),
                this->db()
            ),
            this->mesh(),
            this->dimensions()
        )
        )
    );

    Foam::component(Component.ref().cellFieldRef(),cellField(), d);
    Foam::component(Component.ref().ownerFaceFieldRef(), ownerFaceField(), d);
    Foam::component(Component.ref().neighborFaceFieldRef(), neighborFaceField(), d);

    return Component;
}


template<class Type>
void Foam::dgGaussField<Type>::replace
(
    const direction d,
    const dgGaussField
    <
        typename dgGaussField<Type>::cmptType
      
     >& gcf
)
{
    cellFieldRef().replace(d, gcf.cellField());
    ownerFaceFieldRef().replace(d, gcf.ownerFaceField());
    neighborFaceFieldRef().replace(d, gcf.neighborFaceField());
}


template<class Type>
void Foam::dgGaussField<Type>::replace
(
    const direction d,
    const dimensioned<cmptType>& ds
)
{
    cellFieldRef().replace(d, ds.value());
    ownerFaceFieldRef().replace(d, ds.value());
    neighborFaceFieldRef().replace(d, ds.value());
 
}


template<class Type>
void Foam::dgGaussField<Type>::max
(
    const dimensioned<Type>& dt
)
{
    Foam::max(cellFieldRef(), cellField(), dt.value());
    Foam::max(ownerFaceFieldRef(), ownerFaceField(), dt.value());
    Foam::max(neighborFaceFieldRef(), neighborFaceField(), dt.value());	
}


template<class Type>
void Foam::dgGaussField<Type>::min
(
    const dimensioned<Type>& dt
)
{
    Foam::min(cellFieldRef(), cellField(), dt.value());
    Foam::min(ownerFaceFieldRef(), ownerFaceField(), dt.value());
    Foam::min(neighborFaceFieldRef(), neighborFaceField(), dt.value());	
}


template<class Type>
void Foam::dgGaussField<Type>::negate()
{
    cellFieldRef().negate();
    ownerFaceFieldRef().negate();
    neighborFaceFieldRef().negate();
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

template<class Type>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const dgGaussField<Type>& gf
)
{
	os <<nl<<"gauss"<< gf.name()<<nl<<nl;
	os.writeKeyword("dimensions") << gf.dimensions() << token::END_STATEMENT
        << nl<<nl;

    os << "/ * * * * * * * * cell data * * * * * * * * * /"<<nl<<nl;
    os << gf.cellField();

    os<<"/ * * * * * * * * owner face data * * * * * * * * * /"<<nl<<nl;
    os << gf.ownerFaceField();

    os<<"/ * * * * * * * * neighbor face data * * * * * * * * * /"<<nl<<nl;
    os << gf.neighborFaceField();

    return (os);
}


template<class Type>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const tmp<dgGaussField<Type>>& tgf
)
{
    os << tgf();
    tgf.clear();
    return os;
}


//operator
template<class Type>
void Foam::dgGaussField<Type>::operator=
(
    const dgGaussField<Type>& gf
)
{
    if (this == &gf)
    {
        FatalErrorInFunction
            << "attempted assignment to self"
            << abort(FatalError);
    }

    checkField(*this, gf, "=");

    // Only assign field contents not ID

  	cellFieldRef()= gf.cellField();
	ownerFaceFieldRef() = gf.ownerFaceField();
	neighborFaceFieldRef() = gf.neighborFaceField();
   
}


template<class Type>
void Foam::dgGaussField<Type>::operator=
(
    const tmp< dgGaussField<Type> >& tgf
)
{
    if (this == &(tgf()))
    {
        FatalErrorInFunction
            << "attempted assignment to self"
            << abort(FatalError);
    }

    const dgGaussField<Type>& gf = tgf();

    checkField(*this, gf, "=");

    // Only assign field contents not ID

    this->dimensions() = gf.dimensions();

    // Transfer the storage from the tmp
/*    cellFieldRef().transfer
    (
        const_cast<dgGaussField<Type>&>(gf).cellFieldRef()
    );
*/ 
    cellFieldRef().transfer
    (
        const_cast<dgGaussField<Type>&>(gf).cellFieldRef()
    );

/*    ownerFaceFieldRef().transfer
    (
        const_cast<dgGaussField<Type>&>(gf).ownerFaceFieldRef()
    );
*/
    ownerFaceFieldRef().transfer
    (
        const_cast<dgGaussField<Type>&>(gf).ownerFaceFieldRef()
    );
/*	  neighborFaceFieldRef().transfer
    (
        const_cast<dgGaussField<Type>&>(gf).neighborFaceFieldRef()
    );
*/  
    neighborFaceFieldRef().transfer
    (
        const_cast<dgGaussField<Type>&>(gf).neighborFaceFieldRef()
    );

    tgf.clear();
}


template<class Type>
void Foam::dgGaussField<Type>::operator=
(
    const dimensioned<Type>& dt
)
{
   	cellFieldRef() = dt.value();
    //boundaryFieldRef() = dt.value();
	ownerFaceFieldRef() =  dt.value();
	neighborFaceFieldRef() =  dt.value();
}


	template<class Type>
void Foam::dgGaussField<Type>::operator=
(
	const GeometricDofField<Type, dgPatchField, dgGeoMesh>& gf
){
	checkField(gf,*this, "=");

	initField(gf);

}

		template<class Type>
void Foam::dgGaussField<Type>::operator=
(
	const tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>& tgf
){

	checkField(tgf(),*this, "=");

	initField(tgf());

	tgf.clear();

}
	

	 

template<class Type>
void Foam::dgGaussField<Type>::operator==
(
    const tmp<dgGaussField<Type>>& tgf
)
{
    const dgGaussField<Type>& gf = tgf();

    checkField(*this, gf, "==");

    // Only assign field contents not ID

	// ref() = gf();
	// boundaryFieldRef() == gf.boundaryField();
   	cellFieldRef() = gf.cellField();
  
	ownerFaceFieldRef() =  gf.ownerFaceField();
	neighborFaceFieldRef() =  gf.neighborFaceField();

    tgf.clear();
}


template<class Type>
void Foam::dgGaussField<Type>::operator==
(
    const dimensioned<Type>& dt
)
{
	cellFieldRef() = dt.value();
    //boundaryFieldRef() = dt.value();
	ownerFaceFieldRef() == dt.value();
	neighborFaceFieldRef()==  dt.value();
    //boundaryFieldRef() == dt.value();
}


#define COMPUTED_ASSIGNMENT(TYPE, op)                                          \
                                                                               \
template<class Type>          \
void Foam::dgGaussField<Type>::operator op              \
(                                                                              \
    const dgGaussField<TYPE>& gf                        \
)                                                                              \
{                                                                              \
    checkField(*this, gf, #op);                                                \
                                                                               \
	cellFieldRef() op  gf.cellField() ;					\
	ownerFaceFieldRef() op  gf.ownerFaceField() ;		\
	neighborFaceFieldRef()  op  gf.neighborFaceField()  ;		\
}                                                                              \
                                                                               \
template<class Type>          \
void Foam::dgGaussField<Type>::operator op              \
(                                                                              \
    const tmp<dgGaussField<TYPE>>& tgf                  \
)                                                                              \
{                                                                              \
    operator op(tgf());                                                        \
    tgf.clear();                                                               \
}                                                                              \
                                                                               \
template<class Type>          \
void Foam::dgGaussField<Type>::operator op              \
(                                                                              \
    const dimensioned<TYPE>& dt                                                \
)                                                                              \
{                                                                              \
   	cellFieldRef() op  dt.value();				\
	ownerFaceFieldRef() op dt.value();	\
	neighborFaceFieldRef() op  dt.value();	\
}

COMPUTED_ASSIGNMENT(Type, +=)
COMPUTED_ASSIGNMENT(Type, -=)
COMPUTED_ASSIGNMENT(scalar, *=)
COMPUTED_ASSIGNMENT(scalar, /=)

#undef COMPUTED_ASSIGNMENT

#undef checkField

#include "dgGaussFieldFunctions.C"
