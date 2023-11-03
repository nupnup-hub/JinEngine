#include"JAcceleratorOption.h"
#include"../../../GameObject/JGameObject.h"  
#include"../../../Component/RenderItem/JRenderItem.h"   
#include"../../../JObjectFileIOHelper.h"

namespace JinEngine
{

	namespace Private
	{
		void EraseOverlap(std::vector<JUserPtr<JGameObject>>& vec)
		{
			std::vector<JUserPtr<JGameObject>> finalResult;
			const uint innerCount = (uint)vec.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				bool isOverlap = false;
				const size_t guid = vec[i]->GetGuid();
				const uint resCount = (uint)finalResult.size();
				for (uint j = 0; j < resCount; ++j)
				{
					if (finalResult[j]->GetGuid() == guid)
					{
						isOverlap = true;
						break;
					}
				}
				if (!isOverlap)
					finalResult.push_back(vec[i]);
			}
			vec = std::move(finalResult);
		}
		void EraseOverlap(std::vector<JAcceleratorIntersectInfo::Result>& vec)
		{
			std::vector<JAcceleratorIntersectInfo::Result> finalResult;
			const uint innerCount = (uint)vec.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				bool isOverlap = false;
				const size_t guid = vec[i].obj->GetGuid();
				const uint resCount = (uint)finalResult.size();
				for (uint j = 0; j < resCount; ++j)
				{
					if (finalResult[j].obj->GetGuid() == guid)
					{
						isOverlap = true;
						break;
					}
				}
				if (!isOverlap)
					finalResult.push_back(vec[i]);
			}
			vec = std::move(finalResult);
		}
	}
	JAcceleratorOption::JAcceleratorOption(JUserPtr<JGameObject> innerRoot,
		JUserPtr<JGameObject> debugRoot,
		bool isAcceleratorActivated,
		bool isDebugActivated,
		bool isDebugLeafOnly,
		bool isCullingActivated)
		:innerRoot(innerRoot),
		debugRoot(debugRoot),
		isAcceleratorActivated(isAcceleratorActivated),
		isDebugActivated(isDebugActivated),
		isDebugLeafOnly(isDebugLeafOnly),
		isCullingActivated(isCullingActivated)
	{}
	bool JAcceleratorOption::Equal(const JAcceleratorOption& tar)const noexcept
	{
		return isAcceleratorActivated == tar.isAcceleratorActivated &&
			isDebugActivated == tar.isDebugActivated &&
			isDebugLeafOnly == tar.isDebugLeafOnly &&
			isCullingActivated == tar.isCullingActivated &&
			SameInnerRoot(tar) && SameDebugRoot(tar);
	}
	bool JAcceleratorOption::SameInnerRoot(const JAcceleratorOption& tar)const noexcept
	{
		bool isSameInnerRoot = false;
		bool hasInnerRoot = innerRoot != nullptr && tar.innerRoot != nullptr;
		if (hasInnerRoot)
		{
			if (innerRoot->GetGuid() == tar.innerRoot->GetGuid())
				isSameInnerRoot = true;
		}
		else
		{
			if (innerRoot == nullptr && tar.innerRoot == nullptr)
				isSameInnerRoot = true;
		}
		return isSameInnerRoot;
	}
	bool JAcceleratorOption::SameDebugRoot(const JAcceleratorOption& tar)const noexcept
	{
		bool isSameDebugRoot = false;
		bool hasDebugRoot = debugRoot != nullptr && tar.debugRoot != nullptr;
		if (hasDebugRoot)
		{
			if (debugRoot->GetGuid() == tar.debugRoot->GetGuid())
				isSameDebugRoot = true;
		}
		else
		{
			if (debugRoot == nullptr && tar.debugRoot == nullptr)
				isSameDebugRoot = true;
		}
		return isSameDebugRoot;
	}
	bool JAcceleratorOption::HasInnerRoot()const noexcept
	{
		return innerRoot != nullptr && innerRoot.IsValid();
	}
	bool JAcceleratorOption::HasDebugRoot()const noexcept
	{
		return debugRoot != nullptr && debugRoot.IsValid();
	}
	void JAcceleratorOption::Store(JFileIOTool& tool)
	{
		if (!tool.CanStore())
			return;

		JObjectFileIOHelper::StoreAtomicData(tool, isAcceleratorActivated, "IsAcceleratorActivated:");
		JObjectFileIOHelper::StoreAtomicData(tool, isDebugActivated, "IsDebugActivated:");
		JObjectFileIOHelper::StoreAtomicData(tool, isDebugLeafOnly, "IsDebugLeafOnly:");
		JObjectFileIOHelper::StoreAtomicData(tool, isCullingActivated, "IsCullingActivated:");
		if (innerRoot.IsValid())
		{
			JObjectFileIOHelper::StoreAtomicData(tool, true, "hasInnerRoot:");
			JObjectFileIOHelper::StoreAtomicData(tool, innerRoot->GetGuid(), "innerGuid:");
		}
		else
		{
			JObjectFileIOHelper::StoreAtomicData(tool, false, "hasInnerRoot:");
			JObjectFileIOHelper::StoreAtomicData(tool, 0, "innerGuid:");
		}
	}
	void JAcceleratorOption::Load(JFileIOTool& tool, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
	{
		if (!tool.CanLoad())
			return;

		JObjectFileIOHelper::LoadAtomicData(tool, isAcceleratorActivated, "IsAcceleratorActivated:");
		JObjectFileIOHelper::LoadAtomicData(tool, isDebugActivated, "IsDebugActivated:");
		JObjectFileIOHelper::LoadAtomicData(tool, isDebugLeafOnly, "IsDebugLeafOnly:");
		JObjectFileIOHelper::LoadAtomicData(tool, isCullingActivated, "IsCullingActivated:");
		JObjectFileIOHelper::LoadAtomicData(tool, hasInnerRoot, "hasInnerRoot:");
		JObjectFileIOHelper::LoadAtomicData(tool, innerRootGuid, "innerGuid:");
	}

	JAcceleratorCullingInfo::JAcceleratorCullingInfo(const Graphic::JCullingUserInterface& cullUser,
		DirectX::BoundingFrustum frustum)
		:cullUser(cullUser),
		frustum(frustum),
		hasCullingArea(false),
		useJFrustum(false),
		useBBox(false)
	{}
	JAcceleratorCullingInfo::JAcceleratorCullingInfo(const Graphic::JCullingUserInterface& cullUser,
		DirectX::BoundingFrustum camFrustum,
		DirectX::BoundingFrustum cullingFrustum)
		:cullUser(cullUser),
		frustum(frustum),
		cullingFrustum(cullingFrustum),
		hasCullingArea(true),
		useJFrustum(false),
		useBBox(false)
	{}
	JAcceleratorCullingInfo::JAcceleratorCullingInfo(const Graphic::JCullingUserInterface& cullUser,
		const DirectX::BoundingBox bbox)
		:cullUser(cullUser),
		bbox(bbox),
		hasCullingArea(false),
		useJFrustum(false),
		useBBox(true)
	{}

	JAcceleratorAlignInfo::JAcceleratorAlignInfo(const J_ACCELERATOR_LAYER layer,
		const J_ACCELERATOR_TYPE type,
		const DirectX::BoundingFrustum& frustum,
		const bool hasCullingArea)
		:layer(layer), 
		type(type),
		hasCullingArea(hasCullingArea), 
		pos(frustum.Origin),
		//r(frustum.Orientation.x, frustum.Orientation.y, frustum.Orientation.z),
		tool(ALIGN_TOOL::FRUSTUM)
	{
		if (hasCullingArea)
		{
			cullingFrustum = frustum;
			cullingFrustum.Near = 0;
			cullingFrustum.Far = frustum.Near;
		}
	//	if (hasCullingRad)
	//		cullingRad = frustum.RightSlope > frustum.TopSlope ? frustum.RightSlope : frustum.TopSlope;
	}
	JAcceleratorAlignInfo::JAcceleratorAlignInfo(const J_ACCELERATOR_LAYER layer, const J_ACCELERATOR_TYPE type, const JVector3<float>& pos)
		: layer(layer),
		type(type),
		pos(pos),
	//	dir(dir),
		hasCullingArea(false),
		tool(ALIGN_TOOL::POINT)
	{

	}
	float JAcceleratorAlignInfo::AlignRangeRate()const noexcept
	{
		switch (alignRange)
		{
		case JinEngine::JAcceleratorAlignInfo::ALIGN_RANGE::ALL:
			return 100.0f;	//aligned all node ... 밸런스 이진트리에 가까운 트리이므로 1.0f이 아닌 큰값을 줘서 전부 탐색하게한다.
		case JinEngine::JAcceleratorAlignInfo::ALIGN_RANGE::THREE_QURTERS_DEPTH:
			return 0.75f;
		case JinEngine::JAcceleratorAlignInfo::ALIGN_RANGE::HALF_DEPTH:
			return 0.5f;
		case JinEngine::JAcceleratorAlignInfo::ALIGN_RANGE::QUARTER_DEPTH:
			return 0.25f;
		default:
			break;
		}
	}

	bool JAcceleratorIntersectInfo::Intermediate::CompareAsc(const Intermediate& a, const Intermediate& b)
	{
		return a.dist < b.dist;
	}
	bool JAcceleratorIntersectInfo::Intermediate::CompareDesc(const Intermediate& a, const Intermediate& b)
	{
		return a.dist > b.dist;
	}

	JAcceleratorIntersectInfo::JAcceleratorIntersectInfo(const Core::JRay& ray,
		const J_ACCELERATOR_LAYER layer,
		const J_ACCELERATOR_SORT_TYPE sortType,
		const bool allowContainRayPos,
		const bool untilFirst)
		:ray(ray),
		layer(layer),
		sortType(sortType),
		allowContainRayPos(allowContainRayPos),
		untilFirst(untilFirst)
	{}
	bool JAcceleratorIntersectInfo::CanAdd(const JUserPtr<JGameObject>& obj)
	{
		if (obj == nullptr)
			return false;

		return CanAdd(obj->GetRenderItem()->GetOrientedBoundingBox());
	}
	bool JAcceleratorIntersectInfo::CanAdd(const DirectX::BoundingOrientedBox& bbox)
	{
		return allowContainRayPos || bbox.Contains(ray.PosV()) != DirectX::CONTAINS;
	}
	void JAcceleratorIntersectInfo::EraseOverlap()
	{
		Private::EraseOverlap(result);
	}
	void JAcceleratorIntersectInfo::SortResult()
	{
		auto sortAsecLam = [](const Result& a, const Result& b)
		{
			return a.dist < b.dist;
		};
		auto sortDescLam = [](const Result& a, const Result& b)
		{
			return a.dist > b.dist;
		};
		if(sortType == J_ACCELERATOR_SORT_TYPE::ASCENDING)
			std::sort(result.begin(), result.end(), sortAsecLam);
		else if (sortType == J_ACCELERATOR_SORT_TYPE::DESCENDING)
			std::sort(result.begin(), result.end(), sortDescLam);
	}

	JAcceleratorContainInfo::JAcceleratorContainInfo(const DirectX::BoundingFrustum& frustum,
		const J_ACCELERATOR_LAYER layer,
		const bool allowIntersect)
		: frustum(frustum),
		layer(layer),
		toolType(CONTAIN_TOOL::FRUSTUM),
		allowIntersect(allowIntersect)
	{}
	JAcceleratorContainInfo::JAcceleratorContainInfo(const DirectX::BoundingOrientedBox& obb,
		const J_ACCELERATOR_LAYER layer,
		const bool allowIntersect)
		: obb(obb),
		layer(layer),
		toolType(CONTAIN_TOOL::OBB),
		allowIntersect(allowIntersect)
	{}
	void JAcceleratorContainInfo::EraseOverlap()
	{
		Private::EraseOverlap(resultObjVec);
	}
}
