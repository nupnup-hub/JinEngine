#include"JLazyDestruction.h"
#include"../Identity/JIdentifier.h"
#include"../Time/JGameTimer.h" 
#include"../../Utility/JCommonUtility.h"
#include"../../Object/Resource/Mesh/JStaticMeshGeometry.h"

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
				int index = JCUtil::GetJIndex(objectVec, equalPtr, ptr->GetGuid());
				if (index == -1)
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