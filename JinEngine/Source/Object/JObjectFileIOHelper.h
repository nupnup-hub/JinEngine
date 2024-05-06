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
#include"JObject.h"
#include"../Core/File/JFileIOHelper.h"
namespace JinEngine
{ 
	class JComponent;
	class JObjectFileIOHelper : public JFileIOHelper
	{ 
	private: 
		using JFileIOHelper::StoreHasIden;
		using JFileIOHelper::LoadHasIden;
	public:
		static Core::J_FILE_IO_RESULT StoreObjectIden(JFileIOTool& tool, JObject* obj);
		static Core::J_FILE_IO_RESULT LoadObjectIden(JFileIOTool& tool, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag);
		static Core::J_FILE_IO_RESULT LoadObjectIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag);
	public:
		static Core::J_FILE_IO_RESULT StoreComponentIden(JFileIOTool& tool, JComponent* comp);
		static Core::J_FILE_IO_RESULT LoadComponentIden(JFileIOTool& tool, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated);
		static Core::J_FILE_IO_RESULT LoadComponentIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated);
	public:
		static Core::J_FILE_IO_RESULT _StoreHasIden(JFileIOTool& tool, Core::JIdentifier* iden, const std::string& guide, const bool applyUncopyableSymbol = false);
		/**
		* can use identifier child class
		* but try again ptr only valid object child class
		*/
		static JUserPtr<Core::JIdentifier> _LoadHasIden(JFileIOTool& tool, const std::string& key = "", const bool applyUncopyableSymbol = false);
		template<typename T>
		static JUserPtr<T> _LoadHasIden(JFileIOTool& tool, const std::string& key = "", const bool applyUncopyableSymbol = false)
		{
			return JUserPtr<T>::ConvertChild(_LoadHasIden(tool, key));
		} 
	};
}