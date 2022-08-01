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
			std::vector<JTypeInfo*> jType;
			std::unordered_map<std::string, JEnumInfo*> jEnum;
		public:  
			void AddType(JTypeInfo* newType);
			void AddEnum(JEnumInfo* newEnum);
			JTypeInfo* GetTypeInfo(const std::string& name);
			JEnumInfo* GetEnumInfo(const std::string& name);
			std::vector<JTypeInfo*> GetDerivedTypeInfo(const JTypeInfo& baseType);
		};
		using JReflectionInfo = JSingletonHolder<JReflectionImpl>;
	}
}