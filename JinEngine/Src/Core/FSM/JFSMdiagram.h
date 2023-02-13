#pragma once
#include"JFSMInterface.h"
#include"JFSMparameterValueType.h"
#include"JFSMparameterStorageAccess.h"   
#include"../JDataType.h" 
#include<memory>
#include<string>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMstate; 
		class JFSMdiagramOwnerInterface;
		__interface JFSMparameterStorageUserAccess;

		class JFSMstateOwnerInterface : public JTypeCashInterface<JFSMstate>
		{
		private:
			friend class JFSMstate; 
		private:
			virtual JFSMparameterStorageUserAccess* GetParamStorageInterface()const noexcept = 0;
			virtual bool IsDiagramState(const size_t guid)const noexcept = 0;
		};

		class JFSMdiagram : public JFSMInterface, 
			public JFSMstateOwnerInterface,
			public JFSMparameterStorageUserInterface
		{
			REGISTER_CLASS(JFSMdiagram)
		public:
			struct JFSMdiagramInitData : public JFSMIdentifierInitData
			{
			public:
				JFSMdiagramOwnerInterface* ownerInterface = nullptr;
			public:
				JFSMdiagramInitData(const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* ownerInterface);
				JFSMdiagramInitData(const size_t guid, JFSMdiagramOwnerInterface* ownerInterface);
				JFSMdiagramInitData(JFSMdiagramOwnerInterface* ownerInterface);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			}; 
			using InitData = JFSMdiagramInitData;
		public:
			static constexpr uint maxNumberOffState = 100;
		private:
			//vector + unorered map => 64bit overhead 
			JFSMdiagramOwnerInterface* ownerInterface;
			std::vector<JFSMstate*> stateVec;
			std::unordered_map<size_t, JFSMstate*> stateMap;
			JFSMstate* initState = nullptr;
			size_t nowStateGuid; 
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final; 
			uint GetStateCount()const noexcept;
		protected:
			void Initialize()noexcept;
			JFSMstate* GetNowState()const noexcept;
			JFSMstate* GetState(const size_t guid)const noexcept;
			JFSMstate* GetStateByIndex(const uint index)const noexcept;
			std::vector<JFSMstate*>& GetStateVec()noexcept;
		private:
			JFSMparameterStorageUserAccess* GetParamStorageInterface()const noexcept final;
			bool IsDiagramState(const size_t guid)const noexcept final;
		private:
			bool AddType(JFSMstate* newState)noexcept final;
			bool RemoveType(JFSMstate* state)noexcept final;
		private:
			bool RegisterCashData()noexcept;
			bool DeRegisterCashData()noexcept;
		private:
			void Clear()noexcept override; 
		private:
			void NotifyRemoveParameter(const size_t guid)noexcept final;
		protected:
			JFSMdiagram(const JFSMdiagramInitData& initData);
			virtual ~JFSMdiagram();
			JFSMdiagram(const JFSMdiagram& rhs) = delete;
			JFSMdiagram& operator=(const JFSMdiagram& rhs) = delete;
		};
	}
}