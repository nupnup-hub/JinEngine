#pragma once
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
			class ApplicationInterface
			{
			private:
				friend class Application::JApplication;
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