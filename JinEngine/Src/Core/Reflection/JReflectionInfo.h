#pragma once 
#include"../Singleton/JSingletonHolder.h"  
#include<vector>  
#include<unordered_map>
#include<memory> 

namespace JinEngine
{ 
	namespace Application
	{
		class JApplication;
	}

	namespace Core
	{
		class JTypeInfo;
		class JEnumInfo;

		class JReflectionInfoImpl
		{
		private:
			friend class Application::JApplication;
		private:
			using TypeVec = std::vector<JTypeInfo*>;
			using TypeNameMap = std::unordered_map<size_t, JTypeInfo*>; 
		private:
			using EnumVec = std::vector<JEnumInfo*>;
			using EnumNameMap = std::unordered_map<size_t, JEnumInfo*>;
		private:
			struct JTypeData
			{
			public:
				TypeVec typeVec;
				TypeNameMap typeNameMap; 
			};
			struct JEnumData
			{
			public:
				EnumVec enumVec;
				EnumNameMap enumNameMap;
			};
		private:
			JTypeData jType;
			JEnumData jEnum; 
		public:  
			void AddType(JTypeInfo* newType);
			void AddEnum(JEnumInfo* newEnum);
		public:
			//Get typeInfo by name			.. typeInfo default key is name	because all typeInfo has typename
			JTypeInfo* GetTypeInfo(const std::string& fullname)const noexcept; 
			JTypeInfo* GetTypeInfo(const size_t typeGuid)const noexcept;
			//Get enumInfo by fullname			.. enumInfo default key is fullname( typeid(T).name()) because can't declare typename into enumspace
			JEnumInfo* GetEnumInfo(const std::string& fullname)const noexcept;
			JEnumInfo* GetEnumInfo(const size_t enumGuid)const noexcept;
			std::vector<JTypeInfo*> GetAllTypeInfo()const noexcept;
			std::vector<JTypeInfo*> GetDerivedTypeInfo(const JTypeInfo& baseType)const noexcept;
		public:
			//Debug
			void SearchInstance();
		private:
			//it is valid once
			void Initialize();
			void Clear(); 
		};
		using JReflectionInfo = JSingletonHolder<JReflectionInfoImpl>;
	}
}