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


#include"JGameObject.h"    
#include"JGameObjectPrivate.h"  
#include"../Component/JComponent.h"    
#include"../Component/JComponentHint.h"
#include"../Component/Animator/JAnimator.h" 
#include"../Component/Behavior/JBehavior.h" 
#include"../Component/RenderItem/JRenderItem.h" 
#include"../Component/Transform/JTransform.h"
#include"../Component/Transform/JTransformPrivate.h"
#include"../Resource/Scene/JScenePrivate.h"
#include"../Resource/Scene/JScene.h" 
#include"../JObjectFileIOHelper.h"
#include"../../Core/Identity/JIdenCreator.h" 
#include"../../Core/Reflection/JTypeImplBase.h" 
#include"../../Core/File/JFileConstant.h"
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Empty/JEmptyType.h"
#include"../../Core/Utility/JCommonUtility.h"  
#include<fstream>

namespace JinEngine
{
	namespace
	{
		using SearchCompByTypePtr = bool(*)(const JUserPtr<JComponent>&, const J_COMPONENT_TYPE);
	}
	namespace
	{
		static JGameObjectPrivate gPrivate;
		static SearchCompByTypePtr searchPtr = nullptr;
	}

	class JGameObject::JGameObjectImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JGameObjectImpl)
	public:
		JWeakPtr<JGameObject> thisPointer;
	public:
		std::vector<JUserPtr<JGameObject>> children;
		std::vector<JUserPtr<JComponent>> componentVec;
		std::vector<JUserPtr<JBehavior>> behavior;
		JUserPtr<JGameObject> parent = nullptr;
		JUserPtr<JTransform> transform = nullptr;
		JUserPtr<JAnimator> animator = nullptr;
		JUserPtr<JRenderItem> renderItem = nullptr;
		JUserPtr<JScene> ownerScene = nullptr;
	public:
		//For Editor
		std::unique_ptr<Core::JTypeInstanceSearchHint> parentInfo;			//For editor redo undo
		bool isSelected = false; 
	public:
		JGameObjectImpl(const InitData& initData)
			:parent(initData.parent)
		{}
		~JGameObjectImpl()
		{}
	public:
		void Initialize(const InitData& initData, JGameObject* thisGameObj)
		{
			//parent == nullptr = root
			if (parent != nullptr)
				ownerScene = parent->impl->ownerScene;
			else
				ownerScene = initData.ownerScene;
		}
	public:
		JUserPtr<JComponent> GetComponent(const J_COMPONENT_TYPE compType)const noexcept
		{
			int index = JCUtil::GetIndex(componentVec, searchPtr, compType);
			return index != -1 ? componentVec[index] : nullptr;
		}
		JUserPtr<JComponent> GetComponentWithParent(const J_COMPONENT_TYPE compType)const noexcept
		{
			JUserPtr<JComponent> res = GetComponent(compType);
			if (res == nullptr && parent != nullptr)
				res = parent->impl->GetComponentWithParent(compType);
			return res;
		}
		JUserPtr<JComponent> GetComponentWithChild(const J_COMPONENT_TYPE compType)const noexcept
		{
			JUserPtr<JComponent> res = GetComponent(compType);
			if (res == nullptr)
			{
				for (const auto& child : children)
				{
					res = child->impl->GetComponentWithChild(compType);
					if (res != nullptr)
						break;
				}
			}
			return res;
		}
		void GetComponents(const J_COMPONENT_TYPE compType, std::vector<JUserPtr<JComponent>>& result)const noexcept
		{
			for (const auto& data : componentVec)
			{ 
				if (data->GetComponentType() == compType)
					result.push_back(data);
			}
		}
		void GetComponentsWithParent(const J_COMPONENT_TYPE compType, std::vector<JUserPtr<JComponent>>& res)const noexcept
		{
			GetComponents(compType, res);
			if (parent != nullptr)
				parent->impl->GetComponentsWithParent(compType, res);
		}
		void GetComponentsWithChild(const J_COMPONENT_TYPE compType, std::vector<JUserPtr<JComponent>>& res)const noexcept
		{
			GetComponents(compType, res);
			for (const auto& child : children)
				child->impl->GetComponentsWithChild(compType, res);
		}
		uint GetComponentCount(const J_COMPONENT_TYPE type)const noexcept
		{
			uint res = 0;
			const uint componentCount = (uint)componentVec.size();
			for (uint i = 0; i < componentCount; ++i)
			{
				if (componentVec[i]->GetComponentType() == type)
					++res;
			}
			return res;
		}
	public:
		bool IsChild(JGameObject* gPtr)noexcept
		{
			const uint childrenCount = (uint)children.size();
			for (uint i = 0; i < childrenCount; ++i)
			{
				if (children[i]->GetGuid() == gPtr->GetGuid())
					return true;

				bool res = children[i]->impl->IsChild(gPtr);

				if (res)
					return true;
			}
			return false;
		}
		static bool HasSameName(_In_ JGameObject* parent, _In_ const std::wstring& initName) noexcept
		{
			const uint childrenSize = parent->GetChildrenCount();
			for (uint i = 0; i < childrenSize; ++i)
			{
				if (parent->GetChild(i)->GetName() == initName)
					return true;
			}
			return false;
		}
		bool HasComponent(const J_COMPONENT_TYPE type)const noexcept
		{
			const uint componentCount = (uint)componentVec.size();
			for (uint i = 0; i < componentCount; ++i)
			{
				if (componentVec[i]->GetComponentType() == type)
					return true;
			}
			return false;
		}
		bool CanAddComponent(const J_COMPONENT_TYPE type)const noexcept
		{
			if (!CTypeCommonCall::CallIsAvailableOverlap(type) && GetComponentCount(type) > 0)
				return false;
			else
				return true;
		}
	public:
		JUserPtr<JComponent> FindComponent(const size_t guid)const noexcept
		{
			int index = JCUtil::GetTypeIndex(componentVec, guid);
			if (index != -1)
				return componentVec[index];

			const uint childrenCount = (uint)children.size();
			for (uint i = 0; i < childrenCount; ++i)
				return children[i]->FindComponent(guid);

			return nullptr;
		}
	public:
		void Select()noexcept
		{
			isSelected = true;
		}
		void DeSelect()noexcept
		{
			isSelected = false;
		}
	public:
		static bool DoCopy(JUserPtr<JGameObject> from, JUserPtr<JGameObject> to)
		{
			bool preIsActivated = to->IsActivated();
			to->impl->Clear(to.Get());
			if (preIsActivated)
				to->Activate();
			 
			const uint componentCount = (uint)from->impl->componentVec.size();
			for (uint i = 0; i < componentCount; ++i)
			{
				auto initData = JComponent::CreateInitDIData(from->impl->componentVec[i]->GetComponentType(), from->GetTypeInfo(), to);
				JICI::CreateAndCopy(std::move(initData), from->impl->componentVec[i]);
			}

			const uint childrenCount = (uint)from->impl->children.size();
			for (uint i = 0; i < childrenCount; ++i)
			{
				auto initData = std::make_unique<JGameObject::InitData>(from->impl->children[i]->GetName(), Core::MakeGuid(), OBJECT_FLAG_NONE, to);
				JICI::CreateAndCopy(std::move(initData), from->impl->children[i]);
			}
			return true;
		}
	public:
		static std::unique_ptr<JGameObject::InitData> CreateInitData(JUserPtr<JGameObject> parent)
		{
			return std::make_unique<JGameObject::InitData>(Core::JIdentifier::GetDefaultName(JGameObject::StaticTypeInfo()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				parent,
				parent->GetOwnerScene());
		}
		static std::unique_ptr<JGameObject::InitData> CreateInitData(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, LoadData* loadData)
		{
			auto initData = std::make_unique<JGameObject::InitData>(name, guid, flag, loadData->parent, loadData->ownerScene);
			initData->makeTransform = false;
			return std::move(initData);
		}
	public:
		void Clear(JGameObject* gPtr)
		{ 
			if (gPtr->IsActivated())
				gPtr->DeActivate();

			std::vector<JUserPtr<JGameObject>> gCopy = children;
			const uint childrenCount = (uint)gCopy.size();
			for (uint i = 0; i < childrenCount; ++i)
				Core::JIdentifier::BeginForcedDestroy(gCopy[i].Get());

			std::vector<JUserPtr<JComponent>> cCopy = componentVec;
			const uint componentCount = (uint)cCopy.size();
			for (uint i = 0; i < componentCount; ++i)
				Core::JIdentifier::BeginForcedDestroy(cCopy[i].Get());

			componentVec.clear();
			children.clear();
			behavior.clear();
			transform = nullptr;
			animator = nullptr;
			renderItem = nullptr;
		}
	public:
		bool AddComponent(const JUserPtr<JComponent>& newComp)noexcept
		{
			if (!CanAddComponent(newComp->GetComponentType()))
				return false;
			  
			componentVec.push_back(newComp);
			if (newComp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)
				animator.ConnnectChild(newComp);
			else if (newComp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
				renderItem.ConnnectChild(newComp);
			else if (newComp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM)
				transform.ConnnectChild(newComp); 
			return true;
		}
		bool RemoveComponent(const JUserPtr<JComponent>& comp)noexcept
		{
			JUserPtr<JGameObject> gPtr = comp->GetOwner();
			const J_COMPONENT_TYPE cType = comp->GetComponentType();
			const size_t componenetGuid = comp->GetGuid();
			const J_COMPONENT_TYPE componentType = comp->GetComponentType();

			int index = JCUtil::GetTypeIndex(componentVec, comp->GetGuid());
			if (index == -1)
				return false;

			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)
				animator = nullptr;
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)
				renderItem = nullptr;
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM)
				transform = nullptr;

			componentVec.erase(componentVec.begin() + index);
			return true;
		}
	public:
		bool RegisterInstance()noexcept
		{
			if (parent != nullptr)
				parent->impl->children.push_back(thisPointer);
			else if (parentInfo != nullptr)
			{
				auto pIden = Core::GetRawPtr(*parentInfo);
				parent = pIden != nullptr ? Core::GetUserPtr<JGameObject>(pIden) : nullptr;
				if (parent != nullptr)
					parent->impl->children.push_back(thisPointer);
			}
			return JScenePrivate::OwnTypeInterface::AddGameObject(thisPointer);
		}
		bool DeRegisterInstance()noexcept
		{
			if (parent != nullptr)
			{
				parentInfo = std::make_unique<Core::JTypeInstanceSearchHint>(parent);
				const size_t guid = thisPointer->GetGuid();
				const uint pChildrenCount = (uint)parent->impl->children.size();
				for (uint i = 0; i < pChildrenCount; ++i)
				{
					if (guid == parent->impl->children[i]->GetGuid())
					{
						parent->impl->children.erase(parent->impl->children.begin() + i);
						parent = nullptr;
						break;
					}
				}
			}
			return  JScenePrivate::OwnTypeInterface::RemoveGameObject(thisPointer);
		}
	public:
		void RegisterThisPointer(JGameObject* gPtr)
		{
			thisPointer = Core::GetWeakPtr(gPtr); 
		}
		void CreateTransform()
		{
			transform = JICI::Create<JTransform>(thisPointer);
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JGameObject::StaticTypeInfo(), gPrivate);
			searchPtr = [](const JUserPtr<JComponent>& c, const J_COMPONENT_TYPE compType) {return c->GetComponentType() == compType; };
			IMPL_REALLOC_BIND(JGameObject::JGameObjectImpl, thisPointer)
		}
	};

	JGameObject::InitData::InitData(const JUserPtr<JGameObject>& parent)
		:JObject::InitData(JGameObject::StaticTypeInfo()), parent(parent)
	{
		if (parent != nullptr)
			name = JCUtil::MakeUniqueName(parent->GetChildren(), name);
	}
	JGameObject::InitData::InitData(const JUserPtr<JScene>& ownerScene)
		: JObject::InitData(JGameObject::StaticTypeInfo()), ownerScene(ownerScene)
	{
		if (parent != nullptr)
			name = JCUtil::MakeUniqueName(parent->GetChildren(), name);
	}
	JGameObject::InitData::InitData(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JUserPtr<JGameObject> parent, JUserPtr<JScene> ownerScene)
		: JObject::InitData(JGameObject::StaticTypeInfo(), name, guid, flag), parent(parent), ownerScene(ownerScene)
	{
		if (parent != nullptr)
			InitData::name = JCUtil::MakeUniqueName(parent->GetChildren(), name);
	}

	bool JGameObject::InitData::IsValidData()const noexcept
	{
		//root gPtr only has owner scene 
		//other gPtr need to have parent
		if (!JObject::InitData::IsValidData())
			return false;
		return parent != nullptr ? true : ownerScene != nullptr && (ownerScene->GetRootGameObject() == nullptr || ownerScene->GetDebugRootGameObject() == nullptr);
	}

	JGameObject::LoadData::LoadData(JUserPtr<JScene> ownerScene, JFileIOTool& tool)
		:ownerScene(ownerScene), tool(tool) {}
	JGameObject::LoadData::~LoadData() {}
	bool JGameObject::LoadData::IsValidData()const noexcept
	{
		return tool.CanLoad();
	}

	JGameObject::StoreData::StoreData(JUserPtr<JGameObject> obj, JFileIOTool& tool)
		:JObject::StoreData(obj), tool(tool)
	{}
	bool JGameObject::StoreData::IsValidData()const noexcept
	{
		return JObject::StoreData::IsValidData() && tool.CanStore();
	}

	size_t JGameObject::GetOwnerGuid()const noexcept
	{
		return impl->ownerScene->GetGuid();
	}
	JUserPtr<JScene> JGameObject::GetOwnerScene()const noexcept
	{
		return impl->ownerScene;
	}
	JUserPtr<JAnimator> JGameObject::GetAnimator()const noexcept
	{
		return impl->animator;
	}
	JUserPtr<JTransform> JGameObject::GetTransform()const noexcept
	{
		return impl->transform;
	}
	JUserPtr<JRenderItem> JGameObject::GetRenderItem()const noexcept
	{
		return impl->renderItem;
	}
	JUserPtr<JComponent> JGameObject::GetComponent(const J_COMPONENT_TYPE compType)const noexcept
	{
		return impl->GetComponent(compType);
	}
	JUserPtr<JComponent> JGameObject::GetComponentWithParent(const J_COMPONENT_TYPE compType)const noexcept
	{
		return impl->GetComponentWithParent(compType);
	}
	std::vector<JUserPtr<JComponent>> JGameObject::GetComponents(const J_COMPONENT_TYPE compType)const noexcept
	{
		std::vector<JUserPtr<JComponent>> result;
		impl->GetComponents(compType, result);
		return result;
	}
	std::vector<JUserPtr<JComponent>> JGameObject::GetComponentsWithParent(const J_COMPONENT_TYPE compType)const noexcept
	{
		std::vector<JUserPtr<JComponent>> result;
		impl->GetComponentsWithParent(compType, result);
		return result;
	}
	std::vector<JUserPtr<JComponent>> JGameObject::GetAllComponent()const noexcept
	{
		return impl->componentVec;;
	}
	uint JGameObject::GetAllComponentCount()const noexcept
	{
		return (uint)impl->componentVec.size();
	}
	uint JGameObject::GetComponentCount(const J_COMPONENT_TYPE type)const noexcept
	{
		return (uint)impl->GetComponentCount(type);
	}
	uint JGameObject::GetChildrenCount()const noexcept
	{
		return (uint)impl->children.size();
	}
	JUserPtr<JGameObject> JGameObject::GetParent()const noexcept
	{
		return impl->parent;
	}
	JUserPtr<JGameObject> JGameObject::GetChild(const uint index)const noexcept
	{
		if (index < impl->children.size())
			return impl->children[index];
		else
			return nullptr;
	}
	std::vector<JUserPtr<JGameObject>> JGameObject::GetChildren()const noexcept
	{
		return impl->children;
	}
	Core::JIdentifierPrivate& JGameObject::PrivateInterface()const noexcept
	{
		return gPrivate;
	}
	J_OBJECT_TYPE JGameObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::GAME_OBJECT;
	}
	void JGameObject::SetName(const std::wstring& newName)noexcept
	{
		if (newName == GetName())
			return;

		if (impl->parent != nullptr)
			JObject::SetName(JCUtil::MakeUniqueName(impl->parent->GetChildren(), newName));
	}
	bool JGameObject::IsRoot()const noexcept
	{
		return impl->parent == nullptr;
	}
	bool JGameObject::IsParentLine(JUserPtr<JGameObject> child)const noexcept
	{
		bool isParent = false;
		JUserPtr<JGameObject> childParent = child->GetParent();
		while (childParent != nullptr)
		{
			if (childParent->GetGuid() == GetGuid())
			{
				isParent = true;
				break;
			}
			childParent = childParent->GetParent();
		}
		return isParent;
	}
	bool JGameObject::IsSelected()const noexcept
	{
		return impl->isSelected;
	}
	bool JGameObject::IsEditorObject()const noexcept
	{
		return HasFlag(OBJECT_FLAG_ONLY_USED_IN_EDITOR);
	}
	bool JGameObject::HasComponent(const J_COMPONENT_TYPE type)const noexcept
	{
		return impl->HasComponent(type);
	}
	bool JGameObject::HasRenderItem()const noexcept
	{
		return (impl->renderItem != nullptr);
	}
	bool JGameObject::HasAnimator()const noexcept
	{
		return (impl->animator != nullptr);
	}
	bool JGameObject::CanAddComponent(const J_COMPONENT_TYPE type)const noexcept
	{
		return impl->CanAddComponent(type);
	}
	bool JGameObject::CanActivate() const noexcept
	{
		return impl->parent != nullptr ? impl->parent->IsActivated() : true;
	}
	bool JGameObject::CanDeActivate() const noexcept
	{
		return true;
	}
	void JGameObject::ChangeParent(JUserPtr<JGameObject> newParent)noexcept
	{
		if ((newParent == nullptr || impl->parent == nullptr) ||
			(newParent->GetOwnerScene()->GetGuid() != impl->parent->GetOwnerScene()->GetGuid()))
			return;

		if (impl->IsChild(newParent.Get()))
			return;

		int childIndex = -1;
		const size_t guid = GetGuid();
		for (int i = 0; i < impl->parent->impl->children.size(); ++i)
		{
			if (impl->parent->impl->children[i]->GetGuid() == guid)
			{
				childIndex = i;
				break;
			}
		}

		if (childIndex == -1)
			return;

		impl->parent->impl->children.erase(impl->parent->impl->children.begin() + childIndex);
		impl->parent = newParent;
		SetName(JCUtil::MakeUniqueName(impl->parent->impl->children, GetName()));
		newParent->impl->children.push_back(Core::GetUserPtr(this));

		JTransformPrivate::UpdateWorldInterface::UpdateWorld(impl->transform);
	}
	JUserPtr<JComponent> JGameObject::FindComponent(const size_t guid)const noexcept
	{
		return impl->FindComponent(guid);
	}
	void JGameObject::DoActivate()noexcept
	{
		if (impl->ownerScene != nullptr)
		{
			JObject::DoActivate();
			for (auto& data : impl->componentVec)
				JComponentPrivate::ActivateInterface::Activate(data);
			for (auto& data : impl->children)
				data->Activate();
		}
	}
	void JGameObject::DoDeActivate()noexcept
	{
		for (auto& data : impl->children)
			data->DeActivate();
		for (auto& data : impl->componentVec)
			JComponentPrivate::ActivateInterface::DeActivate(data);
		JObject::DoDeActivate();
	}

	JGameObject::JGameObject(const InitData& initData)
		:JObject(initData), impl(std::make_unique<JGameObjectImpl>(initData))
	{
		impl->Initialize(initData, this);
	}
	JGameObject::~JGameObject()
	{ 
		impl.reset(); 
	}

	using CreateInstanceInterface = JGameObjectPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JGameObjectPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JGameObjectPrivate::AssetDataIOInterface;
	using OwnTypeInterface = JGameObjectPrivate::OwnTypeInterface;
	using ActivateInterface = JGameObjectPrivate::ActivateInterface;
	using SelectInterface = JGameObjectPrivate::SelectInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		JGameObject::InitData* gInitData = static_cast<JGameObject::InitData*>(initData);
		if (gInitData->parent != nullptr)
			gInitData->name = JCUtil::MakeUniqueName(gInitData->parent->impl->children, gInitData->name);
		 
		return  Core::JPtrUtil::MakeOwnerPtr<JGameObject>(*gInitData);
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JGameObject::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JGameObject* gPtr = static_cast<JGameObject*>(createdPtr);
		gPtr->impl->RegisterThisPointer(gPtr);
		
		JGameObject::InitData* gInitData = static_cast<JGameObject::InitData*>(initData);
		if (gInitData->makeTransform)
			gPtr->impl->CreateTransform();
	}
	void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
	{
		JGameObject* gPtr = static_cast<JGameObject*>(createdPtr);
		gPtr->impl->RegisterInstance();
	}
	void CreateInstanceInterface::SetValidInstance(Core::JIdentifier* createdPtr)noexcept
	{
		JGameObject* gPtr = static_cast<JGameObject*>(createdPtr);
		if (gPtr->GetOwnerScene()->IsActivated())
			gPtr->Activate();
	}

	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JGameObject::StaticTypeInfo());
		if (!canCopy)
			return false;
		 
		return JGameObject::JGameObjectImpl::DoCopy(Core::ConvertChildUserPtr<JGameObject>(std::move(from)), Core::ConvertChildUserPtr<JGameObject>(std::move(to)));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		JGameObject* gPtr = static_cast<JGameObject*>(ptr);
		gPtr->impl->Clear(gPtr);
		JObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}
	void DestroyInstanceInterface::SetInvalidInstance(Core::JIdentifier* ptr)noexcept
	{
		JGameObject* gPtr = static_cast<JGameObject*>(ptr);
		if (gPtr->IsActivated())
			gPtr->DeActivate();
	}
	void DestroyInstanceInterface::DeRegisterCash(Core::JIdentifier* ptr)noexcept
	{
		JGameObject* gPtr = static_cast<JGameObject*>(ptr);
		gPtr->impl->DeRegisterInstance();
	}

	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateLoadAssetDIData(JUserPtr<JScene> invoker, JFileIOTool& tool)
	{
		if (invoker == nullptr)
			return std::unique_ptr < Core::JDITypeDataBase>{};

		return std::make_unique<JGameObject::LoadData>(invoker, tool);
	}
	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateStoreAssetDIData(JUserPtr<JGameObject> root, JFileIOTool& tool)
	{
		if (root == nullptr)
			return std::unique_ptr < Core::JDITypeDataBase>{};

		return std::make_unique<JGameObject::StoreData>(root, tool);
	}
	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JGameObject::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring name;
		size_t guid;
		J_OBJECT_FLAG flag;

		auto loadData = static_cast<JGameObject::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;

		JFileIOTool::NextCurrentHint* nextCurrentHint = tool.GetNextCurrentHint();
		if (nextCurrentHint == nullptr)
			nextCurrentHint = tool.CreateNextCurrentHint(std::make_unique<JFileIOTool::NextCurrentHint>(0));
		else
			++nextCurrentHint->index;

		FILE_TOOL_ASSERTION(tool.PushExistStack(std::to_string(nextCurrentHint->index)));
		FILE_ASSERTION(JObjectFileIOHelper::LoadObjectIden(tool, name, guid, flag));
		std::unique_ptr<JGameObject::InitData> initData = JGameObject::JGameObjectImpl::CreateInitData(name, guid, flag, loadData); 
		JUserPtr<Core::JIdentifier> res = gPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &gPrivate);
		auto newGameObject = Core::ConvertChildUserPtr<JGameObject>(std::move(res));
		if (newGameObject == nullptr)
			return nullptr;

		bool isSelected;
		bool isActivated;
		FILE_ASSERTION(JObjectFileIOHelper::LoadAtomicData(tool, isSelected, Core::JFileConstant::GetSelectedSymbol()));
		FILE_ASSERTION(JObjectFileIOHelper::LoadAtomicData(tool, isActivated, Core::JFileConstant::GetActivatedSymbol()));
		if (isSelected)
			newGameObject->impl->Select();
		if (!isActivated)
			newGameObject->DeActivate();

		int componentCount;
		FILE_ASSERTION(JObjectFileIOHelper::LoadAtomicData(tool, componentCount, "ComponentCount:"));
	
		FILE_TOOL_ASSERTION(tool.PushExistStack("ComponentData"));
		for (int i = 0; i < componentCount; ++i)
		{
			std::wstring componentName;
			size_t typeGuid;
			FILE_TOOL_ASSERTION(tool.PushExistStack());
			FILE_ASSERTION(JObjectFileIOHelper::LoadJString(tool, componentName, "ComponentTypeName:"));
			FILE_ASSERTION(JObjectFileIOHelper::LoadAtomicData(tool, typeGuid, "ComponentTypeGuid:"));
			auto compPrivate = static_cast<JComponentPrivate*>(Core::JIdentifier::PrivateInterface(typeGuid));
			auto compLoadData = JComponentPrivate::AssetDataIOInterface::CreateLoadAssetDIData(newGameObject, tool, typeGuid);
			compPrivate->GetAssetDataIOInterface().LoadAssetData(compLoadData.get());
			FILE_TOOL_ASSERTION(tool.PopStack());
		}
		FILE_TOOL_ASSERTION(tool.PopStack());

		int childrenCount;
		int childrenStoredCount;
		FILE_ASSERTION(JObjectFileIOHelper::LoadAtomicData(tool, childrenCount, "ChildrenCount:"));
		FILE_ASSERTION(JObjectFileIOHelper::LoadAtomicData(tool, childrenStoredCount, "ChildrenStoredCount:"));
		FILE_TOOL_ASSERTION(tool.PopStack());
		
		for (int i = 0; i < childrenStoredCount; ++i)
		{ 
			loadData->parent = newGameObject; 
			LoadAssetData(loadData);
		} 

		return newGameObject;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JGameObject::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto gmaeObjStoreData = static_cast<JGameObject::StoreData*>(data);
		if (!gmaeObjStoreData->HasCorrectType(JGameObject::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JFileIOTool& tool = gmaeObjStoreData->tool;
		JGameObject* gObj = static_cast<JGameObject*>(gmaeObjStoreData->obj.Get());

		JFileIOTool::NextCurrentHint* nextCurrentHint = tool.GetNextCurrentHint();
		int currentIndex = 0;
		if (nextCurrentHint == nullptr)
		{
			nextCurrentHint = tool.CreateNextCurrentHint(std::make_unique<JFileIOTool::NextCurrentHint>(0));
			currentIndex = 0;
		}
		else
		{
			++nextCurrentHint->index;
			currentIndex = nextCurrentHint->index;
		}

		FILE_TOOL_ASSERTION(tool.PushMapMember(std::to_string(currentIndex)));
		//tool.PushMapMember(std::to_string(gObj->GetGuid()));
		FILE_ASSERTION(JObjectFileIOHelper::StoreObjectIden(tool, gObj));
		FILE_ASSERTION(JObjectFileIOHelper::StoreAtomicData(tool, gObj->impl->isSelected, Core::JFileConstant::GetSelectedSymbol()));
		FILE_ASSERTION(JObjectFileIOHelper::StoreAtomicData(tool, gObj->IsActivated(), Core::JFileConstant::GetActivatedSymbol()));
		FILE_ASSERTION(JObjectFileIOHelper::StoreAtomicData(tool, gObj->impl->componentVec.size(), "ComponentCount:"));

		FILE_TOOL_ASSERTION(tool.PushArrayOwner("ComponentData"));
		for (auto& comp : gObj->impl->componentVec)
		{
			FILE_TOOL_ASSERTION(tool.PushArrayMember());
			FILE_ASSERTION(JObjectFileIOHelper::StoreJString(tool, comp->GetTypeInfo().Name(), "ComponentTypeName:"));
			FILE_ASSERTION(JObjectFileIOHelper::StoreAtomicData(tool, comp->GetTypeInfo().TypeGuid(), "ComponentTypeGuid:"));
			auto compPrivate = static_cast<JComponentPrivate*>(&comp->PrivateInterface());
			auto compStoreData = compPrivate->GetAssetDataIOInterface().CreateStoreAssetDIData(comp, tool);
			compPrivate->GetAssetDataIOInterface().StoreAssetData(compStoreData.get());
			FILE_TOOL_ASSERTION(tool.PopStack());
		}
		FILE_TOOL_ASSERTION(tool.PopStack());
		 
		FILE_ASSERTION(JObjectFileIOHelper::StoreAtomicData(tool, gObj->impl->children.size(), "ChildrenCount:"));
		FILE_TOOL_ASSERTION(tool.PopStack());

		int childrenStoredCount = 0;
		for (auto& child : gObj->impl->children)
		{
			gmaeObjStoreData->obj = child; 
			if (StoreAssetData(data) == Core::J_FILE_IO_RESULT::SUCCESS)
				++childrenStoredCount;
		}
		FILE_TOOL_ASSERTION(tool.PushMapMember(std::to_string(currentIndex)));
		FILE_ASSERTION(JObjectFileIOHelper::StoreAtomicData(tool, childrenStoredCount, "ChildrenStoredCount:"));
		FILE_TOOL_ASSERTION(tool.PopStack());
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	bool OwnTypeInterface::AddComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		return comp->GetOwner()->impl->AddComponent(comp);
	}
	bool OwnTypeInterface::RemoveComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		return comp->GetOwner()->impl->RemoveComponent(comp);
	}

	void ActivateInterface::Activate(const JUserPtr<JGameObject>& ptr)noexcept
	{
		ptr->Activate();
	}
	void ActivateInterface::DeActivate(const JUserPtr<JGameObject>& ptr)noexcept
	{
		ptr->DeActivate();
	}

	void SelectInterface::Select(const JUserPtr<JGameObject>& ptr)noexcept
	{
		ptr->impl->Select();
	}
	void SelectInterface::DeSelect(const JUserPtr<JGameObject>& ptr)noexcept
	{
		ptr->impl->DeSelect();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JGameObjectPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JGameObjectPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}
