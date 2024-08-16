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


#include"JFrameUpdateInterface.h"  
#include"../../Object/Component/JComponent.h"
#include"../../Object/Resource/JResourceObject.h"

namespace JinEngine
{
	namespace Graphic
	{
		uint JFrameDirtyChain::GetListenerCount()const noexcept
		{
			return (uint)listenerVec.size();
		}
		void JFrameDirtyChain::SetFrameDirty()noexcept
		{
			for (const auto& data : listenerVec)
				static_cast<JFrameUpdateInterface*>((*data.getFrameUserBind)())->GetDirtyBase()->SetFrameDirty();
		}
		bool JFrameDirtyChain::AddFrameDirtyListener(JFrameDirtyListener&& listener)noexcept
		{ 
			listenerVec.push_back(std::move(listener));
			return true;
		}
		bool JFrameDirtyChain::RemoveFrameDirtyListener(const size_t guid)noexcept
		{
			uint listenerCount = (uint)listenerVec.size();
			for (uint i = 0; i < listenerCount; ++i)
			{
				if (listenerVec[i].guid == guid)
				{
					listenerVec.erase(listenerVec.begin() + i);
					return true;
				}
			}
			return false;
		}

		uint JFrameDirtyBase::GetListenerCount()const noexcept
		{
			return dirtyChain == nullptr ? 0 : (uint)dirtyChain->GetListenerCount();
		}
		void JFrameDirtyBase::SetFrameDirty()noexcept
		{
			if (dirtyChain == nullptr)
				return;

			dirtyChain->SetFrameDirty();
		}
		bool JFrameDirtyBase::IsFrameHotDirted()const noexcept
		{
			return  GetFrameDirtyMax() > 0 && GetFrameDirty() == (GetFrameDirtyMax());
		}
		bool JFrameDirtyBase::IsLastFrameHotUpdated()const noexcept
		{
			return GetFrameDirty() == (GetFrameDirtyMax() - 1);
		}
		bool JFrameDirtyBase::AddFrameDirtyListener(JFrameDirtyListener&& listener)noexcept
		{
			if (dirtyChain == nullptr)
				dirtyChain = std::make_unique<JFrameDirtyChain>();

			return dirtyChain->AddFrameDirtyListener(std::move(listener));
		}
		bool JFrameDirtyBase::RemoveFrameDirtyListener(const size_t guid)noexcept
		{
			if (dirtyChain == nullptr)
				return false;

			return dirtyChain->RemoveFrameDirtyListener(guid);
		}

		int JFrameDirtyTrigger::GetFrameDirty()const noexcept
		{
			return 0;
		}
		int JFrameDirtyTrigger::GetFrameDirtyMax()const noexcept
		{
			return 0;
		}
		void JFrameDirtyTrigger::SetFrameDirty()noexcept
		{
			JFrameDirtyBase::SetFrameDirty();
		}
		bool JFrameDirtyTrigger::IsFrameDirted()const noexcept
		{
			return false;
		}
		bool JFrameDirtyTrigger::IsLastFrameUpdated()const noexcept
		{
			return false;
		} 
		void JFrameDirtyTrigger::OffFrameDirty()noexcept
		{

		}
		void JFrameDirtyTrigger::BeginUpdate()noexcept
		{ 
		}
		void JFrameDirtyTrigger::EndUpdate()noexcept
		{ 
		}


		int JFrameDirty::GetFrameDirty()const noexcept
		{
			return frameDirty;
		}
		int JFrameDirty::GetFrameDirtyMax()const noexcept
		{
			return Constants::gNumFrameResources;
		}
		void JFrameDirty::SetFrameDirty()noexcept
		{
			JFrameDirtyBase::SetFrameDirty();
			frameDirty = GetFrameDirtyMax();
		}
		bool JFrameDirty::IsFrameDirted()const noexcept
		{
			return frameDirty;
		}
		bool JFrameDirty::IsLastFrameUpdated()const noexcept
		{
			return isLastFrameUpdated;
		} 
		void JFrameDirty::OffFrameDirty()noexcept
		{
			frameDirty = 0;
		}
		void JFrameDirty::BeginUpdate()noexcept
		{
			isLastFrameUpdated = false;
		}
		void JFrameDirty::EndUpdate()noexcept
		{
			--frameDirty;
			if (frameDirty < 0)
				frameDirty = 0;
			isLastFrameUpdated = true;
		}

