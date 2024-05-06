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


#include"JMethodInfo.h"
#include"JGuiWidgetInfoHandleBase.h"

namespace JinEngine
{
	namespace Core
	{
		std::string JMethodInfo::Name()const noexcept
		{
			return name;
		}
		std::string JMethodInfo::ReturnFieldName()const noexcept
		{
			return returnHint.name;
		}
		std::string JMethodInfo::ParameterFieldName()const noexcept
		{
			std::string names;
			for (int i = 0; i < parameterHint.size() - 1; ++i)
				names += parameterHint[i].name + ", ";
			names += parameterHint[parameterHint.size() - 1].name;
			return names;
		}
		uint JMethodInfo::ParameterCount()const noexcept
		{
			return (uint)parameterHint.size();
		}
		JParameterHint JMethodInfo::GetReturnHint()const noexcept
		{
			return returnHint;
		}
		JTypeInfo* JMethodInfo::GetTypeInfo()const noexcept
		{
			return ownerType;
		}
		JMethodOptionInfo* JMethodInfo::GetOptionInfo()const noexcept
		{
			return optionInfo;
		}
	}
}