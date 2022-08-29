#pragma once
#include <cstdint>
#include <algorithm>
#include <functional>
#include <iostream>
#include <xutility>
#include "../Entity/Entity.h"
#include "../Registry/TypeViewBase.h"

/** If a function exists called SortCompare that takes (const T&, const T&) as parameters, it will automatically set it as the sorting algorithm*/
template <typename T>
concept Sortable = requires(T val0, T val1) { SortCompare(val0, val1); };

//https://en.wikibooks.org/wiki/Algorithm_Implementation/Sorting/Smoothsort

class LeonardoNumber
	
{
private:

	//inline static size_t LeonardoNumbers[]
	//{
	//	1, 1, 3, 5, 9, 15, 25, 41, 67, 109, 177, 287, 465, 753, 1219, 1973, 3193, 5167, 8361, 13529, 21891,
	//	35421, 57313, 92735, 150049, 242785, 392835, 635621, 1028457, 1664079, 2692537, 4356617, 7049155,
	//	11405773, 18454929, 29860703, 48315633, 78176337, 126491971, 204668309, 331160281, 535828591,
	//	866988873, 1402817465, 2269806339, 3672623805, 5942430145, 9615053951, 15557484097, 25172538049,
	//	40730022147, 65902560197, 106632582345, 172535142543, 279167724889, 451702867433, 730870592323,
	//	1182573459757, 1913444052081, 3096017511839, 5009461563921, 8105479075761, 13114940639683, 21220419715445
	//};

public:
	LeonardoNumber() : b(1), c(1) {}
	
	size_t gap() const
	{
		return b - c;
	}
	
	/**  Perform an "up" operation on the actual number.  **/
	LeonardoNumber& operator ++ ()
	{
		size_t s = b; b = b + c + 1; c = s; return *this;
	}

	/**  Perform a "down" operation on the actual number.  **/
	LeonardoNumber& operator -- ()
	{
		size_t s = c; c = b - c - 1; b = s; return *this;
	}

	/**  Return "companion" value.  **/
	size_t operator ~ () const
	{
		return c;
	}

	/**  Return "actual" value.  **/
	operator size_t() const
	{
		return b;
	}


private:
	size_t b;   /**  Actual number.  **/
	size_t c;   /**  Companion number.  **/
};

/**
*	Trinkles the roots of the stretches of a given array and root when the
*	adjacent stretches are trusty.
*
*	@param pArray: Pointer to the first element of the array in question.
*	@param root: Index of the root of the array in question.
*	@param concat: Standard concatenation's codification.
*	@param number: Current Leonardo number.
**/
template <typename T>
inline void semitrinkle(T* pArray, entityId* entityMapping, size_t root, size_t concat, LeonardoNumber number)
{

	if (!SortCompare(pArray[root - ~number], pArray[root]))
	{
		std::swap(pArray[root], pArray[root - ~number]);
		std::swap(entityMapping[root], entityMapping[root - ~number]);
		trinkle<T>(pArray, entityMapping, root - ~number, concat, number);
	}
}

/**
*	Trinkles the roots of the stretches of a given array and root.
*
*   @param pArray: Pointer to the first element of the array in question.
*   @param root: Index of the root of the array in question.
*   @param concat: Standard concatenation's codification.
*   @param number: Current Leonardo number.
**/
template <typename T>
inline void trinkle(T* pArray, entityId* entityMapping, size_t root, size_t concat, LeonardoNumber number)
{

	while (concat)
	{
		for (; !(concat % 2); concat >>= 1)  ++number;

		if (!--concat || !SortCompare(pArray[root], pArray[root - number]))  break;
		else
			if (number == 1)
			{
				std::swap(pArray[root], pArray[root - number]);
				std::swap(entityMapping[root], entityMapping[root - number]);
				root -= number;
			}

			else if (number >= 3)
			{
				size_t r2 = root - number.gap(), r3 = root - number;

				if (!SortCompare(pArray[root - 1], pArray[r2]))
				{
					r2 = root - 1; concat <<= 1; --number;
				}

				if (!SortCompare(pArray[r3], pArray[r2]))
				{
					std::swap(pArray[root], pArray[r3]);
					std::swap(entityMapping[root], entityMapping[r3]);
					root = r3;
				}

				else
				{
					std::swap(pArray[root], pArray[r2]);
					std::swap(entityMapping[root], entityMapping[r2]);
					root = r2; --number; break;
				}
			}
	}

	sift<T>(pArray, entityMapping, root, number);

}

/**
*	Sifts up the root of the stretch in question.
*
*	@param pArray: Pointer to the first element of the array in question.
*   @param root: Index of the root of the array in question.
*   @param number: Current Leonardo number.
**/
template <typename T>
inline void sift(T* pArray, entityId* entityMapping, size_t root, LeonardoNumber number)
{
	size_t r2;

	while (number >= 3)
	{
		if (!SortCompare(pArray[root - number.gap()], pArray[root - 1]))
			r2 = root - number.gap();
		else
		{
			r2 = root - 1; --number;
		}

		if (!SortCompare(pArray[root], pArray[r2]))  break;
		else
		{
			std::swap(pArray[root], pArray[r2]);
			std::swap(entityMapping[root], entityMapping[r2]);
			root = r2; --number;
		}
	}
}

template <typename T>
void SmoothSort(T* pArray, entityId* entityMapping, const volatile ViewDataFlag& viewFlag, size_t size)
{
	if constexpr (Sortable<T>)
	{
		if (!(pArray && size)) return;

		size_t p = 1;
		LeonardoNumber b;

		for (size_t q = 0; ++q < size; ++p)
		{
			if (viewFlag != ViewDataFlag::sorting)
				return;

			if ((p & 0b111) == 3)
			{
				sift<T>(pArray, entityMapping, q - 1, b);

				++++b; p >>= 2;
			}

			else if ((p & 0b11) == 1)
			{
				if (q + ~b < size)  sift<T>(pArray, entityMapping, q - 1, b);
				else  trinkle<T>(pArray, entityMapping, q - 1, p, b);

				for (p <<= 1; --b > 1; p <<= 1);
			}
		}
		trinkle<T>(pArray, entityMapping, size - 1, p, b);

		for (--p; size-- > 1; --p)
			if (b == 1)
				for (; !(p % 2); p >>= 1)  ++b;

			else if (b >= 3)
			{
				if (p)  semitrinkle<T>(pArray, entityMapping, size - b.gap(), p, b);

				--b; p <<= 1; ++p;
				semitrinkle<T>(pArray, entityMapping, size - 1, p, b);
				--b; p <<= 1; ++p;
			}
	}
}



