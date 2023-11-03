#pragma once
#include"JFSMinterface.h"      
#include<vector>

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