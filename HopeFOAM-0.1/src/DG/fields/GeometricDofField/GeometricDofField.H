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
    Foam::GeometricDofField

Description
    Wrapper of GeometricField with Dof Addresing supported

SourceFiles
    GeometricDofField.C
    
Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef GeometricDofField_H
#define GeometricDofField_H

#include "GeometricField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Forward declaration of friend functions and operators


template<class Type, template<class> class PatchField, class GeoMesh>
class GeometricDofField;

template<class Type>
class dgGaussField;

template<class Type, template<class> class PatchField, class GeoMesh>
Ostream& operator<<
(
    Ostream&,
    const GeometricDofField<Type, PatchField, GeoMesh>&
);

template<class Type, template<class> class PatchField, class GeoMesh>
Ostream& operator<<
(
    Ostream&,
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&
);

/*---------------------------------------------------------------------------*\
                       Class GeometricDofField Declaration
\*---------------------------------------------------------------------------*/

template<class Type, template<class> class PatchField, class GeoMesh>
class GeometricDofField
:
    public GeometricField<Type, PatchField, GeoMesh>
{

  //- Read from file if it is present
        bool readIfPresent();

        //- Read old time field from file if it is present
        bool readOldTimeIfPresent();
public:

    // Public typedefs

         //- Type of mesh on which this GeometricDofField is instantiated
        typedef typename GeoMesh::Mesh Mesh;

        //- Type of the internal field from which this GeometricDofField is derived
        typedef DimensionedField<Type, GeoMesh> Internal;

        //- Type of the patch field of which the
        //  GeometricField::Boundary is composed
        typedef PatchField<Type> Patch;

        void initAddressingData();

private:
    
    //Private data

    //- Current time index.
        //  Used to trigger the storing of the old-time value
        mutable label timeIndex_;

        //- Pointer to dgGaussField;
        dgGaussField<Type>* gaussField_ = NULL;

        //- Pointer to old time field
        mutable GeometricDofField<Type, PatchField, GeoMesh>* field0Ptr_ = NULL;

        //-  Pointer to previous iteration (used for under-relaxation)
        mutable GeometricDofField<Type, PatchField, GeoMesh>* fieldPrevIterPtr_ = NULL;


public:

    //- Runtime type information
    TypeName("GeometricDofField");


    // Public typedefs

        typedef typename Field<Type>::cmptType cmptType;

        //- Return a null geometric field
        inline static const GeometricDofField<Type, PatchField, GeoMesh>& null();

    // Constructors

        //- Constructor given IOobject, mesh, dimensions and patch type.
        //  This allocates storage for the field but not values.
        //  Used only within this class to create TEMPORARY variables
        GeometricDofField
        (
            const IOobject&,
            const Mesh&,
            const dimensionSet&,
            const word& patchFieldType=PatchField<Type>::calculatedType()
        );

        //- Constructor given IOobject, mesh, dimensions and patch types.
        //  This allocates storage for the field but not values.
        //  Used only within this class to create TEMPORARY variables
        GeometricDofField
        (
            const IOobject&,
            const Mesh&,
            const dimensionSet&,
            const wordList& wantedPatchTypes,
            const wordList& actualPatchTypes = wordList()
        );

        //- Constructor given IOobject, mesh, dimensioned<Type> and patch type.
        GeometricDofField
        (
            const IOobject&,
            const Mesh&,
            const dimensioned<Type>&,
            const word& patchFieldType=PatchField<Type>::calculatedType()
        );

        //- Constructor given IOobject, mesh, dimensioned<Type> and patch types.
        GeometricDofField
        (
            const IOobject&,
            const Mesh&,
            const dimensioned<Type>&,
            const wordList& wantedPatchTypes,
            const wordList& actualPatchTypes = wordList()
        );

        //- Constructor from components
        GeometricDofField
        (
            const IOobject&,
            const Internal&,
            const PtrList<PatchField<Type>>&
        );

        //- Constructor from components
        GeometricDofField
        (
            const IOobject&,
            const Mesh&,
            const dimensionSet&,
            const Field<Type>&,
            const PtrList<PatchField<Type>>&
        );

        //- Construct and read given IOobject
        GeometricDofField
        (
            const IOobject&,
            const Mesh&
        );

        //- Construct from dictionary
        GeometricDofField
        (
            const IOobject&,
            const Mesh&,
            const dictionary&
        );

        //- Construct as copy
        GeometricDofField
        (
            const GeometricDofField<Type, PatchField, GeoMesh>&
        );

        //- Construct as copy of tmp<GeometricDofField> deleting argument
        #ifndef NoConstructFromTmp
        GeometricDofField
        (
            const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&
        );
        #endif

        //- Construct as copy resetting IO parameters
        GeometricDofField
        (
            const IOobject&,
            const GeometricDofField<Type, PatchField, GeoMesh>&
        );

        //- Construct as copy of tmp<GeometricDofField> resetting IO parameters
        #ifndef NoConstructFromTmp
        GeometricDofField
        (
            const IOobject&,
            const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&
        );
        #endif

        //- Construct as copy resetting name
        GeometricDofField
        (
            const word& newName,
            const GeometricDofField<Type, PatchField, GeoMesh>&
        );

        //- Construct as copy resetting name
        #ifndef NoConstructFromTmp
        GeometricDofField
        (
            const word& newName,
            const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&
        );
        #endif

        //- Construct as copy resetting IO parameters and patch type
        GeometricDofField
        (
            const IOobject&,
            const GeometricDofField<Type, PatchField, GeoMesh>&,
            const word& patchFieldType
        );

        //- Construct as copy resetting IO parameters and boundary types
        GeometricDofField
        (
            const IOobject&,
            const GeometricDofField<Type, PatchField, GeoMesh>&,
            const wordList& patchFieldTypes,
            const wordList& actualPatchTypes = wordList()
        );

        //- Construct as copy resetting IO parameters and boundary types
        #ifndef NoConstructFromTmp
        GeometricDofField
        (
            const IOobject&,
            const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&,
            const wordList& patchFieldTypes,
            const wordList& actualPatchTypes = wordList()
        );
        #endif

    //- Destructor
    virtual ~GeometricDofField();

    //Member Functions

		 //- Return the time index of the field
        inline label timeIndex() const{return timeIndex_;}

        //- Return the time index of the field
        inline label& timeIndex(){ return timeIndex_;}

        //- Store the old-time fields
        void storeOldTimes() const;

        //- Store the old-time field
        void storeOldTime() const;

        //- Return the number of old time fields stored
        label nOldTimes() const;

        //- Return old time field
        const GeometricDofField<Type, PatchField, GeoMesh>& oldTime() const;

        //- Return non-const old time field
        //  (Not a good idea but it is used for sub-cycling)
        GeometricDofField<Type, PatchField, GeoMesh>& oldTime();

        //- Store the field as the previous iteration value
        void storePrevIter() const;

        //- Return previous iteration field
        const GeometricDofField<Type, PatchField, GeoMesh>& prevIter() const;

         //- Relax field (for steady-state solution).
        //  alpha = 1 : no relaxation
        //  alpha < 1 : relaxation
        //  alpha = 0 : do nothing
        void relax(const scalar alpha);

        //- Relax field (for steady-state solution).
        //  alpha is read from controlDict
        void relax();
		
        void correctBoundaryConditions();

        inline dgGaussField<Type>& gaussFieldRef(){
            return *gaussField_;
        }
        inline const dgGaussField<Type>& gaussField()const{
            return *gaussField_;
        }

        void updateGaussField();

	
    tmp<GeometricDofField
        <
            typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType,
            PatchField,
            GeoMesh
        > > component
    (
        const direction d
    );

    // Ostream operators

	inline const Internal& operator()() const;      

	void operator=(const GeometricDofField<Type, PatchField, GeoMesh>&);  

	void operator=(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&);    

	void operator=(const dimensioned<Type>&);      

	void operator==(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&);   

	void operator==(const dimensioned<Type>&);       

	void operator+=(const GeometricDofField<Type, PatchField, GeoMesh>&); 

	void operator+=(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&);    

	void operator-=(const GeometricDofField<Type, PatchField, GeoMesh>&);      

	void operator-=(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&);     

	void operator*=(const GeometricDofField<scalar, PatchField, GeoMesh>&);     

	void operator*=(const tmp<GeometricDofField<scalar,PatchField,GeoMesh>>&);   

	void operator/=(const GeometricDofField<scalar, PatchField, GeoMesh>&);     

	void operator/=(const tmp<GeometricDofField<scalar,PatchField,GeoMesh>>&);  

	void operator+=(const dimensioned<Type>&);      

	void operator-=(const dimensioned<Type>&);      

	void operator*=(const dimensioned<scalar>&);   

	void operator/=(const dimensioned<scalar>&);

	
        friend Ostream& operator<< <Type, PatchField, GeoMesh>
        (
            Ostream&,
            const GeometricDofField<Type, PatchField, GeoMesh>&
        );

        friend Ostream& operator<< <Type, PatchField, GeoMesh>
        (
            Ostream&,
            const tmp<GeometricDofField<Type, PatchField, GeoMesh>>&
        );

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//#include "GeometricFieldI.H"

#ifdef NoRepository
    #include "GeometricDofField.C"
#endif

#include "GeometricDofFieldFunctions.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //