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
#include"../../Core/File/JFileIOHelper.h"
#include"../../Utility/JCommonUtility.h"  
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/File/JFileConstant.h"
#include<fstream>

namespace JinEngine
{
	JAnimator* JGameObject::GetAnimator()const noexcept
	{ 
		return animator;
	}
	JTransform* JGameObject::GetTransform()const noexcept
	{
		return transform;
	}
	JRenderItem* JGameObject::GetRenderItem()const noexcept
	{
		return renderItem;
	}
	std::vector<JComponent*> JGameObject::GetComponentVec()const noexcept
	{
		return component;
	}
	JScene* JGameObject::GetOwnerScene()const noexcept
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
	JGameObject* JGameObject::GetParent()const noexcept
	{
		return parent;
	}
	JGameObject* JGameObject::GetChild(const uint index)const noexcept
	{
		if (index < children.size())
			return children[index];
		else
			return nullptr;
	}
	std::vector<JGameObject*> JGameObject::GetChildren()const noexcept
	{
		return children;
	}
	J_OBJECT_TYPE JGameObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::GAME_OBJECT;
	}
	void JGameObject::SetName(const std::wstring& newName)noexcept
	{
		if (parent != nullptr)
			JObject::SetName(JCUtil::MakeUniqueName(parent->children, newName));
	}
	bool JGameObject::IsRoot()const noexcept
	{
		return parent == nullptr;
	}
	bool JGameObject::IsParentLine(JGameObject* child)const noexcept
	{
		bool isParent = false;
		JGameObject* childParent = child->parent;
		while (childParent != nullptr)
		{
			if (childParent->GetGuid() == GetGuid())
			{
				isParent = true;
				break;
			}
			childParent = childParent->parent;
		}
		return isParent;
	}
	bool JGameObject::IsSelectedbyEditor()const noexcept
	{
		return isSelectedbyEditor;
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
		SetName(JCUtil::MakeUniqueName(parent->children, GetName()));
		newParent->children.push_back(this); 
		static_cast<JTransformGameObjectInterface*>(transform)->ChangeParent();
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
	JGameObjectCompInterface* JGameObject::CompInterface()
	{
		return this;
	}
	JGameObjectEditorInterface* JGameObject::EditorInterface()
	{
		return this;
	}
	void JGameObject::DoCopy(JObject* ori)
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
	void JGameObject::SetSelectedByEditorTrigger(const bool value)noexcept
	{
		isSelectedbyEditor = value;
	}
	bool JGameObject::AddComponent(JComponent& component)noexcept
	{
		if (!CanAddComponent(component.GetComponentType()))
			return false;

		if (IsActivated())
			component.Activate();

		if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)
			animator = static_cast<JAnimator*>(&component);
		else if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
			renderItem = static_cast<JRenderItem*>(&component);
		else if (component.GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM)
			transform = static_cast<JTransform*>(&component);

		JGameObject::component.push_back(&component);
		return true;
	}
	bool JGameObject::RemoveComponent(JComponent& component)noexcept
	{
		if (component.IsActivated())
			component.DeActivate();

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
	bool JGameObject::Destroy(const bool isForced)
	{
		if (HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !isForced)
			return false;
		 
		Clear();	 
		return true;
	}
	void JGameObject::Clear()
	{
		DeActivate();

		std::vector<JGameObject*> gCopy = children;
		const uint childrenCount = (uint)gCopy.size();
		for (uint i = 0; i < childrenCount; ++i)
			JObject::BegineForcedDestroy(gCopy[i]);

		std::vector<JComponent*> cCopy = component;
		const uint componentCount = (uint)cCopy.size();
		for (uint i = 0; i < componentCount; ++i)
			JObject::BegineForcedDestroy(cCopy[i]);

		component.clear();
		children.clear();
		behavior.clear();
		transform = nullptr;
		animator = nullptr;
		renderItem = nullptr;
	}
	bool JGameObject::RegisterCashData()noexcept
	{
		if(parent != nullptr)
			parent->children.push_back(this);
		return ownerScene->GameObjInterface()->AddGameObject(*this);
	}
	bool JGameObject::DeRegisterCashData()noexcept
	{ 
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
		return ownerScene->GameObjInterface()->RemoveGameObject(*this);
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

		JFileIOHelper::StoreObjectIden(stream, gameObject);
		JFileIOHelper::StoreAtomicData(stream, L"ComponentCount:", gameObject->component.size());	 

		for (uint i = 0; i < gameObject->component.size(); ++i)
		{
			JFileIOHelper::StoreJString(stream, L"TypeName:", JCUtil::U8StrToWstr(gameObject->component[i]->GetTypeInfo().Name()));
			JComponentInterface* cI = gameObject->component[i];
			cI->CallStoreComponent(stream);
		}

		JFileIOHelper::StoreAtomicData(stream, L"ChildCount:", gameObject->children.size());
		for (uint i = 0; i < gameObject->children.size(); ++i)
			JGameObject::StoreObject(stream, gameObject->children[i]);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JGameObject* JGameObject::LoadObject(std::wifstream& stream, JGameObject* parent, JScene* owenerScene)
	{
		if (!stream.is_open())
			return nullptr;
		 
		std::wstring name;
		size_t guid;
		J_OBJECT_FLAG flag;
  
		JFileIOHelper::LoadObjectIden(stream, name, guid, flag);

		Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(name, guid, flag, parent, owenerScene);
		JGameObject* newGameObject = ownerPtr.Get();
		
		if (!AddInstance(std::move(ownerPtr)))
			return nullptr;

		int componentCount;
		JFileIOHelper::LoadAtomicData(stream, componentCount);
	  
		for (int i = 0; i < componentCount; ++i)
		{
			std::wstring componentName;
			JFileIOHelper::LoadJString(stream, componentName); 
			JCFI<JComponent>::LoadByName(JCUtil::WstrToU8Str(componentName), stream, *newGameObject);
		}

		int childrenCount;
		JFileIOHelper::LoadAtomicData(stream, childrenCount);
		for (int i = 0; i < childrenCount; ++i)
			JGFI::Create(stream, newGameObject, owenerScene);

		return newGameObject;
	}
	void JGameObject::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* parent) -> JGameObject*
		{
			std::wstring name = GetDefaultName<JGameObject>();
			if (parent != nullptr)
				name = JCUtil::MakeUniqueName(parent->children, name);

			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
			JGameObject* newObj = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE), *newObj);
				return newObj;
			}
			else
				return nullptr;
		};
		auto initC = [](const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* parent, JScene* owner)
			-> JGameObject*
		{
			std::wstring newName = name;
			if (parent != nullptr)
				newName = JCUtil::MakeUniqueName(parent->children, newName);

			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(newName, guid, objFlag, parent, owner);
			JGameObject* newObj = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE), *newObj);
				return newObj;
			}
			else
				return nullptr;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* parent, JScene* ownerScene) -> JGameObject*
		{
			return LoadObject(stream, parent, ownerScene);
		};
		auto copyC = [](JGameObject* oriObj, JGameObject* parent) -> JGameObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JGameObject>(JCUtil::MakeUniqueName(parent->children, oriObj->GetName()),
				Core::MakeGuid(), oriObj->GetFlag(), parent, parent->ownerScene);
			JGameObject* newObj = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newObj->Copy(newObj);
				return newObj;
			}
			else
				return nullptr;
		};
		JGFI::Register(defaultC, initC, loadC, copyC);
	}
	JGameObject::JGameObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* parent, JScene* ownerScene)
		:JGameObjectInterface(name, guid, flag), parent(parent)
	{
		//parent == nullptr = root
		if (parent != nullptr)
			JGameObject::ownerScene = parent->ownerScene;
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
