// Original: https://www.geeksforgeeks.org/bigint-big-integers-in-c-with-example/
#pragma once
#include"JBigInt.h"
#include<iostream>

namespace JinEngine
{
	namespace Core
	{
		//Addition and Subtraction
		static JBigInt& operator+=(JBigInt& a, const JBigInt& b);
		static JBigInt operator+(const JBigInt& a, const JBigInt& b);
		static JBigInt& operator-=(JBigInt& a, const JBigInt& b);
		static JBigInt operator-(const JBigInt& a, const JBigInt& b);

		//Multiplication and Division
		static JBigInt& operator*=(JBigInt& a, const JBigInt& b);
		static JBigInt operator*(const JBigInt& a, const JBigInt& b);
		static JBigInt& operator/=(JBigInt& a, const JBigInt& b);
		static JBigInt operator/(const JBigInt& a, const JBigInt& b);

		//Modulo
		static JBigInt& operator%=(JBigInt& a, const JBigInt& b);
		static JBigInt operator%(const JBigInt& a, const JBigInt& b);

		//Power Function
		static JBigInt& operator^=(JBigInt& a, const JBigInt& b);
		static JBigInt operator^(const JBigInt& a, const JBigInt& b);

		//Comparison operators
		static bool operator==(const JBigInt& a, const JBigInt& b);
		static bool operator!=(const JBigInt& a, const JBigInt& b);
		static bool operator<(const JBigInt& a, const JBigInt& b);
		static bool operator>(const JBigInt& a, const JBigInt& b);
		static bool operator>=(const JBigInt& a, const JBigInt& b);
		static bool operator<=(const JBigInt& a, const JBigInt& b);

		//Read and Write
		static std::istream& operator>>(std::istream& in, JBigInt& a);
		static std::ostream& operator<<(std::ostream& out, const JBigInt& a);

		static JBigInt sqrt(JBigInt& a);
		static JBigInt NthCatalan(int n);
		static JBigInt NthFibonacci(int n);
		static JBigInt Factorial(int n);

		static JBigInt& operator+=(JBigInt& a, const JBigInt& b)
		{
			int t = 0, s, i;
			int n = a.Length(), m = b.Length();
			if (m > n)
				a.digits.append(m - n, 0);
			n = a.Length();
			for (i = 0; i < n; i++)
			{
				if (i < m)
					s = (a.digits[i] + b.digits[i]) + t;
				else
					s = a.digits[i] + t;
				t = s / 10;
				a.digits[i] = (s % 10);
			}
			if (t)
				a.digits.push_back(t);
			return a;
		}
		static JBigInt operator+(const JBigInt& a, const JBigInt& b)
		{
			JBigInt temp;
			temp = a;
			temp += b;
			return temp;
		}
		static JBigInt& operator-=(JBigInt& a, const JBigInt& b)
		{
			if (a < b)
				throw("UNDERFLOW");
			int n = a.Length(), m = b.Length();
			int i, t = 0, s;
			for (i = 0; i < n; i++)
			{
				if (i < m)
					s = a.digits[i] - b.digits[i] + t;
				else
					s = a.digits[i] + t;
				if (s < 0)
					s += 10,
					t = -1;
				else
					t = 0;
				a.digits[i] = s;
			}
			while (n > 1 && a.digits[n - 1] == 0)
				a.digits.pop_back(),
				n--;
			return a;
		}
		static JBigInt operator-(const JBigInt& a, const JBigInt& b)
		{
			JBigInt temp;
			temp = a;
			temp -= b;
			return temp;
		}

		static JBigInt& operator*=(JBigInt& a, const JBigInt& b)
		{
			if (a.Null() || b.Null())
			{
				a = JBigInt();
				return a;
			}
			int n = a.digits.size(), m = b.digits.size();
			std::vector<int> v(n + m, 0);
			for (int i = 0; i < n; i++)
			{
				for (int j = 0; j < m; j++)
					v[i + j] += (a.digits[i]) * (b.digits[j]);
			}
			n += m;
			a.digits.resize(v.size());
			for (int s, i = 0, t = 0; i < n; i++)
			{
				s = t + v[i];
				v[i] = s % 10;
				t = s / 10;
				a.digits[i] = v[i];
			}
			for (int i = n - 1; i >= 1 && !v[i]; i--)
				a.digits.pop_back();
			return a;
		}
		static JBigInt operator*(const JBigInt& a, const JBigInt& b)
		{
			JBigInt temp;
			temp = a;
			temp *= b;
			return temp;
		}
		static JBigInt& operator/=(JBigInt& a, const JBigInt& b)
		{
			if (b.Null())
				throw("Arithmetic Error: Division By 0");
			if (a < b)
			{
				a = JBigInt();
				return a;
			}
			if (a == b)
			{
				a = JBigInt(1);
				return a;
			}
			int i, lgcat = 0, cc;
			int n = a.Length(), m = b.Length();
			std::vector<int> cat(n, 0);
			JBigInt t;
			for (i = n - 1; t * 10 + a.digits[i] < b; i--)
			{
				t *= 10;
				t += a.digits[i];
			}
			for (; i >= 0; i--)
			{
				t = t * 10 + a.digits[i];
				for (cc = 9; cc * b > t; cc--);
				t -= cc * b;
				cat[lgcat++] = cc;
			}
			a.digits.resize(cat.size());
			for (i = 0; i < lgcat; i++)
				a.digits[i] = cat[lgcat - i - 1];
			a.digits.resize(lgcat);
			return a;
		}
		static JBigInt operator/(const JBigInt& a, const JBigInt& b)
		{
			JBigInt temp;
			temp = a;
			temp /= b;
			return temp;
		}

