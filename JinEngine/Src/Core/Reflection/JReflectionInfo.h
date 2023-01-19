#pragma once 
#include"../Singleton/JSingletonHolder.h"  
#include<vector>  
#include<unordered_map>
#include<memory> 

namespace JinEngine
{ 
	namespace Core
	{
		class JTypeInfo;
		class JEnumInfo;

		class JReflectionInfoImpl
		{
		private:
			using TypeVec = std::vector<JTypeInfo*>;
			using TypeNameMap = std::unordered_map<std::string, JTypeInfo*>; 
		private:
			using EnumVec = std::vector<JEnumInfo*>;
			using EnumNameMap = std::unordered_map<std::string, JEnumInfo*>;
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
			//Get typeInfo by name				.. typeInfo default key is name	because all typeInfo has typename
			JTypeInfo* GetTypeInfo(const std::string& name)const noexcept; 
			//Get enumInfo by fullname			.. enumInfo default key is fullname( typeid(T).name()) because can't declare typename into enumspace
			JEnumInfo* GetEnumInfo(const std::string& fullname)const noexcept;
			//Find typeInfo by fullname
			JTypeInfo* FindTypeInfo(const std::string& fullname)const noexcept;
			//Find enumInfo by name
			JEnumInfo* FindEnumInfo(const std::string& name)const noexcept;
			std::vector<JTypeInfo*> GetDerivedTypeInfo(const JTypeInfo& baseType)const noexcept;
		public:
			//Debug
			void SearchInstance();
		};
		using JReflectionInfo = JSingletonHolder<JReflectionInfoImpl>;
	}
}