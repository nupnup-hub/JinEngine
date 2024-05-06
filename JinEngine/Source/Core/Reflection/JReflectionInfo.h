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


#pragma once 
#include"../Singleton/JSingletonHolder.h"    

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