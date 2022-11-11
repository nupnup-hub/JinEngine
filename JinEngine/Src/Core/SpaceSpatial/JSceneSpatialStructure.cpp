#include"JSceneSpatialStructure.h"
#include"Octree/JOctree.h"
#include"Bvh/JBvh.h"
#include"Kd-tree/JKdTree.h"
#include"../../Object/GameObject/JGameObject.h"
 
namespace JinEngine
{
	namespace Core
	{
		JSceneSpatialStructure::ActivatedOptionCash::ActivatedOptionCash(const JOctreeOption& octreeOption,
			const JBvhOption& bvhOption,
			const JKdTreeOption& kdTreeOption)
		{
			SetOctreeOption(octreeOption);
			SetBvhOption(bvhOption);
			SetKdTreeOption(kdTreeOption);
		}
		JOctreeOption JSceneSpatialStructure::ActivatedOptionCash::GetOctreeOption()const noexcept
		{
			JGameObject* innerRoot = nullptr;
			if (preHasInnerRoot[(int)J_SPACE_SPATIAL_TYPE::OCTREE])
				innerRoot = Core::GetRawPtr<JGameObject>(preInnerRootGuid[octreeIndex]);

			JGameObject* debugRoot = nullptr;
			if (preHasDebugRoot[octreeIndex])
				debugRoot = Core::GetRawPtr<JGameObject>(preDebugRootGuid[octreeIndex]);

			JOctreeOption option = preOctreeOption;
			option.commonOption.innerRoot = innerRoot;
			option.commonOption.debugRoot = debugRoot;

			return option;
		}
		JBvhOption JSceneSpatialStructure::ActivatedOptionCash::GetBvhOption()const noexcept
		{
			JGameObject* innerRoot = nullptr;
			if (preHasInnerRoot[bvhIndex])
				innerRoot = Core::GetRawPtr<JGameObject>(preInnerRootGuid[bvhIndex]);

			JGameObject* debugRoot = nullptr;
			if (preHasDebugRoot[bvhIndex])
				debugRoot = Core::GetRawPtr<JGameObject>(preDebugRootGuid[bvhIndex]);

			JBvhOption option = preBvhOption;
			option.commonOption.innerRoot = innerRoot;
			option.commonOption.debugRoot = debugRoot;

			return option;
		}
		JKdTreeOption JSceneSpatialStructure::ActivatedOptionCash::GetKdTreeOption()const noexcept
		{
			JGameObject* innerRoot = nullptr;
			if (preHasInnerRoot[kdTreeIndex])
				innerRoot = Core::GetRawPtr<JGameObject>(preInnerRootGuid[kdTreeIndex]);

			JGameObject* debugRoot = nullptr;
			if (preHasDebugRoot[kdTreeIndex])
				debugRoot = Core::GetRawPtr<JGameObject>(preDebugRootGuid[kdTreeIndex]);

			JKdTreeOption option = preKdTreeOption;
			option.commonOption.innerRoot = innerRoot;
			option.commonOption.debugRoot = debugRoot;

			return option;
		}
		void JSceneSpatialStructure::ActivatedOptionCash::SetOctreeOption(const JOctreeOption& octreeOption)
		{
			preOctreeOption = octreeOption;
			preHasInnerRoot[octreeIndex] = octreeOption.commonOption.HasInnerRoot();
			if (preHasInnerRoot[octreeIndex])
				preInnerRootGuid[octreeIndex] = octreeOption.commonOption.innerRoot->GetGuid();
			else
				preInnerRootGuid[octreeIndex] = 0;

			preHasDebugRoot[octreeIndex] = octreeOption.commonOption.HasDebugRoot();
			if (preHasDebugRoot[octreeIndex])
				preDebugRootGuid[octreeIndex] = octreeOption.commonOption.debugRoot->GetGuid();
			else
				preDebugRootGuid[octreeIndex] = 0;
		}
		void JSceneSpatialStructure::ActivatedOptionCash::SetBvhOption(const JBvhOption& bvhOption)
		{
			preBvhOption = bvhOption;
			preHasInnerRoot[bvhIndex] = bvhOption.commonOption.HasInnerRoot();
			if (preHasInnerRoot[bvhIndex])
				preInnerRootGuid[bvhIndex] = bvhOption.commonOption.innerRoot->GetGuid();
			else
				preInnerRootGuid[bvhIndex] = 0;

			preHasDebugRoot[bvhIndex] = bvhOption.commonOption.HasDebugRoot();
			if (preHasDebugRoot[bvhIndex])
				preDebugRootGuid[bvhIndex] = bvhOption.commonOption.debugRoot->GetGuid();
			else
				preDebugRootGuid[bvhIndex] = 0;
		}
		void JSceneSpatialStructure::ActivatedOptionCash::SetKdTreeOption(const JKdTreeOption& kdTreeOption)
		{
			preKdTreeOption = kdTreeOption;
			preHasInnerRoot[kdTreeIndex] = kdTreeOption.commonOption.HasInnerRoot();
			if (preHasInnerRoot[kdTreeIndex])
				preInnerRootGuid[kdTreeIndex] = kdTreeOption.commonOption.innerRoot->GetGuid();
			else
				preInnerRootGuid[kdTreeIndex] = 0;

			preHasDebugRoot[kdTreeIndex] = kdTreeOption.commonOption.HasDebugRoot();
			if (preHasDebugRoot[kdTreeIndex])
				preDebugRootGuid[kdTreeIndex] = kdTreeOption.commonOption.debugRoot->GetGuid();
			else
				preDebugRootGuid[kdTreeIndex] = 0;
		}

