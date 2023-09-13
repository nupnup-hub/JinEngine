#pragma once
namespace JinEngine
{
	class JMain;
	namespace Core
	{
		class JTypeInfo; 
		class JEnumInfo;
		class JReflectionInfoPrivate
		{
		public:
			class TypeInterface
			{
			private:
				friend class JTypeInfo;
			private:
				static void AddType(JTypeInfo* newType);
			};
			class EnumInterface
			{
			private:
				friend class JEnumInfo;
			private:
				static void AddEnum(JEnumInfo* newEnum);
			};
			class MainAccess
			{
			private: 
				friend class JMain;
			private:
				//it is valid once
				static void Initialize();
				static void Clear();
			private:
				static void Update();
			};
		};
	}
}