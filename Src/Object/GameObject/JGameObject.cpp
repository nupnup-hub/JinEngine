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
	uint JGameObject::GetComponentCount(const J_COMPONENT_TYPE type)noexcept
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
	bool JGameObject::HasComponent(const J_COMPONENT_TYPE type)noexcept
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
	bool JGameObject::HasSameName(_In_ JGameObject* parent, _In_ const std::string& initName) noexcept
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
		const J_COMPONENT_TYPE cType = component.GetComponentType();
		const int overlappedCount = GetComponentCount(component.GetComponentType());
		if (!component.IsAvailableOverlap() && overlappedCount > 0)
			return nullptr;		
	 
		if (IsActivated())
			component.Activate();		
		 
		JGameObject::component.push_back(&component);
		return &component;
	}
	bool JGameObject::RemoveComponent(JComponent& component)noexcept
	{
		const J_COMPONENT_TYPE cType = component.GetComponentType();		
		const std::string componentName = component.GetName();
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
		JGameObject::component.erase(JGameObject::component.begin() + index); 
		return true;
	}
	void JGameObject::Destroy()
	{
		if(HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !IsIgnoreUndestroyableFlag())
			return;

		DeActivate();

		const size_t guid = GetGuid();
		const uint pChildrenCount = (uint)parent->children.size();
		for (uint i = 0; i < pChildrenCount; ++i)
		{
			if (guid == parent->children[i]->GetGuid())
			{
				parent->children.erase(parent->children.begin() + i);
				break;
			}
		}

		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			children[i]->Destroy();
			children[i] = nullptr;
		}

		bool preIgnore = IsIgnoreUndestroyableFlag();
		if(!preIgnore)
			SetIgnoreUndestroyableFlag(true);
		const uint componentCount = (uint)component.size();
		for (uint i = 0; i < componentCount; ++i)
		{
			component[i]->BeginDestroy();
			component[i] = nullptr;
		}
		if(!preIgnore)
			SetIgnoreUndestroyableFlag(false);

		component.clear();
		children.clear();
		 
		ownerScene->GameObjInterface()->RemoveGameObject(*parent);

		delete this;
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

		stream << gameObject->GetWName() << '\n';
		const uint componentCount = (uint)gameObject->component.size();
		stream << componentCount << '\n';
		for (uint i = 0; i < componentCount; ++i)
		{
			stream << JCommonUtility::U8StringToWstring(gameObject->component[i]->GetTypeInfo().Name()) << '\n';
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
			newGameObject = new JGameObject(JCommonUtility::WstringToU8String(name), metadata.guid, metadata.flag, parent);
		else
			newGameObject = new JGameObject(JCommonUtility::WstringToU8String(name), Core::MakeGuid(), OBJECT_FLAG_NONE, parent);

		int componentCount;
		stream >> componentCount;
		newGameObject->component.resize(componentCount);

		for (int i = 0; i < componentCount; ++i)
		{
			std::wstring componentName;
			stream >> componentName;
			newGameObject->component[i] = JCFI<JComponent>::LoadByName(JCommonUtility::WstringToU8String(componentName), stream, *newGameObject);

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
			std::string name = GetDefaultName<JGameObject>();
			if (parent != nullptr)
				name = JCommonUtility::MakeUniqueName(parent->children, name);

			JGameObject* newObj = new JGameObject(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
			newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE), *newObj);
			return newObj;
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* parent, JScene* owner)
		{
			std::string newName = name;
			if (parent != nullptr)
				newName = JCommonUtility::MakeUniqueName(parent->children, newName);

			JGameObject* newObj = new JGameObject(name, guid, objFlag, parent, owner);
			newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE), *newObj);
			return newObj;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* parent)
		{
			return LoadObject(stream, parent);
		};

		auto copyC = [](JGameObject* oriObj, JGameObject* parent)
		{  
			JGameObject* newObj = new JGameObject(JCommonUtility::MakeUniqueName(parent->children, oriObj->GetName()), 
				Core::MakeGuid(), oriObj->GetFlag(), parent, parent->ownerScene);

			const uint componentCount = oriObj->GetComponentCount(); 
			newObj->component.resize(componentCount);
			for (uint i = 0; i < componentCount; ++i)
				newObj->component[i] = JCFIB::CopyByName(oriObj->children[i]->TypeName(), *oriObj->component[i], *newObj);

			const uint childrenCount = oriObj->GetChildrenCount();		
			newObj->children.resize(childrenCount);
			for (uint i = 0; i < childrenCount; ++i)
				newObj->children[i] = JGFI::Copy(*oriObj->children[i], *newObj);

			return newObj;
		};		
		JGFI::Regist(defaultC, initC, loadC, copyC);
		JComponentFactoryImplBase::RegisterAddStroage(&JGameObject::AddComponent);		
	} 
	JGameObject::JGameObject(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* parent, JScene* ownerScene)
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
 