		JSceneSpatialStructure::JSceneSpatialStructure() 
		{
			octree = std::make_unique<JOctree>();
			bvh = std::make_unique<JBvh>();
			kdTree = std::make_unique<JKdTree>();
			spaceSpatialVec.push_back(octree.get());
			spaceSpatialVec.push_back(bvh.get());
			spaceSpatialVec.push_back(kdTree.get());
		}
		JSceneSpatialStructure::~JSceneSpatialStructure(){}
		void JSceneSpatialStructure::Clear()noexcept
		{
			if (octree != nullptr)
			{
				octree->Clear();
				octree.reset();
			}
			if (bvh != nullptr)
			{
				bvh->Clear();
				bvh.reset();
			}
			if (kdTree != nullptr)
			{
				kdTree->Clear();
				kdTree.reset();
			}
			spaceSpatialVec.clear();
		}
		void JSceneSpatialStructure::Culling(const JCullingFrustum& camFrustum)noexcept
		{
			for (const auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated() && data->IsCullingActivated())
					data->Culling(camFrustum);
			}
		}
		void JSceneSpatialStructure::Culling(const DirectX::BoundingFrustum& camFrustum)noexcept
		{
			for (const auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated() && data->IsCullingActivated())
					data->Culling(camFrustum);
			}
		}
		void JSceneSpatialStructure::UpdateGameObject(JGameObject* gameObject)noexcept
		{
			for (auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated() && data->IsCullingActivated())
					data->UpdateGameObject(gameObject);
			}
		}
		void JSceneSpatialStructure::AddGameObject(JGameObject* gameObject)noexcept
		{
			for (auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated())
					data->AddGameObject(gameObject);
			}
		}
		void JSceneSpatialStructure::RemoveGameObject(JGameObject* gameObject)noexcept
		{
			if (gameObject == nullptr)
				return;

			for (auto& data : spaceSpatialVec)
			{
				if (data->IsSpaceSpatialActivated())
					data->RemoveGameObject(gameObject);
			}
		}
		std::vector<JGameObject*> JSceneSpatialStructure::GetAlignedObject(const DirectX::BoundingFrustum& camFrustum)const noexcept
		{
			if (activateTrigger)
			{
				if (kdTree->IsSpaceSpatialActivated())
					return kdTree->GetAlignedObject(camFrustum);
			}
			return std::vector<JGameObject*>();
		}
		JOctreeOption JSceneSpatialStructure::GetOctreeOption()const noexcept
		{
			return octree->GetOctreeOption();
		}
		JBvhOption JSceneSpatialStructure::GetBvhOption()const noexcept
		{
			return bvh->GetBvhOption();
		}
		JKdTreeOption JSceneSpatialStructure::GetKdTreeOption()const noexcept
		{
			return kdTree->GetKdTreeOption();
		}
		void JSceneSpatialStructure::SetOctreeOption(const JOctreeOption& option)
		{
			if (activateTrigger)
				octree->SetOctreeOption(option);
			else
				optionCash->SetOctreeOption(option);
		}
		void JSceneSpatialStructure::SetBvhOption(const JBvhOption& option)
		{
			if (activateTrigger)
				bvh->SetBvhOption(option);
			else
				optionCash->SetBvhOption(option);
		}
		void JSceneSpatialStructure::SetKdTreeOption(const JKdTreeOption& option)
		{
			if (activateTrigger)
				kdTree->SetKdTreeOption(option);
			else
				optionCash->SetKdTreeOption(option);
		}
		void JSceneSpatialStructure::SetInitTrigger(const bool value)noexcept
		{
			initTrigger = value;
		}
		bool JSceneSpatialStructure::IsInitTriggerActivated()const noexcept
		{
			return initTrigger;
		}
		void JSceneSpatialStructure::Activate()noexcept
		{
			if (!activateTrigger)
			{  
				octree->SetOctreeOption(optionCash->GetOctreeOption());
				bvh->SetBvhOption(optionCash->GetBvhOption());
				kdTree->SetKdTreeOption(optionCash->GetKdTreeOption());
				optionCash.reset();
				spaceSpatialVec.push_back(octree.get());
				spaceSpatialVec.push_back(bvh.get());
				spaceSpatialVec.push_back(kdTree.get());
				activateTrigger = true;
			}
		}
		void JSceneSpatialStructure::DeAcitvate()noexcept
		{
			if (activateTrigger)
			{
				optionCash = std::make_unique<ActivatedOptionCash>(octree->GetOctreeOption(), bvh->GetBvhOption(), kdTree->GetKdTreeOption());
				octree->Clear();
				bvh->Clear();
				kdTree->Clear();
				spaceSpatialVec.clear();
				activateTrigger = false;
			}
		}
		void JSceneSpatialStructure::BuildDebugTree(Core::J_SPACE_SPATIAL_TYPE type, Editor::JEditorBinaryTreeView& tree)noexcept
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
					bvh->BuildDebugTree(tree);
					break;
				} 
				case JinEngine::Core::J_SPACE_SPATIAL_TYPE::KD_TREE:
				{
					kdTree->BuildDebugTree(tree);
					break;
				}
				default:
					break;
				}
			}
		}
	}
}