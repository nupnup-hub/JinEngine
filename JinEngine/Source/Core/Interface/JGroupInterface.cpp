#include"JGroupInterface.h"
#include"../Guid/JGuidCreator.h"
#include<unordered_map>

//Not Use
namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			using PushMemPtr = bool(*)(const JUserPtr<JGroupInterface>&, const JUserPtr<JGroupMemberInterface>&);
			using PopMemPtr = bool(*)(const JUserPtr<JGroupInterface>&, const size_t);

			using NotifyPushMemPtr = void(JGroupInterface::*)(const JUserPtr<JGroupMemberInterface>&);
			using NotifyPopMemPtr = void(JGroupInterface::*)(const JUserPtr<JGroupMemberInterface>&);

			using GetMemberGuidPtr = size_t(*)(JGroupMemberInterface*);
			using LeaveMemPtr = bool(JGroupMemberInterface::*)();

			struct PrivateData
			{
			public:
				PushMemPtr pushMemPtr;
				PopMemPtr popMemPtr;
			public:
				NotifyPushMemPtr notifyPushPtr;
				NotifyPopMemPtr notifyPopPtr; 
			public:
				GetMemberGuidPtr getMemGuidPtr;
				LeaveMemPtr leaveMemPtr;
			};
			 

			//key is JGroupInterface guid
			static std::unique_ptr<PrivateData> privateData;
			static std::unordered_map<size_t, JOwnerPtr<JGroupInterface>> groupMap;
		}

		class JGroupInterface::Impl
		{
		public:
			const size_t guid;
			std::vector<JUserPtr<JGroupMemberInterface>> member;
		public:
			JGroupInterface::Impl(const size_t guid)
				:guid(guid)
			{};
		public:
			static bool PushMember(const JUserPtr<JGroupInterface>& gInterface, const JUserPtr<JGroupMemberInterface>& newMember)
			{
				if (gInterface == nullptr || newMember == nullptr || newMember->IsGroupMember())
					return false;

				auto groupData = groupMap.find(gInterface->GetGroupGuid());
				if (groupData == groupMap.end())
					return false;

				gInterface->impl->member.push_back(newMember);
				(gInterface.Get()->*privateData->notifyPushPtr)(newMember);
				return true;
			}
			static bool PopMember(const JUserPtr<JGroupInterface>& gInterface, const size_t guid)
			{ 
				if (gInterface == nullptr)
					return false;

				auto& member = gInterface->impl->member;
				const uint count = member.size();
				for (uint i = 0; i < count; ++i)
				{
					if (privateData->getMemGuidPtr(member[i].Get()) == guid)
					{ 
						(gInterface.Get()->*privateData->notifyPopPtr)(member[i]);
						member.erase(member.begin() + i);
						return true;
					}
				}
				return false;
			}
		};

		static class JGroupInterface::Register
		{
		public:
			Register()
			{
				if (privateData == nullptr)
					privateData = std::make_unique<PrivateData>();
				privateData->notifyPushPtr = &JGroupInterface::NotifyPushMember;
				privateData->notifyPopPtr = &JGroupInterface::NotifyPopMember;
				privateData->pushMemPtr = [](const JUserPtr<JGroupInterface>& group, const JUserPtr<JGroupMemberInterface>& mem)
				{
					return JGroupInterface::Impl::PushMember(group, mem);
				};
				privateData->popMemPtr = [](const JUserPtr<JGroupInterface>& group, const size_t guid)
				{
					return JGroupInterface::Impl::PopMember(group, guid);
				};
			}
		};

		static class JGroupMemberInterface::Register
		{
		public:
			Register()
			{
				if (privateData == nullptr)
					privateData = std::make_unique<PrivateData>();
				privateData->getMemGuidPtr = [](JGroupMemberInterface* mem) {return mem->guid; };
				privateData->leaveMemPtr = &JGroupMemberInterface::LeaveGroup;
			}
		};

		JGroupInterface::JGroupInterface()
			:impl(std::make_unique<Impl>(MakeGuid()))
		{}
		JGroupInterface::JGroupInterface(const size_t guid)
			: impl(std::make_unique<Impl>(guid))
		{} 
		JGroupInterface::~JGroupInterface(){}
		size_t JGroupInterface::GetGroupGuid()const noexcept
		{
			return impl->guid;
		}
		std::vector<JUserPtr<JGroupMemberInterface>> JGroupInterface::GetMember()const noexcept
		{
			return impl->member;
		}
		void JGroupInterface::NotifyPushMember(const JUserPtr<JGroupMemberInterface>& memInterface){}
		void JGroupInterface::NotifyPopMember(const JUserPtr<JGroupMemberInterface>& memInterface) {}
		JUserPtr<JGroupInterface> JGroupInterface::RegisterGroup(JOwnerPtr<JGroupInterface>&& group)
		{
			if (group == nullptr)
				return nullptr;

			auto groupData = groupMap.find(group->GetGroupGuid());
			if (groupData != groupMap.end())
				return nullptr;

			JUserPtr<JGroupInterface> userPtr = group;
			groupMap.emplace(group->GetGroupGuid(), std::move(group));
			return userPtr;
		} 
		bool JGroupInterface::DeRegisterGroup(const size_t guid)
		{
			auto groupData = groupMap.find(guid);
			if (groupData == groupMap.end())
				return false;

			auto member = groupData->second->GetMember();
			for (auto data : member)
				(data.Get()->*privateData->leaveMemPtr)();
			
			groupMap.erase(guid);
			return true;
		}
		bool JGroupMemberInterface::JoinGroup(const JUserPtr<JGroupMemberInterface>& memInterface, const size_t guid)
		{
			auto data = groupMap.find(guid);
			if (data == groupMap.end())
				return false;

			return JoinGroup(memInterface, data->second);
		}
		bool JGroupMemberInterface::JoinGroup(const JUserPtr<JGroupMemberInterface>& memInterface, const JUserPtr<JGroupInterface>& group)
		{
			if (memInterface == nullptr || group == nullptr)
				return false;

			const bool res = privateData->pushMemPtr(group, memInterface);
			if (res)
			{
				memInterface->handle.groupGuid = group->GetGroupGuid();
				memInterface->handle.SetValid(true);
				memInterface->NotifyJoinGroup();
			}
			return res;
		}
		bool JGroupMemberInterface::LeaveGroup()
		{
			if (!IsGroupMember())
				return false;

			auto groupData = groupMap.find(handle.groupGuid);
			const bool res = privateData->popMemPtr(groupData->second, guid);
			if (res)
			{
				handle.SetValid(false);
				NotifyLeaveGroup();
			}
			return res;
		}
		void JGroupMemberInterface::NotifyJoinGroup(){}
		void JGroupMemberInterface::NotifyLeaveGroup(){}
		JUserPtr<JGroupInterface> JGroupMemberInterface::GetGroupInterface()const noexcept
		{
			if (!IsGroupMember())
				return nullptr;

			return groupMap.find(handle.groupGuid)->second;
		}
		bool JGroupMemberInterface::IsGroupMember()const noexcept
		{
			return handle.IsValid();
		}
		JGroupMemberInterface::JGroupMemberInterface()
			:guid(MakeGuid())
		{}
		JGroupMemberInterface::JGroupMemberInterface(const size_t guid)
			: guid(guid)
		{}
		JGroupMemberInterface::~JGroupMemberInterface()
		{
			LeaveGroup();
		}
	}
}