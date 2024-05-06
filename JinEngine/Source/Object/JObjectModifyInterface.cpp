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


#include"JObjectModifyInterface.h"
#include"Component/JComponent.h"
#include"GameObject/JGameObject.h"
#include"Directory/JDirectory.h"
#include"Resource/Scene/JScene.h"
#include"../Core/Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Private
	{
		static JModifiedObjectInfoMap infoMap;
		static JModifiedObjectInfoVector infoVec;
	}
	void JObjectModifyInterface::SetModifiedBit(JUserPtr<Core::JIdentifier> obj, const bool value)noexcept
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
	JUserPtr<Core::JIdentifier> JObjectModifyInterface::GetValidModifiedUser(JUserPtr<Core::JIdentifier> obj)noexcept
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
	bool JModifedObjectInterface::IsModified(const size_t guid)const noexcept
	{
		auto data = Private::infoMap.Get(guid);
		return data != nullptr ? data->isModified : false;
	}
	bool JModifedObjectInterface::IsModifiedAndStoreAble(const size_t guid)const noexcept
	{
		return IsModifiedAndStoreAble(Private::infoMap.Get(guid));
	}
	bool JModifedObjectInterface::IsModifiedAndStoreAble(const JModifiedObjectInfo* info)const noexcept
	{
		return info != nullptr ? info->canStore && info->isModified : false;
	}
	JModifiedObjectInfoVector::ObjectVector& JModifedObjectInterface::GetModifiedObjectInfoVec()const noexcept
	{
		return Private::infoVec.GetVectorAddress();
	}
	void JModifedObjectInterface::RemoveInfo(const size_t guid)
	{
		JModifiedObjectInfoVector::EqualPtr eqaulPtr = [](JModifiedObjectInfo* a, JModifiedObjectInfo* b) {return a->objectGuid == b->objectGuid; };
		JModifiedObjectInfo* exist = Private::infoMap.Get(guid);
		if (exist == nullptr)
			return;

		Private::infoVec.Remove(Private::infoVec.GetIndex(exist, eqaulPtr));
		Private::infoMap.Remove(guid);
	}
	void JModifedObjectInterface::ClearModifiedInfoStructure()noexcept
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