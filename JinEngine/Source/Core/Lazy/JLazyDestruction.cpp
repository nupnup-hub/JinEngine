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


#include"JLazyDestruction.h"
#include"../Identity/JIdentifier.h"
#include"../Time/JGameTimer.h" 
#include"../Utility/JCommonUtility.h"   
namespace JinEngine
{
	namespace Core
	{
		struct ObjectInfo
		{
		public:
			JWeakPtr<JTypeBase> user;
			float stTime;
		public:
			ObjectInfo(JWeakPtr<JTypeBase> user)
				:user(user), stTime(0)
			{}
			~ObjectInfo() {}
		};

		class JLazyDestruction::JLazyDestructionImpl
		{
		public:
			std::unique_ptr<JLazyDestructionInfo> info;
		public:
			std::vector<std::unique_ptr<ObjectInfo>> objectVec;
		public:
			JLazyDestructionImpl(std::unique_ptr<JLazyDestructionInfo>&& info)
				:info(std::move(info))
			{}
		public:
			void Update(const float timeOffset)
			{ 
				for (int i = 0; i < objectVec.size(); ++i)
				{
					if (!objectVec[i]->user.IsValid())
					{ 
						objectVec.erase(objectVec.begin() + i);
						--i;
					}
				}
						 				 
				const float deltaTime = JEngineTimer::Data().DeltaTime() + timeOffset;
				for (int i = 0; i < objectVec.size(); ++i)
				{
					objectVec[i]->stTime += deltaTime;
					if (objectVec[i]->stTime >= info->waitTime)
						info->executeDestroy(objectVec[i]->user.Get());
				} 
			}
			void Clear()noexcept
			{
				info.reset();
				objectVec.clear();
			}
			bool AddUser(JTypeBase* ptr)
			{
				if (ptr == nullptr)
					return false;

				if (info->canDestroy != nullptr)
				{
					if (!info->canDestroy(ptr))
						return false;
				}
				 
				objectVec.emplace_back(std::make_unique<ObjectInfo>(Core::GetUserPtr(ptr)));
				if (info->notifyExecuteLazy != nullptr)
					info->notifyExecuteLazy(ptr);
				return true;
			}
			bool RemoveUser(JTypeBase* ptr)
			{
				if (ptr == nullptr)
					return false;

				if (info->canDestroy != nullptr)
				{
					if (!info->canDestroy(ptr))
						return false;
				}

				bool(*equalPtr)(ObjectInfo*, const size_t) = [](ObjectInfo* info, const size_t guid) {return info->user.IsValid() ? info->user->GetGuid() == guid : false; };
				int index = JCUtil::GetIndex(objectVec, equalPtr, ptr->GetGuid());
				if (index == invalidIndex)
					return false;

				if (info->notifyCancelLazy != nullptr)
					info->notifyCancelLazy(objectVec[index]->user.Get());
				objectVec.erase(objectVec.begin() + index);				 
				return true;
			}
		};
		JLazyDestruction::JLazyDestruction(std::unique_ptr<JLazyDestructionInfo>&& info)
			:impl(std::make_unique<JLazyDestructionImpl>(std::move(info)))
		{}
		JLazyDestruction::~JLazyDestruction()
		{
			impl.reset();
		}
		uint JLazyDestruction::GetWaitingCount()const noexcept
		{
			return impl->objectVec.size();
		}
		void JLazyDestruction::Update(const float timeOffset)noexcept
		{  
			impl->Update(timeOffset);
		}
		void JLazyDestruction::Clear()noexcept
		{
			impl->Clear();
		}
		bool JLazyDestruction::AddUser(JTypeBase* ptr)noexcept
		{
			return impl->AddUser(ptr);
		}
		bool JLazyDestruction::RemoveUser(JTypeBase* ptr)noexcept
		{
			return impl->RemoveUser(ptr);
		}
	}
}