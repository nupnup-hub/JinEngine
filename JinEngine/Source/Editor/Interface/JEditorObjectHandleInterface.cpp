#include"JEditorObjectHandleInterface.h"
#include"../../Core/Identity/JIdentifier.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Directory/JDirectory.h"
#include"../../Object/Resource/Scene/JScene.h"

namespace JinEngine
{
	namespace Private
	{
		static JModifiedObjectInfoMap infoMap;
		static JModifiedObjectInfoVector infoVec;
	}
	namespace Editor
	{
		void JEditorObjectHandlerInterface::SetModifiedBit(JUserPtr<Core::JIdentifier> obj, const bool value)noexcept
		{
			if (!obj.IsValid())
				return;

			JUserPtr<Core::JIdentifier> validObj = GetValidModifiedUser(obj);
			if (!validObj.IsValid())
				return;
			 
			const size_t guid = validObj->GetGuid();
			auto data = Private::infoMap.Get(guid);
			if (data == nullptr)
			{
				Core::JTypeInfo& typeInfo = validObj->GetTypeInfo();
				data = Private::infoMap.Add(std::make_unique<JModifiedObjectInfo>(guid, typeInfo.TypeGuid(), typeInfo.Name()), guid);
				Private::infoVec.Add(data);
			}
			data->isModified = value; 
		}
		JUserPtr<Core::JIdentifier> JEditorObjectHandlerInterface::GetValidModifiedUser(JUserPtr<Core::JIdentifier> obj)noexcept
		{			
			/*
			Component -> Gameobject -> Scene 으로 변환
			Component와 GameObject는 Resource가아니므로 Scene에 기록한다
			*/
			if (obj->GetTypeInfo().IsChildOf(JComponent::StaticTypeInfo()))
				return static_cast<JComponent*>(obj.Get())->GetOwner()->GetOwnerScene();
			else if (obj->GetTypeInfo().IsChildOf(JGameObject::StaticTypeInfo()))
				return static_cast<JGameObject*>(obj.Get())->GetOwnerScene();
			else if (obj->GetTypeInfo().IsChildOf(JResourceObject::StaticTypeInfo()))
				return obj;
			else if (obj->GetTypeInfo().IsChildOf(JDirectory::StaticTypeInfo()))
				return obj;
			else
				return 	JUserPtr<Core::JIdentifier>{};
		}
	}
	bool JEditorModifedObjectInterface::IsModified(const size_t guid)noexcept
	{
		auto data = Private::infoMap.Get(guid);
		return data != nullptr ? data->isModified : false;
	}
	JModifiedObjectInfoVector::ObjectVector& JEditorModifedObjectInterface::GetModifiedObjectInfoVec()noexcept
	{
		return Private::infoVec.GetVectorAddress();
	}
	void JEditorModifedObjectInterface::ClearModifiedInfoStructure()noexcept
	{
		Private::infoVec.Clear();
		Private::infoMap.Clear();
	}
}

/*
* unuse
* remove bit func
		void JEditorObjectHandlerInterface::SetRemoveBit(JUserPtr<Core::JIdentifier> obj)noexcept
		{
			if (!obj.IsValid())
				return;

			JUserPtr<Core::JIdentifier> validObj = GetValidModifiedUser(obj);
			if (!validObj.IsValid())
				return;

			const size_t guid = validObj->GetGuid();
			auto data = Private::infoMap.Get(guid);
			if (data == nullptr)
			{
				Core::JTypeInfo& typeInfo = validObj->GetTypeInfo();
				data = Private::infoMap.Add(std::make_unique<JModifiedObjectInfo>(guid, typeInfo.TypeGuid(), typeInfo.Name()), guid);
				Private::infoVec.Add(data);
			}
			data->isModified = true;
			data->isRemoved = true;
			data->lastObjName = validObj->GetName();
			if (obj->GetTypeInfo().IsChildOf(JResourceObject::StaticTypeInfo()))
			{
				data->lastObjPath = static_cast<JResourceObject*>(validObj.Get())->GetPath();
				data->lastObjMetaPath = static_cast<JResourceObject*>(validObj.Get())->GetMetaFilePath();
			}
			else if (obj->GetTypeInfo().IsChildOf(JDirectory::StaticTypeInfo()))
			{
				//root dir can't delete
				data->lastObjPath = static_cast<JDirectory*>(validObj.Get())->GetPath();
				data->lastObjMetaPath = static_cast<JDirectory*>(validObj.Get())->GetMetaFilePath();
			}
		}

				void JEditorModifedObjectStructureInterface::DestroyHasRemoveBitInfo()noexcept
		{
			JModifiedObjectInfoVector removeVec;
			uint count = Private::infoVec.Count();
			for (uint i = 0; i < count; ++i)
			{
				auto info = Private::infoVec.Get(i);
				if (info->isRemoved && info->isStore)
					removeVec.Add(info);
			}

			auto equalLam = [](JModifiedObjectInfo* a, JModifiedObjectInfo* b) {return a->objectGuid == b->objectGuid; };
			JModifiedObjectInfoVector::EqualPtr equalPtr = equalLam;
			count = removeVec.Count();
			for (uint i = 0; i < count; ++i)
			{
				auto info = removeVec.Get(i);
				Private::infoVec.Remove(info, equalLam);
				Private::infoMap.Remove(info->objectGuid);
			}
		}
*/