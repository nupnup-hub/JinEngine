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

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMstate;  
		class JFSMdiagramOwnerInterface; 
		class JFSMdiagramPrivate;
		class JFSMdiagram : public JFSMinterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMdiagram)
		public: 
			class InitData : public JFSMinterface::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				JFSMdiagramOwnerInterface* ownerInterface = nullptr;
			public:
				InitData(JFSMdiagramOwnerInterface* ownerInterface);
				InitData(const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* ownerInterface);
			public:
				InitData(const JTypeInfo& initTypeInfo, JFSMdiagramOwnerInterface* ownerInterface);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* ownerInterface);
			public:
				bool IsValidData()const noexcept override;
			};  
		private:
			friend class JFSMdiagramPrivate;
			class JFSMdiagramImpl;
		private:
			std::unique_ptr<JFSMdiagramImpl> impl;
		public:
			static constexpr uint GetMaxStateCapacity()noexcept
			{
				return 100;
			}
			JIdentifierPrivate& PrivateInterface()const noexcept override;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final; 
			uint GetStateCount()const noexcept; 
		public:
			JUserPtr<JFSMstate> GetNowState()const noexcept;
			JUserPtr<JFSMstate> GetState(const size_t guid)const noexcept;
			JUserPtr<JFSMstate> GetStateByIndex(const uint index)const noexcept;
			std::vector<JUserPtr<JFSMstate>> GetStateVec()noexcept;
		public:
			bool CanUseParameter(const size_t paramGuid)const noexcept;
			bool CanCreateState()const noexcept; 
		protected:
			void Initialize()noexcept;
			void Clear()noexcept;
		protected:
			JFSMdiagram(const InitData& initData);
			virtual ~JFSMdiagram();
			JFSMdiagram(const JFSMdiagram& rhs) = delete;
			JFSMdiagram& operator=(const JFSMdiagram& rhs) = delete;
		};
	}
}