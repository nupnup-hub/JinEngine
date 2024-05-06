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


#include"JEncryption.h" 
#include"../Unit/JBigIntEx.h"  
#include"../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		// a mod b
		// b가 a에 50%이상인경우 a >= b ? a mod b : a가 더 빠를수있으므로
		// 많은 mod 계산이 필요한 상황에서 고려해볼만하다.
		static int Mod(const std::string& num, const int n)
		{
			// Initialize result
			int res = 0;

			// One by one process all digits of 'num'
			for (int i = 0; i < num.length(); i++)
				res = (res * 10 + num[i] - '0') % n;

			return res;
		}
		static int Mod(const std::string& num, const int n, const int e)
		{
			int v = Mod(num, n);
			int sum = v;
			int result = 1;

			for (int i = 1; i < e; ++i)
			{
				const int local = sum % n;
				sum = (local * local) % n;
				if ((e & (1 << i)) > 0)
					result *= sum;
			}
			return result % n;
		}

		//RSA 개요.
		//암호화에는 mod 해독에는 이산로그를 구해야한다(일방향함수)
		//key는 pulbic, private 두가지가 존재하며 송신자들에게 public data를 전달하면 수신자들이 data를 이용해 messsage를 암호화
		//송신자가 최종적으로 private key를 이용해 암호를 푼다.
		//private는 두개에 소수와 오일러 파이함수를 이용하여 만들어지며 소인수분해에 성질을 가지므로 public data만으로는 
		//시행착오를 겪어야하며 이는 자리수가 커질수록 그 횟수가 기하급수적으로 늘어난다
		// 
		//  e = encrypt
		//  n = number (primeN1 * primeN2)
		//  m = message
		//	sender cal d
		//  -> m^p(n) = 1 mod n = m^(k * p(n)) = 1 mod n =  m * m^p(k * p(n)) = m mod n = m^(k * p(n) + 1) = m mod n = m^e*d
		//  d = (k * p(n) + 1) / e
		//	sender -> e, n -> receiver
		//	receiver encrypt -> m^e mod n = c -> sender
		//  sender decrypt -> c^d = m
		//
		// mod에 빠른 거듭제곱 개념이 사용된다 x^4 modY => (x^2 modY * x^2 modY) * modY
		RSAManager::RSAManager(const uint p1, const uint p2, const uint encryptKey)
			:p1(p1),
			p2(p2),
			n(JBigInt(p1)* JBigInt(p2)),
			encryptKey(encryptKey),
			decryptKey((2 * (JBigInt((p1 - 1)) * JBigInt((p2 - 1))) + 1) / encryptKey)
		{ }
		std::string RSAManager::Decrypt(const JBigInt& c)const noexcept
		{
			return ((c ^ decryptKey) % n).Integral();
		}

		RSAUser::RSAUser(const std::string& message)
			:m(message)
		{}
		JBigInt RSAUser::Encrypt(const JBigInt& e, const JBigInt& n)const noexcept
		{
			return (m ^ e) % n;
		}

		namespace
		{
			std::vector<char> encrypt
			{
				0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
				0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10,
				0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
				0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
				0x3d, 0x56, 0x48, 0x5d, 0x38, 0x3a, 0x77, 0x75,
				0x51, 0x28, 0x2f, 0x70, 0x7b, 0x47, 0x67, 0x72,
				0x74, 0x50, 0x6d, 0x68, 0x52, 0x7d, 0x53, 0x62,
				0x2b, 0x4a, 0x36, 0x35, 0x71, 0x4f, 0x30, 0x7e,
				0x42, 0x6f, 0x22, 0x27, 0x26, 0x44, 0x2e, 0x45,
				0x5b, 0x79, 0x24, 0x2d, 0x78, 0x21, 0x5e, 0x3f,
				0x39, 0x43, 0x7c, 0x66, 0x58, 0x32, 0x76, 0x34,
				0x7a, 0x69, 0x63, 0x5c, 0x25, 0x4d, 0x3b, 0x64,
				0x65, 0x5a, 0x40, 0x23, 0x5f, 0x31, 0x6a, 0x2c,
				0x41, 0x6b, 0x4c, 0x6e, 0x37, 0x57, 0x4e, 0x2a,
				0x60, 0x3c, 0x54, 0x49, 0x73, 0x29, 0x61, 0x33,
				0x4b, 0x6c, 0x55, 0x46, 0x3e, 0x59, 0x7f
			};
			std::vector<char> decrypt
			{
				0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
				0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10,
				0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
				0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
				0x4e, 0x43, 0x64, 0x4b, 0x5d, 0x45, 0x44, 0x2a,
				0x76, 0x70, 0x39, 0x68, 0x4c, 0x47, 0x2b, 0x3f,
				0x66, 0x56, 0x78, 0x58, 0x3c, 0x3b, 0x6d, 0x25,
				0x51, 0x26, 0x5f, 0x72, 0x21, 0x7d, 0x50, 0x63,
				0x69, 0x41, 0x52, 0x46, 0x48, 0x7c, 0x2e, 0x23,
				0x74, 0x3a, 0x79, 0x6b, 0x5e, 0x6f, 0x3e, 0x32,
				0x29, 0x35, 0x37, 0x73, 0x7b, 0x22, 0x6e, 0x55,
				0x7e, 0x62, 0x49, 0x5c, 0x24, 0x4f, 0x65, 0x71,
				0x77, 0x38, 0x5b, 0x60, 0x61, 0x54, 0x2f, 0x34,
				0x5a, 0x67, 0x6a, 0x7a, 0x33, 0x6c, 0x42, 0x2c,
				0x3d, 0x30, 0x75, 0x31, 0x28, 0x57, 0x27, 0x4d,
				0x4a, 0x59, 0x2d, 0x53, 0x36, 0x40, 0x7f
			};

		}

		bool JSecurityObscurity::ApplyXORF(const std::string& path, const std::string& pattern)
		{
			FILE* fin = nullptr;
			FILE* fout = nullptr;

			std::string copied = path + "Temp";

			fopen_s(&fin, path.c_str(), "rb");
			fopen_s(&fout, copied.c_str(), "wb");
			if (!fin || !fout)
				return false;

			static constexpr uint inBufSize = 8 * 1024;
			unsigned char inBuffer[inBufSize];
			size_t res = 0;
			size_t inSize = 0;
			size_t outSize = 0;

			const uint patternCount = (uint)pattern.size();
			do
			{
				res = fread(inBuffer, 1, inBufSize, fin);
				for (uint i = 0; i < res; ++i)
					inBuffer[i] ^= (pattern[i % patternCount] - '0');
				fwrite(inBuffer, 1, res, fout);
			} while (res && inBufSize);

			fclose(fin);
			fclose(fout);
			remove(path.c_str());
			rename(copied.c_str(), path.c_str());
		}
		bool JSecurityObscurity::ApplyXORM(std::string& message, const std::string& pattern)
		{
			const uint patternCount = (uint)pattern.size();
			const uint count = (uint)message.size();
			for (uint i = 0; i < count; ++i)
				message[i] ^= (pattern[i % patternCount] - '0');
			return true;
		}
		bool JSecurityObscurity::ApplyXORM(std::wstring& message, const std::string& pattern)
		{
			std::string str = JCUtil::WstrToU8Str(message);
			const bool res = ApplyXORM(str, pattern);
			message = JCUtil::U8StrToWstr(str);
			return res;
		}
		bool JSecurityObscurity::EncryptF(const std::string& path)
		{
			FILE* fin = nullptr;
			FILE* fout = nullptr;

			std::string copied = path + "Temp";

			fopen_s(&fin, path.c_str(), "rb");
			fopen_s(&fout, copied.c_str(), "wb");
			if (!fin || !fout)
				return false;

			static constexpr uint inBufSize = 8 * 1024;
			unsigned char inBuffer[inBufSize];
			size_t res = 0;
			size_t inSize = 0;
			size_t outSize = 0;

			do
			{
				res = fread(inBuffer, 1, inBufSize, fin);
				for (uint i = 0; i < res; ++i)
					inBuffer[i] = encrypt[inBuffer[i]];
				fwrite(inBuffer, 1, res, fout);
			} while (res && inBufSize);

			fclose(fin);
			fclose(fout);
			remove(path.c_str());
			rename(copied.c_str(), path.c_str());
		}
		bool JSecurityObscurity::EncryptM(std::string& path)
		{
			const uint count = (uint)path.size();
			for (uint i = 0; i < count; ++i)
				path[i] = encrypt[path[i]];
			return true;
		}
		bool JSecurityObscurity::EncryptM(std::wstring& message)
		{
			std::string str = JCUtil::WstrToU8Str(message);
			const bool res = EncryptM(str);
			message = JCUtil::U8StrToWstr(str);
			return res;
		}
		bool JSecurityObscurity::DecryptM(std::string& message)
		{
			const uint count = (uint)message.size();
			for (uint i = 0; i < count; ++i)
				message[i] = decrypt[message[i]];
			return true;
		}
		bool JSecurityObscurity::DecryptM(std::wstring& message)
		{
			std::string str = JCUtil::WstrToU8Str(message);
			const bool res = DecryptM(str);
			message = JCUtil::U8StrToWstr(str);
			return res;
		}
		bool JSecurityObscurity::InputDummyData(const std::string& path, const uint dummyCount)
		{
			if (dummyCount == 0)
				return false;

			FILE* file = nullptr;
			fopen_s(&file, path.c_str(), "r+b");
			if (!file)
				return false;

			std::string dummyStr;
			dummyStr.resize(dummyCount);
			for (uint i = 0; i < dummyCount; ++i)
				dummyStr[i] = ',';

			fwrite(dummyStr.c_str(), 1, dummyCount, file);
			fclose(file);
			return true;
		}


	}
}