		int JFrameUpdateInterface::GetNumber(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type);
			return user != nullptr ? user->GetNumber() : invalidIndex;
		}
		int JFrameUpdateInterface::GetFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type); 
			return user != nullptr ? user->GetFrameIndex() : invalidIndex;
		}
		int JFrameUpdateInterface::GetFrameIndexSize(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type);
			return user != nullptr ? user->GetFrameIndexSize() : invalidIndex;
		}
		int JFrameUpdateInterface::GetLocalFrameCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			auto user = GetFrameInfo(type);
			return user != nullptr ? user->GetAreaInfo()->GetInfoCount() : invalidIndex;
		}
		void JFrameUpdateInterface::SetFrameDirty()noexcept
		{
			auto dirtyBase = GetDirtyBase();
			if (dirtyBase == nullptr)
				return;

			dirtyBase->SetFrameDirty();
		}
		bool JFrameUpdateInterface::IsDirted()const noexcept
		{
			auto dirtyBase = GetDirtyBase();
			if (dirtyBase == nullptr)
				return false;

			return dirtyBase->IsFrameDirted();
		}
		bool JFrameUpdateInterface::IsLastUpdated()const noexcept
		{
			auto dirtyBase = GetDirtyBase();
			if (dirtyBase == nullptr)
				return false;

			return dirtyBase->IsLastFrameUpdated();
		}
		void JFrameUpdateInterface::OffFrameDirty()noexcept
		{
			auto dirtyBase = GetDirtyBase();
			if (dirtyBase == nullptr)
				return;

			dirtyBase->OffFrameDirty();
		}
		void JFrameUpdateInterface::TryExecuteObjectUpdateBind()
		{
			if (objectUpdateB == nullptr)
				return;

			objectUpdateB->InvokeCompletelyBind();
		}
		bool JFrameUpdateInterface::TryRegisterDirtyListener(JFrameDirtyListener&& listener)
		{
			return GetDirtyBase()->AddFrameDirtyListener(std::move(listener));
		}
		bool JFrameUpdateInterface::TryRegisterDirtyListener(const JUserPtr<JObject>& obj)
		{
			JFrameDirtyListener listener;
			listener.guid = obj->GetGuid();

			if (obj->GetObjectType() == J_OBJECT_TYPE::COMPONENT_OBJECT)
			{
				auto listenerLam = [](JUserPtr<JObject> obj)
				{
					return static_cast<JComponent*>(obj.Get())->ModuleManagedData()->GetFrameUpdateUserInterface();
				};
				listener.getFrameUserBind = Core::UniqueBind(std::make_unique<JFrameDirtyListener::GetListenerF>(listenerLam), JUserPtr<JObject>(obj));
			}
			else if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				auto listenerLam = [](JUserPtr<JObject> obj)
				{
					return static_cast<JResourceObject*>(obj.Get())->ModuleManagedData()->GetFrameUpdateUserInterface();
				};
				listener.getFrameUserBind = Core::UniqueBind(std::make_unique<JFrameDirtyListener::GetListenerF>(listenerLam), JUserPtr<JObject>(obj));
			}
			else
				return false;

			return TryRegisterDirtyListener(std::move(listener));
		}
		bool JFrameUpdateInterface::TryDeRegisterDirtyListener(const size_t guid)
		{
			return GetDirtyBase()->RemoveFrameDirtyListener(guid); 
		}
		bool JFrameUpdateInterface::JFrameUpdateInterface::RegisterObjectUpdateB(JFrameObjectUpdateB&& bind)
		{
			objectUpdateB = std::move(bind);
			return true;
		}
		bool JFrameUpdateInterface::DeRegisterObjectUpdateB()
		{
			objectUpdateB = nullptr;
			return true;
		}
	}
}
