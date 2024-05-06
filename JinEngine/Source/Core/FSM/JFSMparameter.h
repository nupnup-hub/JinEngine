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
#include"JFSMinterface.h"
#include"JFSMparameterValueType.h"

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMparameterPrivate; 
		__interface JFSMparameterStoragePublicAccess;
		class JFSMparameter final: public JFSMinterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMparameter)
		public: 
			class InitData final : public JFSMinterface::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				J_FSM_PARAMETER_VALUE_TYPE paramType;
				JUserPtr<JFSMparameterStoragePublicAccess> paramStorage = nullptr;
			public:
				InitData(const JUserPtr<JFSMparameterStoragePublicAccess>& paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL);
				InitData(const std::wstring& name, const size_t guid, const JUserPtr<JFSMparameterStoragePublicAccess>& paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL);
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JFSMparameterPrivate;
			class JFSMparameterImpl;
		private:
			std::unique_ptr<JFSMparameterImpl> impl;
		public:
			JIdentifierPrivate& PrivateInterface()const noexcept final;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			float GetValue()const noexcept;
			J_FSM_PARAMETER_VALUE_TYPE GetParamType()const noexcept; 
		public:
			void SetValue(const float value)noexcept;
			void SetValue(const int value)noexcept;
			void SetValue(const bool value)noexcept;
			void SetParamType(const J_FSM_PARAMETER_VALUE_TYPE paramType)noexcept;
		public:
			bool IsStorageParameter(const size_t storageGuid)const noexcept;
		private:
			JFSMparameter(const InitData& initData);
			~JFSMparameter();
			JFSMparameter(const JFSMparameter& rhs) = delete;
			JFSMparameter& operator=(const JFSMparameter& rhs) = delete;
		};
	}
}