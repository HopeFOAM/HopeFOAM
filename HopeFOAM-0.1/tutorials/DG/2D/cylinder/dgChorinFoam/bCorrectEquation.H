#ifndef bCorrectEquation_H
#define bCorrectEquation_H

#include "Equation.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                             Class EquationAdd Declaration
\*---------------------------------------------------------------------------*/
template<class Type>
class bCorrectEquation 
	: public dg::Equation<Type>
{

public:
	bCorrectEquation(shared_ptr<dg::Equation<Type>> a, scalar factor)
	{
		arg1 = a;
		factor_ = factor;

		if (this->lduMatrixPtr == NULL)
		{
			this->lduMatrixPtr = arg1->getLduMatrixPtr();
		}
	}

	virtual void calculateCell
		(dgTree<physicalCellElement>::leafIterator iter)
    {
		arg1->calculateCell(iter);

		this->lduMatrixPtr->b_ *= factor_;
    }

	virtual label getMatrixLabel()
    {
        return arg1->getMatrixLabel();
    };

	virtual dimensionSet getDimensionSet()
    {
		return arg1->getDimensionSet();
    };

	virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI()
    {
    	if(arg1->getPSI() != NULL) 
			return arg1->getPSI();
    }

	~bCorrectEquation()
	{ }
private:
	shared_ptr<dg::Equation<Type>> arg1;
	scalar factor_;
};

}
#endif
