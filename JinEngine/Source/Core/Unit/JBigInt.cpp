#include"JBigInt.h"

namespace JinEngine
{
	namespace Core
	{
		JBigInt::JBigInt(const std::string& s)
		{
			digits = "";
			int n = s.size();
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
			int i, n = digits.size();
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
			int i, n = digits.size();
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
 