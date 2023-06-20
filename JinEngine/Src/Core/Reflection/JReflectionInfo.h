#pragma once 
#include"../Singleton/JSingletonHolder.h"  
#include<memory> 
#include<vector>

namespace JinEngine
{ 
	namespace Core
	{
		class JTypeInfo; 
		class JEnumInfo;

		class JReflectionInfoPrivate;
		class JReflectionInfo final
		{
		private:
			template<typename T>friend class JCreateUsingNew;
		private:
			friend class JReflectionInfoPrivate;
			class JReflectionInfoImpl;
		private:
			std::unique_ptr<JReflectionInfoImpl> impl;
		public:
			//Get typeInfo by name			.. typeInfo default key is name	because all typeInfo has typename
			JTypeInfo* GetTypeInfo(const std::string& fullname)const noexcept; 
			JTypeInfo* GetTypeInfo(const size_t typeGuid)const noexcept; 
			//Get enumInfo by fullname			.. enumInfo default key is fullname( typeid(T).name()) because can't declare typename into enumspace
			JEnumInfo* GetEnumInfo(const std::string& fullname)const noexcept;
			JEnumInfo* GetEnumInfo(const size_t enumGuid)const noexcept;
			std::vector<JTypeInfo*> GetAllTypeInfo()const noexcept;
			std::vector<JTypeInfo*> GetDerivedTypeInfo(const JTypeInfo& baseType, const bool containBaseType = false)const noexcept;
		public:
			//Debug
			void SearchInstance();
		private:
			JReflectionInfo();
			~JReflectionInfo();
			JReflectionInfo(const JReflectionInfo& rhs) = delete;
			JReflectionInfo& operator=(const JReflectionInfo& rhs) = delete;
		};
	}
	using _JReflectionInfo = Core::JSingletonHolder<Core::JReflectionInfo>;
}