/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JBigInt.h"

namespace JinEngine
{
	namespace Core
	{
		JBigInt::JBigInt(const std::string& s)
		{
			digits = "";
			int n = (int)s.size();
			for (int i = n - 1; i >= 0; i--)
			{
				if (!isdigit(s[i]))
					throw("ERROR");
				digits.push_back(s[i] - '0');
			}
		}
		JBigInt::JBigInt(size_t n)
		{
			do
			{
				digits.push_back(n % 10);
				n /= 10;
			} while (n);
		}
		JBigInt::JBigInt(const char* s)
		{
			digits = "";
			for (int i = strlen(s) - 1; i >= 0; i--)
			{
				if (!isdigit(s[i]))
					throw("ERROR");
				digits.push_back(s[i] - '0');
			}
		}
		JBigInt::JBigInt(const JBigInt& a)
		{
			digits = a.digits;
		}
		std::string JBigInt::Integral()const noexcept
		{
			std::string res;

			const int count = (int)digits.size();
			const int offset = count - 1;
			res.resize(count);

			for (int i = offset; i >= 0; i--)
				res[offset - i] = digits[i] + '0';
			return res;
		}
		bool JBigInt::Null()const noexcept
		{
			if (digits.size() == 1 && digits[0] == 0)
				return true;
			return false;
		}
		int JBigInt::Length()const noexcept
		{
			return digits.size();
		}
		void JBigInt::DiviedByTwo()
		{
			int add = 0;
			for (int i = digits.size() - 1; i >= 0; i--)
			{
				int digit = (digits[i] >> 1) + add;
				add = ((digits[i] & 1) * 5);
				digits[i] = digit;
			}
			while (digits.size() > 1 && !digits.back())
				digits.pop_back();
		}
		int JBigInt::operator[](const int index)const
		{
			if (digits.size() <= index || index < 0)
				throw("ERROR");
			return digits[index];
		}
		JBigInt& JBigInt::operator=(const JBigInt& a)
		{
			digits = a.digits;
			return *this;
		}
		JBigInt& JBigInt::operator++()
		{
			int i, n = (int)digits.size();
			for (i = 0; i < n && digits[i] == 9; i++)
				digits[i] = 0;
			if (i == n)
				digits.push_back(1);
			else
				digits[i]++;
			return *this;
		}
		JBigInt JBigInt::operator++(int temp)
		{
			JBigInt aux;
			aux = *this;
			++(*this);
			return aux;
		}
		JBigInt& JBigInt::operator--()
		{
			if (digits[0] == 0 && digits.size() == 1)
				throw("UNDERFLOW");
			int i, n = (int)digits.size();
			for (i = 0; digits[i] == 0 && i < n; i++)
				digits[i] = 9;
			digits[i]--;
			if (n > 1 && digits[n - 1] == 0)
				digits.pop_back();
			return *this;
		}
		JBigInt JBigInt::operator--(int temp)
		{
			JBigInt aux;
			aux = *this;
			--(*this);
			return aux;
		}
	}
}
 