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
#include"../JCoreEssential.h"
#include<type_traits> 

namespace JinEngine
{
	//class JObject;
	namespace Core
	{
		class JTypeInfo;  
		template<typename Type>
		class JTypeInfoInitializer
		{
		private:
			friend class JTypeInfo;
		private:
			std::string name;
			std::string fullName;
			size_t hashCode;
			JTypeInfo* parent;  
		public:
			JTypeInfoInitializer(const std::string& name)
				:name(name)
			{
				fullName = typeid(Type).name();
				hashCode = std::hash<std::string>{}(fullName);
				if constexpr (std::is_void_v<Type::ParentType>)
					parent = nullptr;
				else
					parent = &Type::ParentType::StaticTypeInfo();
			} 
		};
	}
} 