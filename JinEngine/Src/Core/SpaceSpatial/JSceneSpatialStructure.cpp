#include"JSceneSpatialStructure.h"
#include"Octree/JOctree.h"
#include"Bvh/JBvh.h"
#include"Kd-tree/JKdTree.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Core
	{ 
		namespace
		{
			static constexpr bool onlyBvhCanFrustumCulling = true;
		}

		JOctreeOption JSceneSpatialStructure::ActivatedOptionCash::GetOctreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			JOctreeOption option = preOctreeOption[(int)layer];
			LoadRoot(option.commonOption, J_SPACE_SPATIAL_TYPE::OCTREE, layer);
			return option;
		}
		JBvhOption JSceneSpatialStructure::ActivatedOptionCash::GetBvhOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			JBvhOption option = preBvhOption[(int)layer];
			LoadRoot(option.commonOption, J_SPACE_SPATIAL_TYPE::BVH, layer);
			return option;
		}
		JKdTreeOption JSceneSpatialStructure::ActivatedOptionCash::GetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			JKdTreeOption option = preKdTreeOption[(int)layer];
			LoadRoot(option.commonOption, J_SPACE_SPATIAL_TYPE::KD_TREE, layer);
			return option;
		}
		void JSceneSpatialStructure::ActivatedOptionCash::SetOctreeOption(const J_SPACE_SPATIAL_LAYER layer, const JOctreeOption& octreeOption)
		{ 
			preOctreeOption[(uint)layer] = octreeOption;
			StoreRoot(octreeOption.commonOption, J_SPACE_SPATIAL_TYPE::OCTREE, layer);
		}
		void JSceneSpatialStructure::ActivatedOptionCash::SetBvhOption(const J_SPACE_SPATIAL_LAYER layer, const JBvhOption& bvhOption)
		{ 
			preBvhOption[(uint)layer] = bvhOption;
			StoreRoot(bvhOption.commonOption, J_SPACE_SPATIAL_TYPE::BVH, layer);
		}
		void JSceneSpatialStructure::ActivatedOptionCash::SetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer, const JKdTreeOption& kdTreeOption)
		{
			preKdTreeOption[(uint)layer] = kdTreeOption;
			StoreRoot(kdTreeOption.commonOption, J_SPACE_SPATIAL_TYPE::KD_TREE, layer);
		}
		void JSceneSpatialStructure::ActivatedOptionCash::LoadRoot(JSpaceSpatialOption& option, const J_SPACE_SPATIAL_TYPE type, const J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			uint typeIndex = (uint)type;
			uint layerIndex = (uint)layer;

			option.innerRoot = nullptr;
			if (preHasInnerRoot[typeIndex][layerIndex])
				option.innerRoot = Core::GetUserPtr<JGameObject>(preInnerRootGuid[typeIndex][layerIndex]);

			option.debugRoot = nullptr;
			if (preHasDebugRoot[typeIndex][layerIndex])
				option.debugRoot = Core::GetUserPtr<JGameObject>(preDebugRootGuid[typeIndex][layerIndex]);
		}
		void JSceneSpatialStructure::ActivatedOptionCash::StoreRoot(const JSpaceSpatialOption& option, const J_SPACE_SPATIAL_TYPE type, const J_SPACE_SPATIAL_LAYER layer)noexcept
		{
			uint typeIndex = (uint)type;
			uint layerIndex = (uint)layer;

			preHasInnerRoot[typeIndex][layerIndex] = option.HasInnerRoot();
			if (preHasInnerRoot[typeIndex][layerIndex])
				preInnerRootGuid[typeIndex][layerIndex] = option.innerRoot->GetGuid();
			else
				preInnerRootGuid[typeIndex][layerIndex] = 0;

			preHasDebugRoot[typeIndex][layerIndex] = option.HasDebugRoot();
			if (preHasDebugRoot[typeIndex][layerIndex])
				preDebugRootGuid[typeIndex][layerIndex] = option.debugRoot->GetGuid();
			else
				preDebugRootGuid[typeIndex][layerIndex] = 0;
		}

		JSceneSpatialStructure::JSceneSpatialStructure() 
		{ 
			for(uint i = 0; i < (uint)J_SPACE_SPATIAL_LAYER::COUNT; ++i) 
			{
				octree[i] = std::make_unique<JOctree>((J_SPACE_SPATIAL_LAYER)i);
				bvh[i] = std::make_unique<JBvh>((J_SPACE_SPATIAL_LAYER)i);
				kdTree[i] = std::make_unique<JKdTree>((J_SPACE_SPATIAL_LAYER)i);
		
				spaceSpatialVec.push_back(octree[i].get());
				spaceSpatialVec.push_back(bvh[i].get());
				spaceSpatialVec.push_back(kdTree[i].get());
			}		  
		}
		JSceneSpatialStructure::~JSceneSpatialStructure(){}
		void JSceneSpatialStructure::Clear()noexcept
		{
			for (uint i = 0; i < (uint)J_SPACE_SPATIAL_LAYER::COUNT; ++i)
			{
				octree[i]->Clear();
				bvh[i]->Clear();
				kdTree[i]->Clear();

				octree[i].reset();
				bvh[i].reset();
				kdTree[i].reset();
			}
			spaceSpatialVec.clear();
		}
		/*void JSceneSpatialStructure::Culling(const JCullingFrustum& camFrustum)noexcept
		{
			for (const auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated() && data->IsCullingActivated())
					data->Culling(camFrustum);			 
			}
		}*/
		void JSceneSpatialStructure::Culling(Graphic::JCullingUserInterface cullUser, const DirectX::BoundingFrustum& camFrustum)noexcept
		{
			DirectX::BoundingFrustum nearFrustum = camFrustum;
			nearFrustum.Near = 0;
			nearFrustum.Far = camFrustum.Near;

			for (const auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated() && data->IsCullingActivated())
					data->Culling(cullUser, camFrustum, nearFrustum);
			}
		}
		JUserPtr<JGameObject> JSceneSpatialStructure::IntersectFirst(const J_SPACE_SPATIAL_LAYER layer, const Core::JRay& ray)const noexcept
		{
			if(bvh[(int)layer] != nullptr)
				return bvh[(int)layer]->IntersectFirst(ray);
			else
			{
				for (const auto& data : spaceSpatialVec)
				{
					if (data->IsSpaceSpatialActivated() && data->GetLayer() == layer)
						return data->IntersectFirst(ray);
				}
			}
			return nullptr;
		}
		void JSceneSpatialStructure::UpdateGameObject(const JUserPtr<JGameObject>& gameObject)noexcept
		{
			if (!gameObject->HasRenderItem())
				return;
			 
			for (auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated())
					data->UpdateGameObject(gameObject);
			}
		}
		void JSceneSpatialStructure::AddGameObject(const JUserPtr<JGameObject>& gameObject)noexcept
		{ 
			for (auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated() && data->CanAddGameObject(gameObject))
					data->AddGameObject(gameObject);
			}
		}
		void JSceneSpatialStructure::RemoveGameObject(const JUserPtr<JGameObject>& gameObject)noexcept
		{
			if (gameObject == nullptr)
				return;

			for (auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated())
					data->RemoveGameObject(gameObject);
			}
		}
		std::vector<JUserPtr<JGameObject>> JSceneSpatialStructure::GetAlignedObject(const J_SPACE_SPATIAL_LAYER layer, const DirectX::BoundingFrustum& camFrustum)const noexcept
		{
			if (activateTrigger)
			{
				if (kdTree[(uint)layer]->IsSpaceSpatialActivated())
					return kdTree[(uint)layer]->GetAlignedObject(camFrustum);
			}
			return std::vector<JUserPtr<JGameObject>>();
		}
		JOctreeOption JSceneSpatialStructure::GetOctreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			return octree[(uint)layer]->GetOctreeOption();
		}
		JBvhOption JSceneSpatialStructure::GetBvhOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			return bvh[(uint)layer]->GetBvhOption();
		}
		JKdTreeOption JSceneSpatialStructure::GetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept
		{
			return kdTree[(uint)layer]->GetKdTreeOption();
		}
		void JSceneSpatialStructure::SetOctreeOption(const J_SPACE_SPATIAL_LAYER layer, const JOctreeOption& option)
		{
			if (activateTrigger)
				octree[(uint)layer]->SetOctreeOption(option);
			else
				optionCash->SetOctreeOption(layer, option);
		}
		void JSceneSpatialStructure::SetBvhOption(const J_SPACE_SPATIAL_LAYER layer, const JBvhOption& option)
		{
			if (activateTrigger)
				bvh[(uint)layer]->SetBvhOption(option);
			else
				optionCash->SetBvhOption(layer, option);
		}
		void JSceneSpatialStructure::SetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer, const JKdTreeOption& option)
		{
			if (activateTrigger)
				kdTree[(uint)layer]->SetKdTreeOption(option);
			else
				optionCash->SetKdTreeOption(layer, option);
		}
		void JSceneSpatialStructure::Activate()noexcept
		{
			if (!activateTrigger)
			{
				for (uint i = 0; i < (uint)J_SPACE_SPATIAL_LAYER::COUNT; ++i)
				{
					octree[i]->SetOctreeOption(optionCash->GetOctreeOption((J_SPACE_SPATIAL_LAYER)i));
					bvh[i]->SetBvhOption(optionCash->GetBvhOption((J_SPACE_SPATIAL_LAYER)i));
					kdTree[i]->SetKdTreeOption(optionCash->GetKdTreeOption((J_SPACE_SPATIAL_LAYER)i));
					
					spaceSpatialVec.push_back(octree[i].get());
					spaceSpatialVec.push_back(bvh[i].get());
					spaceSpatialVec.push_back(kdTree[i].get());
				}
				 			
				optionCash.reset();
				activateTrigger = true;
			}
		}
		void JSceneSpatialStructure::DeAcitvate()noexcept
		{
			if (activateTrigger)
			{
				optionCash = std::make_unique<ActivatedOptionCash>();
				for (uint i = 0; i < (uint)J_SPACE_SPATIAL_LAYER::COUNT; ++i)
				{
					optionCash->SetOctreeOption(octree[i]->GetLayer(), octree[i]->GetOctreeOption());
					optionCash->SetBvhOption(bvh[i]->GetLayer(), bvh[i]->GetBvhOption());
					optionCash->SetKdTreeOption(kdTree[i]->GetLayer(), kdTree[i]->GetKdTreeOption());

					octree[i]->Clear();
					bvh[i]->Clear();
					kdTree[i]->Clear();
				}
 
				spaceSpatialVec.clear();
				activateTrigger = false;
			}
		}
		void JSceneSpatialStructure::BuildDebugTree(const Core::J_SPACE_SPATIAL_TYPE type, 
			const J_SPACE_SPATIAL_LAYER layer,
			Editor::JEditorBinaryTreeView& tree)noexcept
		{
			if (activateTrigger)
			{
				switch (type)
				{
				case JinEngine::Core::J_SPACE_SPATIAL_TYPE::OCTREE:
				{
					break;
				}
				case JinEngine::Core::J_SPACE_SPATIAL_TYPE::BVH:
				{
					bvh[(uint)layer]->BuildDebugTree(tree);
					break;
				} 
				case JinEngine::Core::J_SPACE_SPATIAL_TYPE::KD_TREE:
				{
					kdTree[(uint)layer]->BuildDebugTree(tree);
					break;
				}
				default:
					break;
				}
			}
		}
	}
}