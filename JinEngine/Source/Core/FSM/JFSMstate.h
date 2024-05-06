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
		class JFSMtransition; 
		class JFSMdiagram;  
		class JFSMstatePrivate;
		class JFSMstate : public JFSMinterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMstate)
		public: 
			class InitData : public JFSMinterface::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				JUserPtr<JFSMdiagram> ownerDiagram;
			public:
				InitData(const JUserPtr<JFSMdiagram>& ownerDiagram);
				InitData(const std::wstring& name, const size_t guid, const JUserPtr<JFSMdiagram>& ownerDiagram);
			public:
				InitData(const JTypeInfo& initTypeInfo, const JUserPtr<JFSMdiagram>& ownerDiagram);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, const JUserPtr<JFSMdiagram>& ownerDiagram);
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JFSMstatePrivate;
			class JFSMstateImpl;
		private:
			std::unique_ptr<JFSMstateImpl> impl;
		public:
			static constexpr uint GetMaxTransitionCapacity()noexcept
			{
				return 50;
			}
			Core::JIdentifierPrivate& PrivateInterface()const noexcept override;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			JUserPtr<JFSMdiagram> GetOwner()const noexcept;
			uint GetTransitionCount()const noexcept;
			JUserPtr<JFSMtransition>  GetTransition(const size_t guid)noexcept;
			JUserPtr<JFSMtransition>  GetTransitionByOutGuid(const size_t outputGuid)noexcept;
			JUserPtr<JFSMtransition> GetTransitionByIndex(uint index)noexcept;
		protected:
			void Initialize()noexcept;
			void Clear()noexcept;
		protected:
			void EnterState()noexcept;    
		protected:
			JFSMstate(const InitData& initData);
			~JFSMstate();
			JFSMstate(const JFSMstate& rhs) = delete;
			JFSMstate& operator=(const JFSMstate& rhs) = delete;
		};
	}
}
 