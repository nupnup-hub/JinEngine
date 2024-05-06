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