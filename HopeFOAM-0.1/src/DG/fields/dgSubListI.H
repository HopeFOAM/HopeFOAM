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
    Foam::dgSubList

Description
    Store the data of part of the field
    
Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class T>
inline Foam::dgSubList<T>::dgSubList()
{}

template<class T>
inline Foam::dgSubList<T>::dgSubList
(
    const UList<T>& list,
    const label subSize,
    const label startIndex
)
:
    UList<T>(&(list.v_[startIndex]), subSize)
{
    #ifdef FULLDEBUG

    // Artificially allow the start of a zero-sized subList to be
    // one past the end of the original list.
    if (subSize)
    {
        list.checkStart(startIndex);
        list.checkSize(startIndex + subSize);
    }
    else
    {
        // Start index needs to fall between 0 and size.  One position
        // behind the last element is allowed
        list.checkSize(startIndex);
    }
    #endif
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class T>
inline const Foam::dgSubList<T>& Foam::dgSubList<T>::null()
{
    return NullObjectRef<dgSubList<T>>();
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class T>
inline Foam::dgSubList<T>::operator const Foam::List<T>&() const
{
    return *reinterpret_cast<const List<T>* >(this);
}


template<class T>
inline void Foam::dgSubList<T>::operator=(const dgSubList<T>& sl)
{
    UList<T>::deepCopy(sl);
}


template<class T>
inline void Foam::dgSubList<T>::operator=(const UList<T>& l)
{
    UList<T>::deepCopy(l);
}


template<class T>
inline void Foam::dgSubList<T>::operator=(const T& t)
{
    UList<T>::operator=(t);
}

// ************************************************************************* //