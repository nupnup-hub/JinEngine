#include"JGameObject.h"    
#include"JGameObjectFactory.h"  
#include"../Component/Animator/JAnimator.h"  
#include"../Component/Behavior/JBehavior.h"  
#include"../Component/Camera/JCamera.h"  
#include"../Component/Light/JLight.h"  
#include"../Component/RenderItem/JRenderItem.h"  
#include"../Component/Transform/JTransform.h"   
#include"../Component/JComponentFactory.h" 
#include"../Resource/Scene/JScene.h" 
#include"../../Utility/JCommonUtility.h"  
#include"../../Core/Guid/GuidCreator.h"
#include<fstream>

namespace JinEngine
{
	JAnimator* JGameObject::GetAnimator()noexcept
	{
		return animator;
	}
	JTransform* JGameObject::GetTransform() noexcept
	{
		return transform;
	}
	JRenderItem* JGameObject::GetRenderItem()noexcept
	{
		return renderItem;
	}
	JScene* JGameObject::GetOwnerScene()noexcept
	{
		return ownerScene;
	}
	uint JGameObject::GetChildrenCount()const noexcept
	{
		return (uint)children.size();
	}
	uint JGameObject::GetComponentCount()const noexcept
	{
		return (uint)component.size();
	}
	uint JGameObject::GetComponentCount(const J_COMPONENT_TYPE type)const noexcept
	{
		uint res = 0;
		const uint componentCount = (uint)component.size();
		for (uint i = 0; i < componentCount; ++i)
		{
			if (component[i]->GetComponentType() == type)
				++res;
		}
		return res;
	}
	JGameObject* JGameObject::GetParent()noexcept
	{
		return parent;
	}
	JGameObject* JGameObject::GetChild(const uint index)noexcept
	{
		if (index < children.size())
			return children[index];
		else
			return nullptr;
	}
	J_OBJECT_TYPE JGameObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::GAME_OBJECT;
	}
	JComponent* JGameObject::FindComponent(const size_t guid)const noexcept
	{
		const uint compCount = (uint)component.size();
		for (uint i = 0; i < compCount; ++i)
		{
			if (component[i]->GetGuid() == guid)
				return component[i];
		}
		 
		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
			return children[i]->FindComponent(guid);

		return nullptr;
	}
	bool JGameObject::IsRoot()const noexcept
	{
		return parent == nullptr;
	}
	bool JGameObject::HasComponent(const J_COMPONENT_TYPE type)const noexcept
	{
		const uint componentCount = (uint)component.size();
		for (uint i = 0; i < componentCount; ++i)
		{
			if (component[i]->GetComponentType() == type)
				return true;
		}
		return false;
	}
	bool JGameObject::HasRenderItem()const noexcept
	{
		return (renderItem != nullptr);
	}
	bool JGameObject::HasAnimator()const noexcept
	{
		return (animator != nullptr);
	}
	bool JGameObject::CanAddComponent(const J_COMPONENT_TYPE type)const noexcept
	{ 
		if (!JCI::CallIsAvailableOverlap(type) && GetComponentCount(type) > 0)
			return false;
		else
			return true;
	}
	void JGameObject::ChangeParent(JGameObject* newParent)noexcept
	{
		if ((newParent == nullptr || parent == nullptr) ||
			(newParent->ownerScene->GetGuid() != parent->ownerScene->GetGuid()))
			return;

		if (IsChild(newParent))
			return;

		int childIndex = -1;
		const size_t guid = GetGuid();
		for (int i = 0; i < parent->children.size(); ++i)
		{
			if (parent->children[i]->GetGuid() == guid)
			{
				childIndex = i;
				break;
			}
		}

		if (childIndex == -1)
			return;

		parent->children.erase(parent->children.begin() + childIndex);
		parent = newParent;
		SetName(JCommonUtility::MakeUniqueName(parent->children, GetName()));
		newParent->children.push_back(this);
		static_cast<JTransformInterface*>(transform)->ChangeParent(); 
	}
	JGameObjectCompInterface* JGameObject::CompInterface()
	{
		return this;
	}
	bool JGameObject::Copy(JObject* ori)
	{
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;

		if (typeInfo.IsA(ori->GetTypeInfo()))
		{
			bool preIsActivated = IsActivated();
			Clear();
			if (preIsActivated)
				Activate();

			JGameObject* oriGobj = static_cast<JGameObject*>(ori);
			const uint componentCount = (uint)oriGobj->component.size();
			for (uint i = 0; i < componentCount; ++i)
				JCFIB::CopyByName(*oriGobj->component[i], *this);
				 
			const uint childrenCount = (uint)oriGobj->children.size();
			for (uint i = 0; i < childrenCount; ++i)
				JGFI::Copy(*oriGobj->children[i], *this);
			return true;
		}
		else
			return false;
	}
	void JGameObject::DoActivate()noexcept
	{
		if (ownerScene != nullptr)
		{
			JObject::DoActivate();
			const uint componentCount = (uint)component.size();
			for (uint i = 0; i < componentCount; ++i)
			{
				if (!component[i]->IsActivated())
					component[i]->Activate();
			}
		}
	}
	void JGameObject::DoDeActivate()noexcept
	{ 
		JObject::DoDeActivate();
		const uint componentCount = (uint)component.size();
		for (uint i = 0; i < componentCount; ++i)
		{
			if (component[i]->IsActivated())
				component[i]->DeActivate();
		}
	}
	bool JGameObject::HasSameName(_In_ JGameObject* parent, _In_ const std::wstring& initName) noexcept
	{
		const uint childrenSize = parent->GetChildrenCount();
		for (uint i = 0; i < childrenSize; ++i)
		{
			if (parent->GetChild(i)->GetName() == initName)
				return true;
		}
		return false;
	}
	bool JGameObject::IsChild(JGameObject* obj)noexcept
	{
		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			if (children[i]->GetGuid() == obj->GetGuid())
				return true;

			bool res = children[i]->IsChild(obj);

			if (res)
				return true;
		}
		return false;
	}
	JComponent* JGameObject::AddComponent(JComponent& component)noexcept
	{
		if (!CanAddComponent(component.GetComponentType()))
			return nullptr;		
	 
		if (IsActivated())
			component.Activate();	

		if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)
			animator = static_cast<JAnimator*>(&component);
		else if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
			renderItem = static_cast<JRenderItem*>(&component);

		JGameObject::component.push_back(&component);
		return &component;
	}
	bool JGameObject::RemoveComponent(JComponent& component)noexcept
	{
		const J_COMPONENT_TYPE cType = component.GetComponentType();		
		const size_t componenetGuid = component.GetGuid();
		const J_COMPONENT_TYPE componentType = component.GetComponentType();

		int index = -1;
		const uint componentCount = (uint)JGameObject::component.size();
		for (uint i = 0; i < componentCount; ++i)
		{
			if (componenetGuid == JGameObject::component[i]->GetGuid())
			{
				index = i;
				break;
			}
		}
		if (index == -1)
			return false;

		if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)
			animator = nullptr;
		else if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
			renderItem = nullptr;
		else if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM)
			transform = nullptr;

		JGameObject::component.erase(JGameObject::component.begin() + index); 
		return true;
	}
	void JGameObject::Destroy()
	{
		if(HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !IsIgnoreUndestroyableFlag())
			return;

		Clear();
		ownerScene->GameObjInterface()->RemoveGameObject(*parent);
		ownerScene = nullptr;
	}
	void JGameObject::Clear()
	{
		DeActivate();
		if (parent != nullptr)
		{
			const size_t guid = GetGuid();
			const uint pChildrenCount = (uint)parent->children.size();
			for (uint i = 0; i < pChildrenCount; ++i)
			{
				if (guid == parent->children[i]->GetGuid())
				{
					parent->children.erase(parent->children.begin() + i);
					parent = nullptr;
					break;
				}
			}
		}

		bool preIgnore = IsIgnoreUndestroyableFlag();
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(true);
		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			children[i]->BeginDestroy();
			children[i] = nullptr;
		}
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(false);

		if (!preIgnore)
			SetIgnoreUndestroyableFlag(true);
		const uint componentCount = (uint)component.size();
		for (uint i = 0; i < componentCount; ++i)
		{
			component[i]->BeginDestroy();
			component[i] = nullptr;
		}
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(false);

		component.clear();
		children.clear();
		behavior.clear();
		transform = nullptr;
		animator = nullptr;
		renderItem = nullptr; 
	}
	Core::J_FILE_IO_RESULT JGameObject::CallStoreGameObject(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JGameObject::StoreObject(std::wofstream& stream, JGameObject* gameObject)
	{
		if (gameObject == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)gameObject->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, gameObject);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		stream << gameObject->GetName() << '\n';
		const uint componentCount = (uint)gameObject->component.size();
		stream << componentCount << '\n';
		for (uint i = 0; i < componentCount; ++i)
		{
			stream << JCommonUtility::U8StrToWstr(gameObject->component[i]->GetTypeInfo().Name()) << '\n';
			JComponentInterface* cI = gameObject->component[i];
			cI->CallStoreComponent(stream);
		}

		stream << gameObject->children.size() << '\n';
		const uint childrenCount = (uint)gameObject->children.size();
		for (uint i = 0; i < childrenCount; ++i)
			JGameObject::StoreObject(stream, gameObject->children[i]);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JGameObject* JGameObject::LoadObject(std::wifstream& stream, JGameObject* parent)
	{
		if (parent == nullptr)
			return nullptr;

		if (!stream.is_open())
			return nullptr;

		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);

		std::wstring name;
		stream >> name;

		JGameObject* newGameObject;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(JCommonUtility::WstrToU8Str(name),
				metadata.guid,
				metadata.flag,
				parent);
			newGameObject = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
		}
		else
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(JCommonUtility::WstrToU8Str(name), 
				Core::MakeGuid(), 
				OBJECT_FLAG_NONE,
				parent);
			newGameObject = ownerPtr.Get();
			AddInstance(std::move(ownerPtr)); 
		}

		int componentCount;
		stream >> componentCount;
		newGameObject->component.resize(componentCount);

		for (int i = 0; i < componentCount; ++i)
		{
			std::wstring componentName;
			stream >> componentName;
			newGameObject->component[i] = JCFI<JComponent>::LoadByName(JCommonUtility::WstrToU8Str(componentName), stream, *newGameObject);

			if (newGameObject->component[i] == nullptr)
				return nullptr;
		}

		int childrenCount;
		stream >> childrenCount;
		newGameObject->children.resize(childrenCount);
		for (int i = 0; i < childrenCount; ++i)
		{
			newGameObject->children[i] = JGFI::Create(stream, newGameObject);
			if (newGameObject->children[i] == nullptr)
				return nullptr;
		}
		return newGameObject;
	}
	void JGameObject::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* parent)
		{
			std::wstring name = GetDefaultName<JGameObject>();
			if (parent != nullptr)
				name = JCommonUtility::MakeUniqueName(parent->children, name);
			 
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
			JGameObject* newObj = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));

			newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE), *newObj);
			return newObj;
		};
		auto initC = [](const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* parent, JScene* owner)
		{
			std::wstring newName = name;
			if (parent != nullptr)
				newName = JCommonUtility::MakeUniqueName(parent->children, newName);

			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(name, guid, objFlag, parent, owner);
			JGameObject* newObj = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));

			newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE), *newObj);
			return newObj;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* parent)
		{
			return LoadObject(stream, parent);
		};
		auto copyC = [](JGameObject* oriObj, JGameObject* parent)
		{  
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(JCommonUtility::MakeUniqueName(parent->children, oriObj->GetName()),
				Core::MakeGuid(), oriObj->GetFlag(), parent, parent->ownerScene);
			JGameObject* newObj = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			newObj->Copy(newObj);
			return newObj;
		};		
		JGFI::Regist(defaultC, initC, loadC, copyC);
		JComponentFactoryImplBase::RegisterAddStroage(&JGameObject::AddComponent);		
	} 
	JGameObject::JGameObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* parent, JScene* ownerScene)
		:JGameObjectInterface(name, guid, flag), parent(parent)
	{
		//parent == nullptr = root
		if (parent != nullptr)
		{
			JGameObject::ownerScene = parent->ownerScene;
			parent->children.push_back(this);
		}
		else
			JGameObject::ownerScene = ownerScene;
	}
	JGameObject::~JGameObject()
	{
		component.clear();
		behavior.clear();
		transform = nullptr;
		animator = nullptr;
		renderItem = nullptr;
		ownerScene = nullptr; 

		children.clear();
		parent = nullptr;
	}
}
 