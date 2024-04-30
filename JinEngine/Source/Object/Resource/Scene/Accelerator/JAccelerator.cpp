#include"JAccelerator.h"
#include"../../../GameObject/JGameObject.h"
#include"../../../Component/RenderItem/JRenderItem.h"
#include"../../../Component/Light/JLight.h"

namespace JinEngine
{
	namespace Private
	{
		static bool IsValidLayer(const J_RENDER_LAYER objLayer, const J_ACCELERATOR_LAYER layer)noexcept
		{
			return ConvertAcceleratorLayer(objLayer) == layer;
		}
		static void FindInnerObject(std::vector<JUserPtr<JGameObject>>& vec, const JUserPtr<JGameObject>& parent, const J_ACCELERATOR_LAYER layer) noexcept
		{
			if (layer == J_ACCELERATOR_LAYER::INVALID)
				return;

			if (parent == nullptr)
				return;

			auto rItem = parent->GetRenderItem();
			if (rItem != nullptr && rItem->IsActivated() && rItem->GetMesh() != nullptr && IsValidLayer(rItem->GetRenderLayer(), layer))
				vec.push_back(parent);

			std::vector<JUserPtr<JGameObject>> children = parent->GetChildren();
			for (const auto& data : children)
				FindInnerObject(vec, data, layer);
		} 
		static void FindInnerNonDeltaLight(std::vector<JUserPtr<JGameObject>>& vec, const JUserPtr<JGameObject>& parent) noexcept
		{ 
			if (parent == nullptr)
				return;

			auto light = parent->GetComponents(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
			for (const auto& data : light)
			{
				JUserPtr<JLight> lit = Core::ConnectChildUserPtr<JLight>(data);
				if (lit->GetLightType() == J_LIGHT_TYPE::RECT)
					vec.push_back(lit->GetOwner());
			}

			std::vector<JUserPtr<JGameObject>> children = parent->GetChildren();
			for (const auto& data : children)
				FindInnerNonDeltaLight(vec, data);
		}
	}
	JCpuAccelerator::JCpuAccelerator(const J_ACCELERATOR_LAYER layer)
		:layer(layer)
	{}
	void JCpuAccelerator::Clear()noexcept
	{
		UnBuild();
		innerRoot = nullptr;
		debugRoot = nullptr;
		isAcceleratorActivated = false;
		isDebugActivated = false;
		isDebugLeafOnly = true;
	}
	J_ACCELERATOR_LAYER JCpuAccelerator::GetLayer()const noexcept
	{
		return layer;
	}
	std::vector<JUserPtr<JGameObject>> JCpuAccelerator::GetInnerObject()const noexcept
	{
		std::vector<JUserPtr<JGameObject>> innerVec;
		Private::FindInnerObject(innerVec, innerRoot, layer);
		return innerVec;
	}
	JUserPtr<JGameObject> JCpuAccelerator::GetInnerRoot()const noexcept
	{
		return innerRoot;
	}
	JUserPtr<JGameObject> JCpuAccelerator::GetDebugRoot()const noexcept
	{
		return debugRoot;
	}
	JAcceleratorOption JCpuAccelerator::GetCommonOption()const noexcept
	{
		return JAcceleratorOption(innerRoot, debugRoot, isAcceleratorActivated, isDebugActivated, isDebugLeafOnly, isCullingActivated);
	}
	void JCpuAccelerator::SetInnerRoot(const JUserPtr<JGameObject>& newInnerRoot)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		bool isSameInnerRoot = false;
		bool hasInnerRoot = innerRoot != nullptr && newInnerRoot != nullptr;
		if (hasInnerRoot)
		{
			if (innerRoot->GetGuid() == newInnerRoot->GetGuid())
				isSameInnerRoot = true;
		}
		else
		{
			if (innerRoot == nullptr && newInnerRoot == nullptr)
				isSameInnerRoot = true;
		}
		if (!isSameInnerRoot)
		{
			if (newInnerRoot == nullptr)
				SetAcceleratorActivate(false);

			innerRoot = newInnerRoot;
			UnBuild();
			if (isAcceleratorActivated)
				Build();
		}
	}
	void JCpuAccelerator::SetDebugRoot(const JUserPtr<JGameObject>& newDebugRoot)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		bool isSameDebugRoot = false;
		bool hasDebugRoot = debugRoot != nullptr && newDebugRoot != nullptr;
		if (hasDebugRoot)
		{
			if (debugRoot->GetGuid() == newDebugRoot->GetGuid())
				isSameDebugRoot = true;
		}
		else
		{
			if (debugRoot == nullptr && newDebugRoot == nullptr)
				isSameDebugRoot = true;
		}
		if (!isSameDebugRoot)
		{
			if (newDebugRoot == nullptr)
				SetDebugActivated(false);

			debugRoot = newDebugRoot;
			OffDebugGameObject();
			if (isDebugActivated)
				OnDebugGameObject();
		}
	}
	void JCpuAccelerator::SetAcceleratorActivate(bool value)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		if (value != isAcceleratorActivated)
		{
			if (HasInnerRoot())
			{
				isAcceleratorActivated = value;
				if (value)
					Build();
				else
					UnBuild();
			}
		}
	}
	void JCpuAccelerator::SetDebugActivated(bool value)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		//if (layer == J_ACCELERATOR_LAYER::DEBUG_OBJECT)
		//	return;

		if (value != isDebugActivated)
		{
			if (HasDebugRoot())
			{
				isDebugActivated = value;
				if (value)
					OnDebugGameObject();
				else
					OffDebugGameObject();
			}
		}
	}
	void JCpuAccelerator::SetDebugLeafOnly(bool value)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		//if (layer == J_ACCELERATOR_LAYER::DEBUG_OBJECT)
		//	return;

		if (isDebugLeafOnly != value)
		{
			isDebugLeafOnly = value;
			if (isDebugActivated && HasDebugRoot())
			{
				OffDebugGameObject();
				OnDebugGameObject();
			}
		}
	}
	void JCpuAccelerator::SetCullingActivate(bool value)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		if (isCullingActivated != value)
			isCullingActivated = value;
	}
	void JCpuAccelerator::SetCommonOption(const JAcceleratorOption& newOption)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		SetInnerRoot(newOption.innerRoot);
		SetDebugRoot(newOption.debugRoot);
		SetAcceleratorActivate(newOption.isAcceleratorActivated);
		SetDebugActivated(newOption.isDebugActivated);
		SetDebugLeafOnly(newOption.isDebugLeafOnly);
		SetCullingActivate(newOption.isCullingActivated);
	}
	bool JCpuAccelerator::IsInnerRoot(const JUserPtr<JGameObject>& gameObj)const noexcept
	{
		if (innerRoot == nullptr)
			return false;
		return gameObj->GetGuid() == innerRoot->GetGuid();
	}
	bool JCpuAccelerator::IsDebugRoot(const JUserPtr<JGameObject>& gameObj)const noexcept
	{
		if (debugRoot == nullptr)
			return false;
		return gameObj->GetGuid() == debugRoot->GetGuid();
	}
	bool JCpuAccelerator::IsAcceleratorActivated()const noexcept
	{
		return isAcceleratorActivated;
	}
	bool JCpuAccelerator::IsDebugActivated()const noexcept
	{
		return isDebugActivated;
	}
	bool JCpuAccelerator::IsDebugLeafOnly()const noexcept
	{
		return isDebugLeafOnly;
	}
	bool JCpuAccelerator::IsCullingActivated()const noexcept
	{
		return isCullingActivated;
	}
	bool JCpuAccelerator::IsValidLayer(const J_RENDER_LAYER objLayer)const noexcept
	{
		return Private::IsValidLayer(objLayer, layer);
	}
	bool JCpuAccelerator::HasInnerRoot()const noexcept
	{
		return innerRoot != nullptr;
	}
	bool JCpuAccelerator::HasDebugRoot()const noexcept
	{
		return debugRoot != nullptr;
	}
	bool JCpuAccelerator::CanAddGameObject(const JUserPtr<JGameObject>& gameObj)const noexcept
	{
		return gameObj->HasRenderItem() &&
			((gameObj->GetRenderItem()->GetAcceleratorMask() & ACCELERATOR_ALLOW_BUILD) > 0) &&
			(ConvertAcceleratorLayer(gameObj->GetRenderItem()->GetRenderLayer()) == layer) &&
			innerRoot->IsParentLine(gameObj);
	}

	JGpuAccelerator::JGpuAccelerator()
	{
		RegisterInterfacePointer();
	}
	void JGpuAccelerator::Build()noexcept
	{ 
		Graphic::JGpuAcceleratorBuildDesc desc;
		desc.flag = option.flag;
		Private::FindInnerObject(desc.obj, option.root, J_ACCELERATOR_LAYER::COMMON_OBJECT);
		if (Core::HasSQValueEnum(desc.flag, Graphic::J_GPU_ACCELERATOR_BUILD_OPTION_LIGHT_SHAPE))
			Private::FindInnerNonDeltaLight(desc.localLight, option.root);
		JGpuAcceleratorInterface::CreateGpuAccelerator(desc);
	}
	void JGpuAccelerator::UnBuild()noexcept
	{
		JGpuAcceleratorInterface::DestroyGpuAccelerator();
	}
	void JGpuAccelerator::Clear()noexcept
	{
		UnBuild();
	}
	void JGpuAccelerator::UpdateTransform(const JUserPtr<JComponent>& comp)noexcept
	{
		JGpuAcceleratorInterface::UpdateTransform(comp);
	}
	void JGpuAccelerator::AddComponent(const JUserPtr<JComponent>& newComp)noexcept
	{
		JGpuAcceleratorInterface::AddComponent(newComp);
	}
	void JGpuAccelerator::RemoveComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		JGpuAcceleratorInterface::RemoveComponent(comp);
	}
	JGpuAcceleratorOption JGpuAccelerator::GetOption()const noexcept
	{
		return option;
	}
	void JGpuAccelerator::SetOption(const JGpuAcceleratorOption& newOption)
	{
		if (JGpuAcceleratorInterface::HasInfo())
			UnBuild();

		option = newOption;
		Build();
	}
	bool JGpuAccelerator::CanBuild()noexcept
	{
		return JGpuAcceleratorInterface::CanBuildGpuAccelerator();
	}
	void JGpuAccelerator::RegisterInterfacePointer()
	{
		Graphic::JGpuAcceleratorInterface::SetInterfacePointer(this);
	}
}
 