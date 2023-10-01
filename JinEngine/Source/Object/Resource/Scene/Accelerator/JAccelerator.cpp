#include"JAccelerator.h"
#include"../../../GameObject/JGameObject.h"
#include"../../../Component/RenderItem/JRenderItem.h"

namespace JinEngine
{
	JAccelerator::JAccelerator(const J_ACCELERATOR_LAYER layer)
		:layer(layer)
	{}
	void JAccelerator::Clear()noexcept
	{
		innerRoot = nullptr;
		debugRoot = nullptr;
		isAcceleratorActivated = false;
		isDebugActivated = false;
		isDebugLeafOnly = true;
	}
	J_ACCELERATOR_LAYER JAccelerator::GetLayer()const noexcept
	{
		return layer;
	}
	std::vector<JUserPtr<JGameObject>> JAccelerator::GetInnerObject()const noexcept
	{
		std::vector<JUserPtr<JGameObject>> innerVec;
		FindInnerObject(innerVec, innerRoot);
		return innerVec;
	}
	JUserPtr<JGameObject> JAccelerator::GetInnerRoot()const noexcept
	{
		return innerRoot;
	}
	JUserPtr<JGameObject> JAccelerator::GetDebugRoot()const noexcept
	{
		return debugRoot;
	}
	JAcceleratorOption JAccelerator::GetCommonOption()const noexcept
	{
		return JAcceleratorOption(innerRoot, debugRoot, isAcceleratorActivated, isDebugActivated, isDebugLeafOnly, isCullingActivated);
	}
	void JAccelerator::SetInnerRoot(const JUserPtr<JGameObject>& newInnerRoot)noexcept
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
	void JAccelerator::SetDebugRoot(const JUserPtr<JGameObject>& newDebugRoot)noexcept
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
	void JAccelerator::SetAcceleratorActivate(bool value)noexcept
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
	void JAccelerator::SetDebugActivated(bool value)noexcept
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
	void JAccelerator::SetDebugLeafOnly(bool value)noexcept
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
	void JAccelerator::SetCullingActivate(bool value)noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		if (isCullingActivated != value)
			isCullingActivated = value;
	}
	void JAccelerator::SetCommonOption(const JAcceleratorOption& newOption)noexcept
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
	bool JAccelerator::IsInnerRoot(const JUserPtr<JGameObject>& gameObj)const noexcept
	{
		if (innerRoot == nullptr)
			return false;
		return gameObj->GetGuid() == innerRoot->GetGuid();
	}
	bool JAccelerator::IsDebugRoot(const JUserPtr<JGameObject>& gameObj)const noexcept
	{
		if (debugRoot == nullptr)
			return false;
		return gameObj->GetGuid() == debugRoot->GetGuid();
	}
	bool JAccelerator::IsAcceleratorActivated()const noexcept
	{
		return isAcceleratorActivated;
	}
	bool JAccelerator::IsDebugActivated()const noexcept
	{
		return isDebugActivated;
	}
	bool JAccelerator::IsDebugLeafOnly()const noexcept
	{
		return isDebugLeafOnly;
	}
	bool JAccelerator::IsCullingActivated()const noexcept
	{
		return isCullingActivated;
	}
	bool JAccelerator::IsValidLayer(const J_RENDER_LAYER objLayer)const noexcept
	{
		return ConvertAcceleratorLayer(objLayer) == layer;
	}
	bool JAccelerator::HasInnerRoot()const noexcept
	{
		return innerRoot != nullptr;
	}
	bool JAccelerator::HasDebugRoot()const noexcept
	{
		return debugRoot != nullptr;
	}
	bool JAccelerator::CanAddGameObject(const JUserPtr<JGameObject>& gameObj)const noexcept
	{
		return gameObj->HasRenderItem() &&
			((gameObj->GetRenderItem()->GetAcceleratorMask() & ACCELERATOR_ALLOW_BUILD) > 0) &&
			(ConvertAcceleratorLayer(gameObj->GetRenderItem()->GetRenderLayer()) == layer) &&
			innerRoot->IsParentLine(gameObj);
	}
	void JAccelerator::FindInnerObject(std::vector<JUserPtr<JGameObject>>& vec, const JUserPtr<JGameObject>& parent)const noexcept
	{
		if (layer == J_ACCELERATOR_LAYER::INVALID)
			return;

		if (parent == nullptr)
			return;

		if (parent->HasRenderItem() && IsValidLayer(parent->GetRenderItem()->GetRenderLayer()))
			vec.push_back(parent);

		std::vector<JUserPtr<JGameObject>> children = parent->GetChildren();
		for (const auto& data : children)
			FindInnerObject(vec, data);
	}
}