/*
void CreateKey()
{
	std::set<char> set;
	std::vector<char> encrypt;
	std::vector<char> decrypt;
	encrypt.resize(128);
	decrypt.resize(128);

	std::cout << "Count: " << encrypt.size() << std::endl;
	std::cout << "Count: " << decrypt.size() << std::endl;

	std::random_device rd;	//use hardware
	std::mt19937 gen(rd());
	std::uniform_int_distribution<short> distr(0, 127);

	for (int i = 0; i < 128; ++i)
	{
		if (i < 33 || i == '\\' || i == 127)
		{
			encrypt[i] = i;
			set.emplace(i);
			continue;
		}

		bool pass = false;
		while (!pass)
		{
			auto random = distr(gen);
			if (random == i || random < 33 || random == 92 || random == 127)
				continue;

			auto data = set.find(random);
			if (data != set.end())
				continue;

			encrypt[i] = (char)random;
			set.emplace((char)random);
			pass = true;
		}
	}

	for (int i = 0; i < 128; ++i)
		decrypt[encrypt[i]] = i;


	std::cout << "Count: " << encrypt.size() << std::endl;
	for (int i = 0; i < encrypt.size(); ++i)
	{
		std::cout << "0x";
		std::cout << std::hex << (int)encrypt[i];
		std::cout << ", ";
		if (i % 8 == 0 && i > 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Count: " << decrypt.size() << std::endl;
	for (int i = 0; i < decrypt.size(); ++i)
	{
		std::cout << "0x";
		std::cout << std::hex << (int)decrypt[i];
		std::cout << ", ";
		if (i % 8 == 0 && i > 0)
			std::cout << std::endl;
	}

}
 */