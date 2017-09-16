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
#include "GeometricDofField.H"


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


//***********************private functions**********************//

template<class Type, template<class> class PatchField, class GeoMesh>
bool Foam::GeometricDofField<Type, PatchField, GeoMesh>::readIfPresent()
{
    if
    (
        this->readOpt() == IOobject::MUST_READ
     || this->readOpt() == IOobject::MUST_READ_IF_MODIFIED
    )
    {
        WarningInFunction
            << "read option IOobject::MUST_READ or MUST_READ_IF_MODIFIED"
            << " suggests that a read constructor for field " << this->name()
            << " would be more appropriate." << endl;
    }
    else if (this->readOpt() == IOobject::READ_IF_PRESENT )//&& this->headerOk())
    {
       // readFields();

        // Check compatibility between field and mesh
        if (this->size() != GeoMesh::size(this->mesh()))
        {
            FatalIOErrorInFunction(this->readStream(typeName))
                << "   number of field elements = " << this->size()
                << " number of mesh elements = "
                << GeoMesh::size(this->mesh())
                << exit(FatalIOError);
        }

        readOldTimeIfPresent();
		

        return true;
    }

  //div_count1 ++;
    return false;
}



template<class Type, template<class> class PatchField, class GeoMesh>
bool Foam::GeometricDofField<Type, PatchField, GeoMesh>::readOldTimeIfPresent()
{
    //div_count2 ++;
    // Read the old time field if present
    IOobject field0
    (
        this->name()  + "_0",
        this->time().timeName(),
        this->db(),
        IOobject::READ_IF_PRESENT,
        IOobject::AUTO_WRITE,
        this->registerObject()
    );

    if (field0.headerOk())
    {
        if (debug)
        {
            InfoInFunction << "Reading old time level for field"
                           << endl << this->info() << endl;
        }

        field0Ptr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            field0,
            this->mesh()
        );

        field0Ptr_->timeIndex_ = timeIndex_ - 1;

        if (!field0Ptr_->readOldTimeIfPresent())
        {
            field0Ptr_->oldTime();
        }

        return true;
    }

    return false;
}
// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //
template<class Type, template<class> class PatchField, class GeoMesh>
inline const Foam::GeometricDofField<Type, PatchField, GeoMesh>&
Foam::GeometricDofField<Type, PatchField, GeoMesh>::null()
{
    return NullObjectRef<GeometricDofField<Type, PatchField, GeoMesh>>();
}

template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Mesh& mesh,
    const dimensionSet& ds,
    const word& patchFieldType
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, mesh, ds, patchFieldType)
{
   // readOldTimeIfPresent();
   readIfPresent();
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Mesh& mesh,
    const dimensionSet& ds,
    const wordList& patchFieldTypes,
    const wordList& actualPatchTypes
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, mesh, ds, patchFieldTypes, actualPatchTypes)
{
   // readOldTimeIfPresent();
   readIfPresent();
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Mesh& mesh,
    const dimensioned<Type>& dt,
    const word& patchFieldType
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, mesh, dt, patchFieldType)
{
   // readOldTimeIfPresent();

   readIfPresent();
	
//time_div2 += (t_end.tv_sec-t_start.tv_sec)+(t_end.tv_usec-t_start.tv_usec)*1e-06;
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Mesh& mesh,
    const dimensioned<Type>& dt,
    const wordList& patchFieldTypes,
    const wordList& actualPatchTypes
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, mesh, dt, patchFieldTypes, actualPatchTypes)
{
   // readOldTimeIfPresent();
   readIfPresent();
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Internal& diField,
    const PtrList<PatchField<Type>>& ptfl
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, diField, ptfl)
{
  //  readOldTimeIfPresent();
  readIfPresent();
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Mesh& mesh,
    const dimensionSet& ds,
    const Field<Type>& iField,
    const PtrList<PatchField<Type>>& ptfl
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, mesh, ds, iField, ptfl)
{
    //readOldTimeIfPresent();
    readIfPresent();
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Mesh& mesh
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, mesh)
{

   readOldTimeIfPresent();
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const Mesh& mesh,
    const dictionary& dict
)
    :
    timeIndex_(this->time().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, mesh, dict)
{
    readOldTimeIfPresent();
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
    :
    timeIndex_(gf.timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(gf)
{
    if (gf.field0Ptr_)
    {
        field0Ptr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            *gf.field0Ptr_
        );
    }
    this->writeOpt() = IOobject::NO_WRITE;
}


#ifndef NoConstructFromTmp
template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
    :
    timeIndex_(tgf().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(tgf())
{
    this->writeOpt() = IOobject::NO_WRITE;
}
#endif


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
    :
    timeIndex_(gf.timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, gf)
{
    if (!readIfPresent() && gf.field0Ptr_)
    {
        field0Ptr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            io.name() + "_0",
            *gf.field0Ptr_
        );
    }
}


#ifndef NoConstructFromTmp
template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
    :
    timeIndex_(tgf().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, tgf())
{
 //   readOldTimeIfPresent();
    tgf.clear();
    readIfPresent();
}
#endif


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const word& newName,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
    :
    timeIndex_(gf.timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(newName, gf)
{
    if (!readIfPresent() && gf.field0Ptr_)
    {
        field0Ptr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            newName + "_0",
            *gf.field0Ptr_
        );
    }
}


#ifndef NoConstructFromTmp
template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const word& newName,
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
    :
    timeIndex_(tgf().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(newName, tgf())
{
}
#endif


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf,
    const word& patchFieldType
)
    :
    timeIndex_(gf.timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, gf, patchFieldType)
{
  if (!readIfPresent() && gf.field0Ptr_)
    {
        field0Ptr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            io.name() + "_0",
            *gf.field0Ptr_
        );
    }
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf,
    const wordList& patchFieldTypes,
    const wordList& actualPatchTypes

)
    :
    timeIndex_(gf.timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, gf, patchFieldTypes, actualPatchTypes)
{


    if (!readIfPresent() && gf.field0Ptr_)
    {
        field0Ptr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            io.name() + "_0",
            *gf.field0Ptr_
        );
    }
}