		static JBigInt& operator%=(JBigInt& a, const JBigInt& b)
		{
			if (b.Null())
				throw("Arithmetic Error: Division By 0");
			if (a < b)
			{
				return a;
			}
			if (a == b)
			{
				a = JBigInt();
				return a;
			}
			int i, lgcat = 0, cc;
			int n = a.Length(), m = b.Length();
			std::vector<int> cat(n, 0);
			JBigInt t;
			for (i = n - 1; t * 10 + a.digits[i] < b; i--)
			{
				t *= 10;
				t += a.digits[i];
			}
			for (; i >= 0; i--)
			{
				t = t * 10 + a.digits[i];
				for (cc = 9; cc * b > t; cc--);
				t -= cc * b;
				cat[lgcat++] = cc;
			}
			a = t;
			return a;
		}
		static JBigInt operator%(const JBigInt& a, const JBigInt& b)
		{
			JBigInt temp;
			temp = a;
			temp %= b;
			return temp;
		}

		static JBigInt& operator^=(JBigInt& a, const JBigInt& b)
		{
			JBigInt Exponent, Base(a);
			Exponent = b;
			a = 1;
			while (!Exponent.Null())
			{
				if (Exponent[0] & 1)
					a *= Base;
				Base *= Base;
				Exponent.DiviedByTwo();
			}
			return a;
		}
		static JBigInt operator^(const JBigInt& a, const JBigInt& b)
		{
			JBigInt temp(a);
			temp ^= b;
			return temp;
		}

		static bool operator==(const JBigInt& a, const JBigInt& b)
		{
			return a.digits == b.digits;
		}
		static bool operator!=(const JBigInt& a, const JBigInt& b)
		{
			return !(a == b);
		}
		static bool operator<(const JBigInt& a, const JBigInt& b)
		{
			int n = a.Length(), m = b.Length();
			if (n != m)
				return n < m;
			while (n--)
			{
				if (a.digits[n] != b.digits[n])
					return a.digits[n] < b.digits[n];
			}
			return false;
		}
		static bool operator>(const JBigInt& a, const JBigInt& b)
		{
			return b < a;
		}
		static bool operator>=(const JBigInt& a, const JBigInt& b)
		{
			return !(a < b);
		}
		static bool operator<=(const JBigInt& a, const JBigInt& b)
		{
			return !(a > b);
		}

		static std::istream& operator>>(std::istream& in, JBigInt& a)
		{
			std::string s;
			in >> s;
			int n = s.size();
			for (int i = n - 1; i >= 0; i--)
			{
				if (!isdigit(s[i]))
					throw("INVALID NUMBER");
				a.digits[n - i - 1] = s[i];
			}
			return in;
		}
		static std::ostream& operator<<(std::ostream& out, const JBigInt& a)
		{
			for (int i = a.digits.size() - 1; i >= 0; i--)
				std::cout << (short)a.digits[i];
			return std::cout;
		}
		template<int bitCount>
		static JBigInt CreateByBitSet(const std::bitset<bitCount>& bit)
		{
			JBigInt res;
			JBigInt two(2);
			for (int i = 0; i < bitCount; ++i)
			{
				if (bit[i])
					res += two ^ i;
			}
			return res;
		}

		static JBigInt sqrt(JBigInt& a)
		{
			JBigInt left(1), right(a), v(1), mid, prod;
			right.DiviedByTwo();
			while (left <= right)
			{
				mid += left;
				mid += right;
				mid.DiviedByTwo();
				prod = (mid * mid);
				if (prod <= a)
				{
					v = mid;
					++mid;
					left = mid;
				}
				else
				{
					--mid;
					right = mid;
				}
				mid = JBigInt();
			}
			return v;
		}
		static JBigInt NthCatalan(int n)
		{
			JBigInt a(1), b;
			for (int i = 2; i <= n; i++)
				a *= i;
			b = a;
			for (int i = n + 1; i <= 2 * n; i++)
				b *= i;
			a *= a;
			a *= (n + 1);
			b /= a;
			return b;
		}
		static JBigInt NthFibonacci(int n)
		{
			JBigInt a(1), b(1), c;
			if (!n)
				return c;
			n--;
			while (n--)
			{
				c = a + b;
				b = a;
				a = c;
			}
			return b;
		}
		static JBigInt Factorial(int n)
		{
			JBigInt f(1);
			for (int i = 2; i <= n; i++)
				f *= i;
			return f;
		}
	}
}
 