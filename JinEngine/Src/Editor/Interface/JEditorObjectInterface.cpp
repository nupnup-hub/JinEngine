#include"JEditorObjectInterface.h"
#include"../../Core/Identity/JIdentifier.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Resource/Scene/JScene.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{ 
			static JModifiedObjectInfoMap infoMap;
			static JModifiedObjectInfoVector infoVec;
		}

		void JEditorObjectHandlerInterface::SetModifiedBit(Core::JUserPtr<Core::JIdentifier> obj, const bool value)noexcept
		{
			if (!obj.IsValid())
				return;

			Core::JUserPtr<Core::JIdentifier> validObj = GetValidModifiedUser(obj);
			if (!validObj.IsValid())
				return;

			const size_t guid = validObj->GetGuid();
			auto data = Private::infoMap.Get(guid);
			if (data == nullptr)
			{
				data = Private::infoMap.Add(std::make_unique<JModifiedObjectInfo>(guid, validObj->GetTypeInfo().Name()), guid);
				Private::infoVec.Add(data);
			}
			data->isModified = value;
		}
		void JEditorObjectHandlerInterface::RemoveModifiedInfo(Core::JUserPtr<Core::JIdentifier> obj)noexcept
		{
			if (!obj.IsValid())
				return;

			Core::JUserPtr<Core::JIdentifier> validObj = GetValidModifiedUser(obj);
			Private::infoVec.Remove(validObj->GetGuid());
			Private::infoMap.Remove(validObj->GetGuid());
		}
		Core::JUserPtr<Core::JIdentifier> JEditorObjectHandlerInterface::GetValidModifiedUser(Core::JUserPtr<Core::JIdentifier> obj)const noexcept
		{			
			/*
			Component -> Gameobject -> Scene 으로 변환
			Component와 GameObject는 Resource가아니므로 Scene에 기록한다
			*/
			if (obj->GetTypeInfo().IsChildOf(JComponent::StaticTypeInfo()))
				return Core::GetUserPtr(static_cast<JComponent*>(obj.Get())->GetOwner()->GetOwnerScene());
			else if (obj->GetTypeInfo().IsChildOf(JGameObject::StaticTypeInfo()))
				return Core::GetUserPtr(static_cast<JGameObject*>(obj.Get())->GetOwnerScene());
			else if (obj->GetTypeInfo().IsChildOf(JResourceObject::StaticTypeInfo()))
				return obj;
			else
				return 	Core::JUserPtr<Core::JIdentifier>{};
		}

		JModifiedObjectInfoVector::ObjectVector& JEditorModifedObjectStructureInterface::GetModifiedObjectInfoVec()noexcept
		{ 
			return Private::infoVec.GetVectorAddress();
		}
		void JEditorModifedObjectStructureInterface::ClearModifiedInfoStructure()noexcept
		{
			Private::infoVec.Clear();
			Private::infoMap.Clear();
		}
	}
}