#ifndef NoConstructFromTmp
template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::GeometricDofField
(
    const IOobject& io,
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf,
    const wordList& patchFieldTypes,
    const wordList& actualPatchTypes
)
    :
    timeIndex_(tgf().timeIndex()),
    field0Ptr_(NULL),
    fieldPrevIterPtr_(NULL),
    GeometricField<Type, PatchField, GeoMesh>(io, tgf, patchFieldTypes, actualPatchTypes)
{
}
#endif

// * * * * * * * * * * * * * * * Destructor * * * * * * * * * * * * * * * * * //

template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>::~GeometricDofField()
{
    deleteDemandDrivenData(field0Ptr_);
    deleteDemandDrivenData(fieldPrevIterPtr_);

    if(gaussField_){
        delete gaussField_;
    }
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::storeOldTimes() const
{
    if
    (
        field0Ptr_
        && timeIndex_ != this->time().timeIndex()
        && !(
            this->name().size() > 2
            && this->name()(this->name().size()-2, 2) == "_0"
        )
    )
    {
        storeOldTime();
    }

    // Correct time index
    timeIndex_ = this->time().timeIndex();
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::storeOldTime() const
{
    if (field0Ptr_)
    {
        field0Ptr_->storeOldTime();

        if (debug)
        {
            InfoInFunction
                    << "Storing old time field for field" << endl
                    << this->info() << endl;
        }

        *field0Ptr_ == *this;
        field0Ptr_->timeIndex_ = timeIndex_;

        if (field0Ptr_->field0Ptr_)
        {
            field0Ptr_->writeOpt() = this->writeOpt();
        }
    }
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::label Foam::GeometricDofField<Type, PatchField, GeoMesh>::nOldTimes() const
{
    if (field0Ptr_)
    {
        return field0Ptr_->nOldTimes() + 1;
    }
    else
    {
        return 0;
    }
}


template<class Type, template<class> class PatchField, class GeoMesh>
const Foam::GeometricDofField<Type, PatchField, GeoMesh>&
Foam::GeometricDofField<Type, PatchField, GeoMesh>::oldTime() const
{
    if (!field0Ptr_)
    {
     
        field0Ptr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            IOobject
            (
                this->name() + "_0",
                this->time().timeName(),
                this->db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE,
                this->registerObject()
            ),
            *this
        );
    }
    else
    {
     
        storeOldTimes();
    }

    return *field0Ptr_;
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::GeometricDofField<Type, PatchField, GeoMesh>&
Foam::GeometricDofField<Type, PatchField, GeoMesh>::oldTime()
{
    static_cast<const GeometricDofField<Type, PatchField, GeoMesh>&>(*this)
    .oldTime();

    return *field0Ptr_;
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::storePrevIter() const
{
    if (!fieldPrevIterPtr_)
    {
        if (debug)
        {
            InfoInFunction
                    << "Allocating previous iteration field" << endl
                    << this->info() << endl;
        }

        fieldPrevIterPtr_ = new GeometricDofField<Type, PatchField, GeoMesh>
        (
            this->name() + "PrevIter",
            *this
        );
    }
    else
    {
        *fieldPrevIterPtr_ == *this;
    }
}


template<class Type, template<class> class PatchField, class GeoMesh>
const Foam::GeometricDofField<Type, PatchField, GeoMesh>&
Foam::GeometricDofField<Type, PatchField, GeoMesh>::prevIter() const
{
    if (!fieldPrevIterPtr_)
    {
        FatalErrorInFunction
                << "previous iteration field" << endl << this->info() << endl
                << "  not stored."
                << "  Use field.storePrevIter() at start of iteration."
                << abort(FatalError);
    }

    return *fieldPrevIterPtr_;
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::correctBoundaryConditions()
{
    this->setUpToDate();
    GeometricField<Type, PatchField, GeoMesh>::storeOldTimes();
    this->boundaryFieldRef().evaluate();
}

template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::updateGaussField()
{
    if(gaussField_){
        gaussField_->initField(*this);
    }
    else{
        gaussField_ = new dgGaussField<Type>(*this);
    }
}

template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::relax(const scalar alpha)
{
    if (debug)
    {
        InfoInFunction
           << "Relaxing" << endl << this->info() << " by " << alpha << endl;
    }

    operator==(prevIter() + alpha*(*this - prevIter()));
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::relax()
{
    word name = this->name();

    if
    (
        this->mesh().data::template lookupOrDefault<bool>
        (
            "finalIteration",
            false
        )
    )
    {
        name += "Final";
    }

    if (this->mesh().relaxField(name))
    {
        relax(this->mesh().fieldRelaxationFactor(name));
    }
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::tmp<Foam::GeometricDofField<
typename Foam::GeometricDofField<Type, PatchField, GeoMesh>::cmptType, PatchField,
         GeoMesh>
         >   Foam::GeometricDofField<Type, PatchField, GeoMesh>::component
         (
             const direction d
         )
{

    typedef typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType  cmptType;

    tmp<GeometricDofField<cmptType, PatchField, GeoMesh>> tcomp
            (
                new GeometricDofField<cmptType, PatchField, GeoMesh>
                (
                    IOobject
                    (
                        this->name() + ".component(" + Foam::name(d) + ')',
                        this->instance(),
                        this->db(),
                        IOobject::NO_READ,
                        IOobject::NO_WRITE
                    ),
                    this->mesh(),
                    this->dimensions()
                )
            );

    Foam::component(tcomp.ref(),*this,d);

    return tcomp;
}

template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator=
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
{
    Foam::GeometricField<Type, PatchField, GeoMesh>::operator=(gf);
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator=
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
{
    const GeometricDofField<Type, PatchField, GeoMesh>& gf = tgf();
    tmp<GeometricField<Type, PatchField, GeoMesh>> tmpgf(gf);

    Foam::GeometricField<Type, PatchField, GeoMesh>::operator=(tmpgf);
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator=
(
    const dimensioned<Type>& dt
)
{
    Foam::GeometricField<Type, PatchField, GeoMesh>::operator=(dt);
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator==
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
{
    Foam::GeometricField<Type, PatchField, GeoMesh>::operator=(tgf());
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator==
(
    const dimensioned<Type>& dt
)
{
    Foam::GeometricField<Type, PatchField, GeoMesh>::operator==(dt);
}


#define COMPUTED_ASSIGNMENT(TYPE, op)                                          \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator op              \
(                                                                              \
    const GeometricDofField<TYPE, PatchField, GeoMesh>& gf                        \
)                                                                              \
{                                                                              \
   Foam::GeometricField<Type, PatchField, GeoMesh>::operator op(gf) ;  \
}                                                                              \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator op              \
(                                                                              \
    const tmp<GeometricDofField<TYPE, PatchField, GeoMesh>>& tgf                  \
)                                                                              \
{                                                                              \
    operator op(tgf());                                                        \
    tgf.clear();                                                               \
}                                                                              \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
void Foam::GeometricDofField<Type, PatchField, GeoMesh>::operator op              \
(                                                                              \
    const dimensioned<TYPE>& dt                                                \
)                                                                              \
{                                                                              \
    Foam::GeometricField<Type, PatchField, GeoMesh>::operator op(dt) ;  \
}

COMPUTED_ASSIGNMENT(Type, +=)
COMPUTED_ASSIGNMENT(Type, -=)
COMPUTED_ASSIGNMENT(scalar, *=)
COMPUTED_ASSIGNMENT(scalar, /=)

#undef COMPUTED_ASSIGNMENT
// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
{
    // os << nl<<gf.name()<<nl<<nl;
    // os.writeKeyword("dimensions") << gf.dimensions() << token::END_STATEMENT
    //                               << nl<<nl;

    Info << static_cast<GeometricField<Type, PatchField, GeoMesh>>(gf);

    return (os);
}


template<class Type, template<class> class PatchField, class GeoMesh>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
{
    os << tgf();
    tgf.clear();
    return os;
}

#undef checkField

#include "GeometricDofFieldFunctions.C"

