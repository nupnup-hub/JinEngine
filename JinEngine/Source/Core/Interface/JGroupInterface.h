#pragma once  
#include"JValidInterface.h" 
#include "../Pointer/JOwnerPtr.h" 

//Not Use
namespace JinEngine
{
	namespace Core
	{
		enum J_GROUP_FLAG
		{
			J_GROUP_FLAG_NONE = 0,
			J_GROUP_FLAG_DEREGISTER_IF_MEMBER_ZERO = 1 << 0
		};

		class JGroupMemberInterface;
		class JGroupHandle : public JValidInterface
		{
		private:
			friend class JGroupMemberInterface;
		private:
			size_t groupGuid; 
		};

		class JGroupInterface
		{ 
		public:
			using GroupType = int;
			class Impl;
			class Register;
		private: 
			std::unique_ptr<Impl> impl; 
		public:
			JGroupInterface();
			JGroupInterface(const size_t guid); 
			virtual ~JGroupInterface();
		public:
			size_t GetGroupGuid()const noexcept;
			std::vector<JUserPtr<JGroupMemberInterface>> GetMember()const noexcept;
		private:
			virtual void NotifyPushMember(const JUserPtr<JGroupMemberInterface>& memInterface);	//notify post push
			virtual void NotifyPopMember(const JUserPtr<JGroupMemberInterface>& memInterface);	//notify pre pop
		public: 
			static JUserPtr<JGroupInterface> RegisterGroup(JOwnerPtr<JGroupInterface>&& group);
			static bool DeRegisterGroup(const size_t guid);
		};

		class JGroupMemberInterface
		{ 
		public:
			using GroupMemberType = int; 
			class Register;
		private:
			const size_t guid; 
			JGroupHandle handle;
		public:
			static bool JoinGroup(const JUserPtr<JGroupMemberInterface>& memInterface, const size_t guid);
			static bool JoinGroup(const JUserPtr<JGroupMemberInterface>& memInterface, const JUserPtr<JGroupInterface>& group);
			bool LeaveGroup();
		private:
			virtual void NotifyJoinGroup();
			virtual void NotifyLeaveGroup();
		public:
			virtual GroupMemberType GetMemberType()const noexcept = 0;
			JUserPtr<JGroupInterface> GetGroupInterface()const noexcept;
		public:
			bool IsGroupMember()const noexcept;
		public:
			JGroupMemberInterface();
			JGroupMemberInterface(const size_t guid);
			~JGroupMemberInterface();
		};
	}
}