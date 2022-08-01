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
#include"../Resource/Scene/ISceneGameObjectEvent.h" 
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
	GameObjectDirty* JGameObject::GetGameObjectDirty()noexcept
	{
		return gameObjectDirty.get();
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
		std::string name = GetName();
		bool isOk = false;
		int count = 0;

		while (!isOk)
		{
			bool hasSameName = false;
			for (const auto& data : parent->children)
			{
				if (data->GetName() == name)
				{
					hasSameName = true;
					break;
				}
			}

			if (hasSameName)
			{
				JCommonUtility::ModifyOverlappedName(name, name.length(), count);
				++count;
			}
			else
				isOk = true;
		}
		SetName(name);
		newParent->children.push_back(this);
		transform->ChangeParent(childIndex, newParent->transform);
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
	JComponent* JGameObject::AddComponent(JComponent* component)noexcept
	{
		if (component == nullptr)
			return nullptr;

		const J_COMPONENT_TYPE cType = component->GetComponentType();
		const int overlappedCount = GetComponentCount(component->GetComponentType());
		if (!component->IsAvailableOverlap() && overlappedCount > 0)
			return nullptr;		

		gameObjectDirty->RegisterComponent(component);
		if (IsActivated())
		{
			if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)
				RegisterAnimator(static_cast<JAnimator*>(component));
			else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
				RegisterCamera(static_cast<JCamera*>(component));
			else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
				RegisterLight(static_cast<JLight*>(component));
			else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
				RegisterRenderItem(static_cast<JRenderItem*>(component));
			component->Activate();
		}
		 
		JGameObject::component.push_back(component);
		return component;
	}
	bool JGameObject::EraseComponent(JComponent* component)noexcept
	{
		if (component == nullptr)
			return false;

		const J_COMPONENT_TYPE cType = component->GetComponentType();

		if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)
			DeRegisterAnimator(static_cast<JAnimator*>(component));
		else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			DeRegisterCamera(static_cast<JCamera*>(component));
		else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			DeRegisterLight(static_cast<JLight*>(component));
		else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
			DeRegisterRenderItem(static_cast<JRenderItem*>(component));

		component->DeActivate();
		gameObjectDirty->DeRegisterComponent(component);

		const std::string componentName = component->GetName();
		const size_t componenetGuid = component->GetGuid();
		const J_COMPONENT_TYPE componentType = component->GetComponentType();

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
	bool JGameObject::RegisterAnimator(JAnimator* animator)noexcept
	{
		JGameObject::animator = animator;
		return sceneGameObjectEvent->RegisterAnimator(this, animator);
	}
	bool JGameObject::RegisterCamera(JCamera* camera)noexcept
	{
		return sceneGameObjectEvent->RegisterCamera(this, camera);
	}
	bool JGameObject::RegisterLight(JLight* light)noexcept
	{
		return sceneGameObjectEvent->RegisterLight(this, light);
	}
	bool JGameObject::RegisterRenderItem(JRenderItem* renderItem)noexcept
	{
		JGameObject::renderItem = renderItem;
		return sceneGameObjectEvent->RegisterRenderItem(this, renderItem);
	}
	bool JGameObject::RegisterShadowLight(JLight* light)noexcept
	{
		return sceneGameObjectEvent->RegisterShadowLight(this, light);
	}
	bool JGameObject::DeRegisterAnimator(JAnimator* animator)noexcept
	{
		return sceneGameObjectEvent->DeRegisterAnimator(this, animator);
	}
	bool JGameObject::DeRegisterCamera(JCamera* camera)noexcept
	{
		return sceneGameObjectEvent->DeRegisterCamera(this, camera);
	}
	bool JGameObject::DeRegisterLight(JLight* light)noexcept
	{
		return sceneGameObjectEvent->DeRegisterLight(this, light);
	}
	bool JGameObject::DeRegisterRenderItem(JRenderItem* renderItem)noexcept
	{
		return sceneGameObjectEvent->DeRegisterRenderItem(this, renderItem);
	}
	bool JGameObject::DeRegisterShadowLight(JLight* light)noexcept
	{
		return sceneGameObjectEvent->DeRegisterShadowLight(this, light);
	}
	bool JGameObject::ReRegisterAnimator(JAnimator* animator)noexcept
	{
		if (animator->GetOwnerGuid() != GetGuid())
			return false;

		DeRegisterAnimator(animator);
		return RegisterAnimator(animator);
	}
	bool JGameObject::ReRegisterRenderItem(JRenderItem* renderItem)noexcept
	{
		if (renderItem->GetOwnerGuid() != GetGuid())
			return false;

		DeRegisterRenderItem(renderItem);
		return RegisterRenderItem(renderItem);
	}
	JCamera* JGameObject::SetMainCamera(JCamera* camera)noexcept
	{
		return sceneGameObjectEvent->SetMainCamera(this, camera);
	}
	void JGameObject::UpdateGameObjectTransform()noexcept
	{
		sceneGameObjectEvent->UpdateGameObjectTransform(this);
	}
	void JGameObject::EraseGameObject(JGameObject* gameObject)noexcept
	{
		if (gameObject == nullptr || (gameObject->GetFlag() & JOBJECT_FLAG::OBJECT_FLAG_INERASABLE) > 0)
			return;

		const size_t guid = gameObject->GetGuid();
		JGameObject* parent = gameObject->GetParent();
		const uint childrenCount = (uint)parent->children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			if (guid == parent->children[i]->GetGuid())
			{
				parent->children.erase(parent->children.begin() + i);
				break;
			}
		}
		EraseGameObjectChildren(gameObject);
	}
	void JGameObject::EraseGameObjectChildren(JGameObject* parent)noexcept
	{
		const uint childrenCount = (uint)parent->children.size();
		for (uint i = 0; i < childrenCount; ++i)
			EraseGameObjectChildren(parent->children[i]);

		const uint componentCount = (uint)parent->component.size();
		for (uint i = 0; i < componentCount; ++i)
			parent->component[i]->DeActivate();

		parent->component.clear();
		parent->children.clear();
		parent->sceneGameObjectEvent->EraseGameObject(parent);
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
	void JGameObject::RegisterFunc()
	{
		auto defaultC = [](JGameObject* parent)
		{
			std::string name = GetDefaultName<JGameObject>();
			if (parent != nullptr)
				name = JCommonUtility::MakeUniqueName(parent->children, name);

			JGameObject* newObj = new JGameObject(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
			newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_INERASABLE), *newObj);
			return newObj;
		};
		auto initC = [](const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* parent, JScene* owner)
		{
			std::string newName = name;
			if (parent != nullptr)
				newName = JCommonUtility::MakeUniqueName(parent->children, newName);

			JGameObject* newObj = new JGameObject(name, guid, objFlag, parent, owner);
			newObj->transform = JCFI<JTransform>::Create(Core::MakeGuid(), (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_INERASABLE), *newObj);
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
	} 
	JGameObject::JGameObject(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JGameObject* parent, JScene* ownerScene)
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

		sceneGameObjectEvent = JGameObject::ownerScene;
		gameObjectDirty = std::make_unique<GameObjectDirty>(GetGuid());
	}
	JGameObject::~JGameObject()
	{
		component.clear();
		behavior.clear();
		transform = nullptr;
		animator = nullptr;
		renderItem = nullptr;
		ownerScene = nullptr;
		sceneGameObjectEvent = nullptr;

		children.clear();
		parent = nullptr;
	}
}
 