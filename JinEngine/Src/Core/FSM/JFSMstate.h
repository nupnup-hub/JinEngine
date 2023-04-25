#pragma once
#include"JFSMinterface.h" 
#include<memory>

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
				InitData(JUserPtr<JFSMdiagram> ownerDiagram);
				InitData(const std::wstring& name, const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram);
			public:
				InitData(const JTypeInfo& initTypeInfo, JUserPtr<JFSMdiagram> ownerDiagram);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram);
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
			Core::JIdentifierPrivate& GetPrivateInterface()const noexcept override;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			JFSMdiagram* GetOwner()const noexcept;
			uint GetTransitionCount()const noexcept;
			JFSMtransition* GetTransition(const size_t guid)noexcept;
			JFSMtransition* GetTransitionByOutGuid(const size_t outputGuid)noexcept;
			JFSMtransition* GetTransitionByIndex(uint index)noexcept; 
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
 