#include"JSpaceSpatial.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"

namespace JinEngine
{
	namespace Core
	{
		JSpaceSpatial::JSpaceSpatial(const J_SPACE_SPATIAL_LAYER layer)
			:layer(layer)
		{}
		void JSpaceSpatial::Clear()noexcept
		{
			innerRoot = nullptr;
			debugRoot = nullptr;
			isSpaceSpatialActivated = false;
			isDebugActivated = false;
			isDebugLeafOnly = true;
		}
		bool JSpaceSpatial::IsInnerRoot(JGameObject* gameObj)const noexcept
		{
			if (innerRoot == nullptr)
				return false;
			return gameObj->GetGuid() == innerRoot->GetGuid();
		}
		bool JSpaceSpatial::IsDebugRoot(JGameObject* gameObj)const noexcept
		{
			if (debugRoot == nullptr)
				return false;
			return gameObj->GetGuid() == debugRoot->GetGuid();
		}
		bool JSpaceSpatial::IsSpaceSpatialActivated()const noexcept
		{
			return isSpaceSpatialActivated;
		}
		bool JSpaceSpatial::IsDebugActivated()const noexcept
		{
			return isDebugActivated;
		}
		bool JSpaceSpatial::IsDebugLeafOnly()const noexcept
		{
			return isDebugLeafOnly;
		}
		bool JSpaceSpatial::IsCullingActivated()const noexcept
		{
			return isCullingActivated;
		}
		bool JSpaceSpatial::IsValidLayer(const J_RENDER_LAYER objLayer)const noexcept
		{
			return ConvertSpaceSpatialLayer(objLayer) == layer;
		}
		bool JSpaceSpatial::HasInnerRoot()const noexcept
		{
			return innerRoot != nullptr;
		}
		bool JSpaceSpatial::HasDebugRoot()const noexcept
		{
			return debugRoot != nullptr;
		}
		bool JSpaceSpatial::CanAddGameObject(JGameObject* gameObj)const noexcept
		{ 
			return gameObj->HasRenderItem() &&
				(gameObj->GetRenderItem()->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_BUILD) > 0 &&
				ConvertSpaceSpatialLayer(gameObj->GetRenderItem()->GetRenderLayer()) == layer &&
				innerRoot->IsParentLine(gameObj);
		}
		J_SPACE_SPATIAL_LAYER JSpaceSpatial::GetLayer()const noexcept
		{
			return layer;
		}
		std::vector<JGameObject*> JSpaceSpatial::GetInnerObject()const noexcept
		{
			std::vector<JGameObject*> innerVec;
			FindInnerObject(innerVec, innerRoot);
			return innerVec;
		}
		JGameObject* JSpaceSpatial::GetInnerRoot()const noexcept
		{
			return innerRoot;
		}
		JGameObject* JSpaceSpatial::GetDebugRoot()const noexcept
		{
			return debugRoot;
		}
		JSpaceSpatialOption JSpaceSpatial::GetCommonOption()const noexcept
		{
			return JSpaceSpatialOption(innerRoot, debugRoot, isSpaceSpatialActivated, isDebugActivated, isDebugLeafOnly, isCullingActivated);
		}
		void JSpaceSpatial::SetInnerRoot(JGameObject* newInnerRoot)noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
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
					SetSpaceSpatialActivate(false);

				innerRoot = newInnerRoot;
				UnBuild();
				if (isSpaceSpatialActivated)
					Build();
			}	 
		}
		void JSpaceSpatial::SetDebugRoot(JGameObject* newDebugRoot)noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
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
		void JSpaceSpatial::SetSpaceSpatialActivate(bool value)noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
				return;

			if (value != isSpaceSpatialActivated)
			{
				if (HasInnerRoot())
				{
					isSpaceSpatialActivated = value;
					if (value)
						Build();
					else
						UnBuild();
				}
			}
		}
		void JSpaceSpatial::SetDebugActivated(bool value)noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
				return;

			//if (layer == J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT)
			//	return;

			if (value != isDebugActivated)
			{
				if(HasDebugRoot())
				{
					isDebugActivated = value;
					if (value)
						OnDebugGameObject();
					else
						OffDebugGameObject();
				}
			}
		}
		void JSpaceSpatial::SetDebugLeafOnly(bool value)noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
				return;

			//if (layer == J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT)
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
		void JSpaceSpatial::SetCullingActivate(bool value)noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
				return;

			if(isCullingActivated != value)
			{
				isCullingActivated = value;
				if (!isCullingActivated)
					OffCulling();
			}
		}
		void JSpaceSpatial::SetCommonOption(const JSpaceSpatialOption& newOption)noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
				return;

			SetInnerRoot(newOption.innerRoot);
			SetDebugRoot(newOption.debugRoot);
			SetSpaceSpatialActivate(newOption.isSpaceSpatialActivated);
			SetDebugActivated(newOption.isDebugActivated);
			SetDebugLeafOnly(newOption.isDebugLeafOnly);
			SetCullingActivate(newOption.isCullingActivated);
		}
		void JSpaceSpatial::FindInnerObject(std::vector<JGameObject*>& vec, JGameObject* parent)const noexcept
		{
			if (layer == J_SPACE_SPATIAL_LAYER::INVALID)
				return;

			if (parent == nullptr)
				return;

			if (parent->HasRenderItem() && IsValidLayer(parent->GetRenderItem()->GetRenderLayer()))
				vec.push_back(parent);

			std::vector<JGameObject*> children = parent->GetChildren();
			for (const auto& data : children)
				FindInnerObject(vec, data);
		}
	}
}