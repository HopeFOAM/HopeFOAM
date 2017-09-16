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
    Foam::dgGaussField

Description
    Field to store the data intepolated from GeometricDofField with gauss base

SourceFiles
    dgGaussField.C
    
Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifndef dgGaussField_H
#define dgGaussField_H

//#include "GeometricDofField.H"
#include "dgFieldsFwd.H"
#include "dgMesh.H"
//#include "dgSubList.H"
#include "dgGeoMesh.H"

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Forward declaration of friend functions and operators
class dgGeoMesh;

template<class Type>
class dgPatchField;

template<class Type>
class dgGaussField;

typedef dgGaussField<scalar>  dgGaussScalarField;
typedef dgGaussField<vector> dgGaussVectorField;
typedef dgGaussField<tensor> dgGaussTensorField;
typedef dgGaussField<sphericalTensor> dgGaussSphericalTensorField;
typedef dgGaussField<symmTensor> dgGaussSymmTensorField;

template<class Type>
Ostream& operator<<
(
    Ostream&,
    const dgGaussField<Type>&
);

template<class Type>
Ostream& operator<<
(
    Ostream&,
    const tmp<dgGaussField<Type>>&
);
/*---------------------------------------------------------------------------*\
                       Class dgGaussField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class dgGaussField
:
 	public refCount,
    public regIOobject
{
public:

    //- Runtime type information
    //TypeName("dgGaussField");
    typedef typename dgGeoMesh::Mesh Mesh;

    typedef typename Field<Type>::cmptType cmptType;


private:

    // Private data
    	//- Reference to mesh
        const Mesh& mesh_;
        //- Reference to GeometricDofField
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& originField_;
        //- Dimension set for this field
        dimensionSet dimensions_;

    	//- cell data
		Field<Type> cellField_;

		//- owner face data
		Field<Type> ownerFaceField_;

		//- neighbor face data
		Field<Type> neighborFaceField_;

	// Private member function

public:

    //- Runtime type information
    //TypeName("dgGaussField");

    // Constructors

    	//- Construct as copy
        dgGaussField
        (
            const dgGaussField<Type>&
        );

    	 dgGaussField
        (
        	const word& newName,
            const dgGaussField<Type>&
        );
	 
        //- Construct as copy of tmp<dgGaussField> deleting argument
        #ifndef NoConstructFromTmp
        dgGaussField
        (
            const tmp<dgGaussField<Type>>&
        );

	    dgGaussField
        (

		const word& newName,
            const tmp<dgGaussField<Type>>&
        );
        #endif


        //- Construct from GeometricDofField
        dgGaussField
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );

        //- Construct from tmp<GeometricDofField> deleting argument
        #ifndef NoConstructFromTmp
        dgGaussField
        (
            const tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>&
        );
        #endif

        //- Construct from GeometricField resetting name
        dgGaussField
        (
            const word& newName,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );

        //- Construct from tmp<GeometricDofField> resetting name
        #ifndef NoConstructFromTmp
        dgGaussField
        (
            const word& newName,
            const tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>&
        );
        #endif

        // member function

            //- init field data
            void initField(const GeometricDofField<Type, dgPatchField, dgGeoMesh>&);

        	//- Return mesh
        	inline const Mesh& mesh() const{return mesh_;}
            inline const GeometricDofField<Type, dgPatchField, dgGeoMesh>& originField()const{return originField_;}

            inline const Field<Type>& cellField() const{return cellField_;}
			inline  Field<Type>& cellFieldRef()  {return cellField_;}
			

            inline const Field<Type>& ownerFaceField() const{return ownerFaceField_;}
            inline  Field<Type>& ownerFaceFieldRef() {return ownerFaceField_;}
        	
            
            inline const Field<Type>& neighborFaceField() const{return neighborFaceField_;}
            inline  Field<Type>& neighborFaceFieldRef() {return neighborFaceField_;}
        	

	        //- Return dimensions
	        inline const dimensionSet& dimensions() const{return dimensions_;}

	        //- Return non-const access to dimensions
	        inline dimensionSet& dimensions(){return dimensions_;}

        //- WriteData member function required by regIOobject
        bool writeData(Ostream&) const;


// Member function *this operators



        tmp<dgGaussField<Type>>	T() const;

        tmp<dgGaussField < typename dgGaussField<Type>::cmptType>> component
		(    
            const direction d
		) const ;

        void negate();

        void replace
        (
            const direction,
            const dgGaussField<cmptType>&
        );

        void replace
        (
            const direction,
            const dimensioned<cmptType>&
        );

        void max(const dimensioned<Type>&);
        void min(const dimensioned<Type>&);

        void max
        (
            const dgGaussField<Type>&,
            const dimensioned<Type>&
        );

        void min
        (
            const dgGaussField<Type>&,
            const dimensioned<Type>&
        );

   



        // Ostream operators

        friend Ostream& operator<< <Type>
        (
            Ostream&,
            const dgGaussField<Type>&
        );

        friend Ostream& operator<< <Type>
        (
            Ostream&,
            const tmp<dgGaussField<Type>>&
        );

		 //- Return a const-reference to the dimensioned internal field
        //  Useful in the formulation of source-terms for FV equations
        //inline const Internal& operator()() const;

        void operator=(const dgGaussField<Type>&);

	    void operator=(const GeometricDofField<Type, dgPatchField, dgGeoMesh>&);
	    void operator=(const tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>&);
	 
        void operator=(const tmp<dgGaussField<Type>>&);
        void operator=(const dimensioned<Type>&);

        void operator==(const tmp<dgGaussField<Type>>&);
        void operator==(const dimensioned<Type>&);

        void operator+=(const  dgGaussField<Type>&);
        void operator+=(const tmp<dgGaussField<Type>>&);

        void operator-=(const  dgGaussField<Type>&);
        void operator-=(const tmp<dgGaussField<Type>>&);

        void operator*=(const  dgGaussField<scalar>&);
        void operator*=(const tmp<dgGaussField<scalar>>&);

        void operator/=(const  dgGaussField<scalar>&);
        void operator/=(const tmp<dgGaussField<scalar>>&);
		
        void operator+=(const dimensioned<Type>&);
        void operator-=(const dimensioned<Type>&);

        void operator*=(const dimensioned<scalar>&);
        void operator/=(const dimensioned<scalar>&);
};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
#include "dgGaussFieldFunctions.H"

#ifdef NoRepository
    #include "dgGaussField.C"
#endif


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
