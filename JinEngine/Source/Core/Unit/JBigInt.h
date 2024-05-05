// reference: https://www.geeksforgeeks.org/bigint-big-integers-in-c-with-example/
#pragma once 
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		class JBigInt
		{
		public:
			std::string digits;
		public:
			//Constructors:
			JBigInt(size_t n = 0);
			JBigInt(const std::string&);
			JBigInt(const char*);
			JBigInt(const JBigInt&);
		public:
			std::string Integral()const noexcept;
			bool Null()const noexcept;
			int Length()const noexcept;
			void DiviedByTwo();
		public:
			/* * * * Operator Overloading * * * */

			//Indexing
			int operator[](const int)const;

			//Direct assignment
			JBigInt& operator=(const JBigInt&);

			//Post/Pre - Incrementation
			JBigInt& operator++();
			JBigInt operator++(int temp);
			JBigInt& operator--();
			JBigInt operator--(int temp);
		};
	}
}