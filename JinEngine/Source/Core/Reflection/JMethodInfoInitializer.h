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
#include"JParameter.h"
#include"JMethodOptionInfo.h"

namespace JinEngine
{
	namespace Core
	{
		class JTypeInfo;
		class JMethodInfo;
		class JCallableBase;
		class JCallableHintBase;

		template<typename Ret, typename ...Param>
		class JMethodInfoInitializer
		{
		private:
			friend class JMethodInfo;
		private:
			constexpr static char fence = '@';
		private:
			std::string name;
			std::string identificationName;
			JParameterHint returnHint = CreateParameterHint<Ret>(typeid(Ret).name());
			std::vector<JParameterHint> parameterHint;
			JCallableBase* callHandle;
			JCallableHintBase* hintHandle;
			JTypeInfo* ownerType;
		private:
			JMethodOptionInfo* optionInfo;
		public:
			JMethodInfoInitializer(const std::string& name,
				const std::string& identificationName,
				const std::string& funcPtrName,
				JCallableBase& callHandle,
				JCallableHintBase& hintHandle,
				JTypeInfo& ownerType,
				JMethodOptionInfo& optionInfo)
				:name(name),
				identificationName(identificationName),
				callHandle(&callHandle),
				hintHandle(&hintHandle),
				ownerType(&ownerType),
				optionInfo(&optionInfo)
			{
				std::vector<std::string> nameVec;
				ClassifyParamName((uint)sizeof...(Param), nameVec, funcPtrName);
				CreateParameterHint<Param...>(std::make_index_sequence<sizeof...(Param)>(), parameterHint, nameVec);
			}
		};
	}
}