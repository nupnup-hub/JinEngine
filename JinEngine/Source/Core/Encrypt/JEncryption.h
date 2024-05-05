#pragma once
#include"../JCoreEssential.h"
#include"../Unit/JBigInt.h" 
 
namespace JinEngine
{
	namespace Core
	{
		class RSAManager
		{
		private:
			const uint p1 = 0;	//소수
			const uint p2 = 0;	//소수
		public:
			const JBigInt n;		//p1 * p2
			const uint encryptKey;	//(p1 - 1) 과 (p2 - 1)하고 서로소
		private:
			const JBigInt decryptKey;	//오일러정리
		public:
			RSAManager(const uint p1, const uint p2, const uint encryptKey);
		public:
			std::string Decrypt(const JBigInt& c)const noexcept;
		};
		class RSAUser
		{
		public:
			std::string m;
		public:
			RSAUser(const std::string& message);
		public:
			JBigInt Encrypt(const JBigInt& e, const JBigInt& n)const noexcept;
		};

		class JSecurityObscurity
		{
		public:
			static bool ApplyXORF(const std::string& path, const std::string& pattern);
			static bool ApplyXORM(std::string& message, const std::string& pattern);
			static bool ApplyXORM(std::wstring& message, const std::string& pattern);
			static bool EncryptF(const std::string& path);
			static bool EncryptM(std::string& message);
			static bool EncryptM(std::wstring& message);
			static bool DecryptM(std::string& message);
			static bool DecryptM(std::wstring& message);
			static bool InputDummyData(const std::string& path, const uint dummyCount);
		};
	}
}