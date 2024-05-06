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


#include"JFileConstant.h"
#include"../Reflection/JTypeInfo.h"
#include"../Utility/JCommonUtility.h"
#include<fstream> 
namespace JinEngine
{
	namespace Core
	{ 
		std::string JFileConstant::GetLastOpenSymbol(const JTypeInfo& typeInfo)noexcept
		{
			return typeInfo.Name() + "IsOpen: ";
		}
		std::string JFileConstant::GetLineSymbol(const std::string& notification)noexcept
		{
			return "_________" + notification + "_________";
		}
		std::wstring JFileConstant::GetLastOpenSymbolW(const JTypeInfo& typeInfo)noexcept
		{
			return JCUtil::StrToWstr(typeInfo.Name()) + L"IsOpen: ";
		}
		std::wstring JFileConstant::GetLineSymbolW(const std::wstring& notification)noexcept
		{
			return L"_________" + notification + L"_________";
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name)
		{
			return folderPath + L"\\" + name;
		}
		std::wstring JFileConstant::MakeFilePath(const std::wstring& folderPath, const std::wstring& name, const std::wstring& format)
		{
			return folderPath + L"\\" + name + format;
		}
	}
}