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
#include"JFSMparameterValueType.h"
#include"../File/JFileIOResult.h"
#include"../JCoreEssential.h" 

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMparameter;
		class JFSMparameterStoragePrivate;
		class JFSMparameterStorageUserInterface
		{
			friend class JFSMparameterStoragePrivate;
		public:
			virtual ~JFSMparameterStorageUserInterface() = default;
		private:
			virtual void NotifyRemoveParameter(const size_t guid)noexcept = 0;
		};

		__interface JFSMparameterStoragePublicAccess
		{
		public:
			size_t GetGuid()const noexcept;
			std::wstring GetParameterUniqueName(const std::wstring& initName)const noexcept;
			uint GetParameterCount()const noexcept;
			uint GetParameterMaxCount()const noexcept;
			JUserPtr<JFSMparameter> GetParameter(const size_t guid)const noexcept;
			JUserPtr<JFSMparameter> GetParameterByIndex(const uint index)const noexcept;
			std::vector<JUserPtr<JFSMparameter>> GetParameterVec()const noexcept;
		};

		__interface JFSMparameterStorageUserAccess : public JFSMparameterStoragePublicAccess
		{ 
		public:
			bool AddUser(JFSMparameterStorageUserInterface* user, const size_t guid)noexcept;
			bool RemoveUser(JFSMparameterStorageUserInterface* user, const size_t guid)noexcept;
		};	 
	}
}