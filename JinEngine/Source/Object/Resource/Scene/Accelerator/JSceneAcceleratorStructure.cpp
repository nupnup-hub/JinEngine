#include"JSceneAcceleratorStructure.h"
#include"JAcceleratorVisualizeInterface.h"
#include"Octree/JOctree.h"
#include"Bvh/JBvh.h"
#include"Kd-tree/JKdTree.h"
#include"../../../GameObject/JGameObject.h"
#include"../../../Component/RenderItem/JRenderItem.h"
#include"../../../Component/Light/JLight.h"
#include"../../../../Core/Log/JLogMacro.h"
using namespace DirectX;
namespace JinEngine
{

	namespace
	{
		static constexpr bool onlyBvhCanFrustumCulling = true;
		static constexpr bool restrictGpuAccLightShape = true;
	}

	JOctreeOption JSceneAcceleratorStructure::ActivatedOptionCash::GetOctreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		JOctreeOption option = preOctreeOption[(int)layer];
		LoadRoot(option.commonOption, J_ACCELERATOR_TYPE::OCTREE, layer);
		return option;
	}
	JBvhOption JSceneAcceleratorStructure::ActivatedOptionCash::GetBvhOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		JBvhOption option = preBvhOption[(int)layer];
		LoadRoot(option.commonOption, J_ACCELERATOR_TYPE::BVH, layer);
		return option;
	}
	JKdTreeOption JSceneAcceleratorStructure::ActivatedOptionCash::GetKdTreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		JKdTreeOption option = preKdTreeOption[(int)layer];
		LoadRoot(option.commonOption, J_ACCELERATOR_TYPE::KD_TREE, layer);
		return option;
	} 
	JGpuAcceleratorOption JSceneAcceleratorStructure::ActivatedOptionCash::GetGpuOption()const noexcept
	{
		return gpuBuildOption;
	}
	void JSceneAcceleratorStructure::ActivatedOptionCash::SetOctreeOption(const J_ACCELERATOR_LAYER layer, const JOctreeOption& octreeOption)
	{
		preOctreeOption[(uint)layer] = octreeOption;
		StoreRoot(octreeOption.commonOption, J_ACCELERATOR_TYPE::OCTREE, layer);
	}
	void JSceneAcceleratorStructure::ActivatedOptionCash::SetBvhOption(const J_ACCELERATOR_LAYER layer, const JBvhOption& bvhOption)
	{
		preBvhOption[(uint)layer] = bvhOption;
		StoreRoot(bvhOption.commonOption, J_ACCELERATOR_TYPE::BVH, layer);
	}
	void JSceneAcceleratorStructure::ActivatedOptionCash::SetKdTreeOption(const J_ACCELERATOR_LAYER layer, const JKdTreeOption& kdTreeOption)
	{
		preKdTreeOption[(uint)layer] = kdTreeOption;
		StoreRoot(kdTreeOption.commonOption, J_ACCELERATOR_TYPE::KD_TREE, layer);
	} 
	void JSceneAcceleratorStructure::ActivatedOptionCash::SetGpuOption(const JGpuAcceleratorOption& option)
	{
		gpuBuildOption = option;
	}
	void JSceneAcceleratorStructure::ActivatedOptionCash::LoadRoot(JAcceleratorOption& option, const J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer)const noexcept
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
	void JSceneAcceleratorStructure::ActivatedOptionCash::StoreRoot(const JAcceleratorOption& option, const J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer)noexcept
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

	JSceneAcceleratorStructure::JSceneAcceleratorStructure()
	{
		for (uint i = 0; i < (uint)J_ACCELERATOR_LAYER::COUNT; ++i)
		{
			octree[i] = std::make_unique<JOctree>((J_ACCELERATOR_LAYER)i);
			bvh[i] = std::make_unique<JBvh>((J_ACCELERATOR_LAYER)i);
			kdTree[i] = std::make_unique<JKdTree>((J_ACCELERATOR_LAYER)i);

			spaceSpatialVec.push_back(octree[i].get());
			spaceSpatialVec.push_back(bvh[i].get());
			spaceSpatialVec.push_back(kdTree[i].get());
		} 
	}
	JSceneAcceleratorStructure::~JSceneAcceleratorStructure() {}
	void JSceneAcceleratorStructure::Clear()noexcept
	{
		for (uint i = 0; i < (uint)J_ACCELERATOR_LAYER::COUNT; ++i)
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
	/*void JSceneAcceleratorStructure::Culling(const JCullingFrustum& camFrustum)noexcept
	{
		for (const auto& data : spaceSpatialVec)
		{
			if (data->IsAcceleratorActivated() && data->IsCullingActivated())
				data->Culling(camFrustum);
		}
	}*/
	void JSceneAcceleratorStructure::Culling(JAcceleratorCullingInfo& info)noexcept
	{
		//JFustum은 아직 미개발이므로 DirectX::BoundingFrustum 주로 사용한다
		info.useJFrustum = false;

		bool cullingOnece = false;
		for (const auto& data : spaceSpatialVec)
		{
			if (data->IsAcceleratorActivated() && data->IsCullingActivated())
			{
				data->Culling(info);
				cullingOnece = true;
			}
		}
		if (!cullingOnece && !info.useJFrustum)
			info.cullUser.OffCullingArray(Graphic::J_CULLING_TYPE::FRUSTUM, Graphic::J_CULLING_TARGET::RENDERITEM);
	}
	void JSceneAcceleratorStructure::Intersect(JAcceleratorIntersectInfo& info)const noexcept
	{ 
		for (const auto& data : spaceSpatialVec)
		{
			if (data->IsAcceleratorActivated() && data->GetLayer() == info.layer && info.spacitalType == data->GetType())
			{ 
				data->Intersect(info);
				return;
			}
		}
		if (info.findOtherAcceleratorIfTypeNull)
		{
			for (const auto& data : spaceSpatialVec)
			{
				if (data->IsAcceleratorActivated() && data->GetLayer() == info.layer && info.spacitalType != data->GetType())
				{
					data->Intersect(info);
					return;
				}
			}
		}
	}
	void JSceneAcceleratorStructure::Contain(JAcceleratorContainInfo& info)const noexcept
	{ 
		for (const auto& data : spaceSpatialVec)
		{
			if (data->IsAcceleratorActivated() && data->GetLayer() == info.layer && info.spacitalType == data->GetType())
			{ 
				data->Contain(info);
				break;
			}
		}
	}
	std::vector<JUserPtr<JGameObject>> JSceneAcceleratorStructure::AlignedObject(JAcceleratorAlignInfo& info)const noexcept
	{
		std::vector<JUserPtr<JGameObject>> result;
		int count;
		AlignedObjectF(info, result, count);
		return result;
	}
	void JSceneAcceleratorStructure::AlignedObjectF(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& validCount)const noexcept
	{ 
		for (const auto& data : spaceSpatialVec)
		{
			if (data->IsAcceleratorActivated() && data->GetLayer() == info.layer && data->GetType() == info.type)
			{ 
				data->AlignedObject(info, aligned, validCount);
				return;
			}
		}
	}
	void JSceneAcceleratorStructure::UpdateGameObject(const JUserPtr<JComponent>& comp)noexcept
	{ 
		const J_COMPONENT_TYPE compType = comp->GetComponentType();
		auto gameObject = comp->GetOwner();
		if ((compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM || compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM) && gameObject->HasRenderItem())
		{
			auto renderItem = gameObject->GetRenderItem();
			const J_ACCELERATOR_LAYER accLayer = ConvertAcceleratorLayer(renderItem->GetRenderLayer());
			for (auto& data : spaceSpatialVec)
			{
				if (data->IsAcceleratorActivated() && data->GetLayer() == accLayer)
					data->UpdateGameObject(gameObject);
			}
			if (gpuAccelerator != nullptr && accLayer == J_ACCELERATOR_LAYER::COMMON_OBJECT)
				gpuAccelerator->UpdateTransform(renderItem);
		}

		if (gpuAccelerator != nullptr && compType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			gpuAccelerator->UpdateTransform(comp);
	}
	void JSceneAcceleratorStructure::AddGameObject(const JUserPtr<JRenderItem>& rItem)noexcept
	{
		if (rItem->GetMesh() == nullptr)
			return;

		auto gameObject = rItem->GetOwner();
		const J_ACCELERATOR_LAYER accLayer = ConvertAcceleratorLayer(rItem->GetRenderLayer());
		for (auto& data : spaceSpatialVec)
		{
			if (data->IsAcceleratorActivated() && data->CanAddGameObject(gameObject) && data->GetLayer() == accLayer)
				data->AddGameObject(gameObject);
		}
		if (gpuAccelerator != nullptr && accLayer == J_ACCELERATOR_LAYER::COMMON_OBJECT)
			gpuAccelerator->AddComponent(rItem);
	}
	void JSceneAcceleratorStructure::AddGameObject(const JUserPtr<JLight>& light)noexcept
	{
		if (gpuAccelerator != nullptr && light->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			gpuAccelerator->AddComponent(light);
	}
	void JSceneAcceleratorStructure::RemoveGameObject(const JUserPtr<JRenderItem>& rItem)noexcept
	{ 
		auto gameObject = rItem->GetOwner();
		const J_ACCELERATOR_LAYER accLayer = ConvertAcceleratorLayer(gameObject->GetRenderItem()->GetRenderLayer());
		for (auto& data : spaceSpatialVec)
		{
			if (data->IsAcceleratorActivated())
				data->RemoveGameObject(gameObject);
		}
		if (gpuAccelerator != nullptr && accLayer == J_ACCELERATOR_LAYER::COMMON_OBJECT)
			gpuAccelerator->RemoveComponent(rItem);
	}
	void JSceneAcceleratorStructure::RemoveGameObject(const JUserPtr<JLight>& light)noexcept
	{
		if (gpuAccelerator != nullptr && light->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			gpuAccelerator->RemoveComponent(light);
	}
	JOctreeOption JSceneAcceleratorStructure::GetOctreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return octree[(uint)layer]->GetOctreeOption();
	}
	JBvhOption JSceneAcceleratorStructure::GetBvhOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return bvh[(uint)layer]->GetBvhOption();
	}
	JKdTreeOption JSceneAcceleratorStructure::GetKdTreeOption(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return kdTree[(uint)layer]->GetKdTreeOption();
	}
	Core::JBBox JSceneAcceleratorStructure::GetSceneBBox(const J_ACCELERATOR_LAYER layer, _Out_ bool& isValidBBox)const noexcept
	{
		//Preferred order
		//1. bvh
		//2. kd
		//3. octree 
		Core::JBBox result;
		if (bvh[(uint)layer]->IsAcceleratorActivated())
			result = bvh[(uint)layer]->GetSceneBBox();
		if (result.IsDistanceZero() && kdTree[(uint)layer]->IsAcceleratorActivated())
			result = kdTree[(uint)layer]->GetSceneBBox();
		if (result.IsDistanceZero() && octree[(uint)layer]->IsAcceleratorActivated())
			result = octree[(uint)layer]->GetSceneBBox();

		isValidBBox = !result.IsDistanceZero();
		return result;
	}
	const Graphic::JGpuAcceleratorUserInterface JSceneAcceleratorStructure::GpuAcceleratorUserInterface()const noexcept
	{
		return Graphic::JGpuAcceleratorUserInterface(gpuAccelerator.get());
	}
	void JSceneAcceleratorStructure::SetOctreeOption(const J_ACCELERATOR_LAYER layer, const JOctreeOption& option)
	{
		if (activateTrigger)
			octree[(uint)layer]->SetOctreeOption(option);
		else
			optionCash->SetOctreeOption(layer, option);
	}
	void JSceneAcceleratorStructure::SetBvhOption(const J_ACCELERATOR_LAYER layer, const JBvhOption& option)
	{
		if (activateTrigger)
			bvh[(uint)layer]->SetBvhOption(option);
		else
			optionCash->SetBvhOption(layer, option);
	}
	void JSceneAcceleratorStructure::SetKdTreeOption(const J_ACCELERATOR_LAYER layer, const JKdTreeOption& option)
	{
		if (activateTrigger)
			kdTree[(uint)layer]->SetKdTreeOption(option);
		else
			optionCash->SetKdTreeOption(layer, option);
	}
	void JSceneAcceleratorStructure::SetGpuAccelerator(JGpuAcceleratorOption option)
	{
		if (!JGpuAccelerator::CanBuild())
			return;

		if (gpuAccelerator == nullptr)
			gpuAccelerator = std::make_unique<JGpuAccelerator>();
 
		if constexpr (restrictGpuAccLightShape)
		{ 
			if (Core::HasSQValueEnum(option.flag, Graphic::J_GPU_ACCELERATOR_BUILD_OPTION_LIGHT_SHAPE))
				J_LOG_PRINT_OUT("Invalid data", "J_GPU_ACCELERATOR_BUILD_OPTION_LIGHT_SHAPE");
			option.flag = Core::MinusSQValueEnum(option.flag, Graphic::J_GPU_ACCELERATOR_BUILD_OPTION_LIGHT_SHAPE);
		}
		if (activateTrigger)
			gpuAccelerator->SetOption(option);
		else
			optionCash->SetGpuOption(option);
	}
	bool JSceneAcceleratorStructure::IsActivated(const J_ACCELERATOR_LAYER layer, const J_ACCELERATOR_TYPE type)
	{
		if (type == J_ACCELERATOR_TYPE::OCTREE)
			return octree[(uint)layer]->IsAcceleratorActivated();
		else if (type == J_ACCELERATOR_TYPE::BVH)
			return bvh[(uint)layer]->IsAcceleratorActivated();
		else
			return kdTree[(uint)layer]->IsAcceleratorActivated();
	} 
	bool JSceneAcceleratorStructure::HasCanCullingAccelerator(const J_ACCELERATOR_LAYER layer)const noexcept
	{
		return octree[(uint)layer]->IsAcceleratorActivated() && octree[(uint)layer]->IsCullingActivated() ||
			bvh[(uint)layer]->IsAcceleratorActivated() && bvh[(uint)layer]->IsCullingActivated() ||
			kdTree[(uint)layer]->IsAcceleratorActivated() && kdTree[(uint)layer]->IsCullingActivated();
	}
	void JSceneAcceleratorStructure::Activate()noexcept
	{
		if (!activateTrigger)
		{
			for (uint i = 0; i < (uint)J_ACCELERATOR_LAYER::COUNT; ++i)
			{
				octree[i]->SetOctreeOption(optionCash->GetOctreeOption((J_ACCELERATOR_LAYER)i));
				bvh[i]->SetBvhOption(optionCash->GetBvhOption((J_ACCELERATOR_LAYER)i));
				kdTree[i]->SetKdTreeOption(optionCash->GetKdTreeOption((J_ACCELERATOR_LAYER)i));

				spaceSpatialVec.push_back(octree[i].get());
				spaceSpatialVec.push_back(bvh[i].get());
				spaceSpatialVec.push_back(kdTree[i].get());
			}
			if (gpuAccelerator != nullptr)
				gpuAccelerator->SetOption(optionCash->GetGpuOption());

			optionCash.reset();
			activateTrigger = true;
		}
	}
	void JSceneAcceleratorStructure::DeAcitvate()noexcept
	{
		if (activateTrigger)
		{
			optionCash = std::make_unique<ActivatedOptionCash>();
			for (uint i = 0; i < (uint)J_ACCELERATOR_LAYER::COUNT; ++i)
			{
				optionCash->SetOctreeOption(octree[i]->GetLayer(), octree[i]->GetOctreeOption());
				optionCash->SetBvhOption(bvh[i]->GetLayer(), bvh[i]->GetBvhOption());
				optionCash->SetKdTreeOption(kdTree[i]->GetLayer(), kdTree[i]->GetKdTreeOption());

				octree[i]->Clear();
				bvh[i]->Clear();
				kdTree[i]->Clear();
			}
			if (gpuAccelerator != nullptr)
			{
				optionCash->SetGpuOption(gpuAccelerator->GetOption());
				gpuAccelerator->Clear();
			}
			spaceSpatialVec.clear();
			activateTrigger = false;
		}
	}
	void JSceneAcceleratorStructure::BuildDebugTree(const J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer, JAcceleratorVisualizeInterface* tree)noexcept
	{
		if (tree == nullptr || !tree->IsMatch(type))
			return;
 
		if (activateTrigger)
		{
			switch (type)
			{
			case JinEngine::J_ACCELERATOR_TYPE::OCTREE:
			{
				break;
			}
			case JinEngine::J_ACCELERATOR_TYPE::BVH:
			{
				bvh[(uint)layer]->BuildDebugTree(tree);
				break;
			}
			case JinEngine::J_ACCELERATOR_TYPE::KD_TREE:
			{
				kdTree[(uint)layer]->BuildDebugTree(tree);
				break;
			}
			default:
				break;
			}
		}
	}
	void JSceneAcceleratorStructure::RegisterInterfacePointer()
	{
		if (gpuAccelerator == nullptr)
			return;

		gpuAccelerator->RegisterInterfacePointer();
	}
}