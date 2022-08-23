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

		class JReflectionImpl
		{
		private:
			using TypeVec = std::vector<JTypeInfo*>;
			using TypeMap = std::unordered_map<std::string, JTypeInfo*>;
		private:
			struct JTypeData
			{
			public:
				TypeVec typeVec;
				TypeMap typeMap;
			};
		private:
			JTypeData jType;
			std::unordered_map<std::string, JEnumInfo*> jEnum;
		public:  
			void AddType(JTypeInfo* newType);
			void AddEnum(JEnumInfo* newEnum);
			//Get typeInfo by name
			JTypeInfo* GetTypeInfo(const std::string& name);
			//Get enumInfo by fullname
			JEnumInfo* GetEnumInfo(const std::string& fullname);
			std::vector<JTypeInfo*> GetDerivedTypeInfo(const JTypeInfo& baseType);
		};
		using JReflectionInfo = JSingletonHolder<JReflectionImpl>;
	}
}