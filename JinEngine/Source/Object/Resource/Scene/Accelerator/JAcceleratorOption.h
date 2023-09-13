#pragma once
#include"JAcceleratorType.h"
#include"../../../../Core/Pointer/JOwnerPtr.h"
#include"../../../../Core/Geometry/JCullingFrustum.h"
#include"../../../../Core/Geometry/JRay.h" 
#include"../../../../Core/JCoreEssential.h"
#include"../../../../Graphic/Culling/JCullingInterface.h"
#include<fstream>
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject; 
	class JRenderItem;
	class JAcceleratorOption
	{
	public:
		JUserPtr<JGameObject> innerRoot = nullptr;
		JUserPtr<JGameObject> debugRoot = nullptr;
	public:
		bool isAcceleratorActivated = false;
		bool isDebugActivated = false;
		bool isDebugLeafOnly = true;
		bool isCullingActivated = false;
	public:
		JAcceleratorOption() = default;
		JAcceleratorOption(JUserPtr<JGameObject> innerRoot,
			JUserPtr<JGameObject> debugRoot,
			bool isAcceleratorActivated,
			bool isDebugActivated,
			bool isDebugLeafOnly,
			bool isCullingActivated);
	public:
		bool Equal(const JAcceleratorOption& tar)const noexcept;
		bool SameInnerRoot(const JAcceleratorOption& tar)const noexcept;
		bool SameDebugRoot(const JAcceleratorOption& tar)const noexcept;
	public:
		bool HasInnerRoot()const noexcept;
		bool HasDebugRoot()const noexcept;
	public:
		void Store(std::wofstream& stream);
		void Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
	};
	struct JAcceleratorCullingInfo
	{
	public:
		Graphic::JCullingUserInterface cullUser;
		DirectX::BoundingFrustum frustum;
		DirectX::BoundingFrustum cullingFrustum;
	public:
		DirectX::BoundingBox bbox;
	public:
		Core::JCullingFrustum jFrustum;
		Core::JCullingFrustum jCullingFrustum;
	public:
		bool hasCullingArea;
		//JFustum은 아직 미흡한 부분이있으므로 DirectX::BoundingFrustum 주로 사용한다
		bool useJFrustum;
		bool useBBox = false;
	public:
		JAcceleratorCullingInfo(const Graphic::JCullingUserInterface& cullUser,
			DirectX::BoundingFrustum camFrustum);
		JAcceleratorCullingInfo(const Graphic::JCullingUserInterface& cullUser,
			DirectX::BoundingFrustum camFrustum,
			DirectX::BoundingFrustum cullingFrustum);
		JAcceleratorCullingInfo(const Graphic::JCullingUserInterface& cullUser,
			const DirectX::BoundingBox bbox);
	};
	struct JAcceleratorAlignInfo
	{
	public:
		enum class ALIGN_RANGE
		{
			ALL,	
			THREE_QURTERS_DEPTH,
			HALF_DEPTH,
			QUARTER_DEPTH
		};
		enum class ALIGN_TOOL
		{
			FRUSTUM,
			POINT
		};
	public:
		using AlignPassCondPtr = bool(*)(JGameObject* obj);
	public:
		AlignPassCondPtr alignPassCondPtr = nullptr;
	public:
		J_ACCELERATOR_LAYER layer;
		J_ACCELERATOR_TYPE type = J_ACCELERATOR_TYPE::BVH;
	public: 
		DirectX::BoundingFrustum cullingFrustum;
	public: 
		JVector3<float> pos = JVector3<float>(0, 0, 0); 
	public:
		ALIGN_RANGE alignRange = ALIGN_RANGE::ALL;
		ALIGN_TOOL tool;
	public:
		int acceleratorMaxDepth = 0;		//_Out_ defined accelerator class
		int alignMaxDepth = 0;				//_Out_ defined accelerator class
	public:
		bool hasCullingArea = false; 
		//condition 추가 필요
	public:
		JAcceleratorAlignInfo() = default; 
		JAcceleratorAlignInfo(const J_ACCELERATOR_LAYER layer,
			const J_ACCELERATOR_TYPE type,
			const DirectX::BoundingFrustum& frustum,
			const bool hasCullingArea = false);
		JAcceleratorAlignInfo(const J_ACCELERATOR_LAYER layer, const J_ACCELERATOR_TYPE type, const JVector3<float>& pos);
	public:
		float AlignRangeRate()const noexcept;
	};
	struct JAcceleratorIntersectInfo
	{
	public:
		struct Intermediate
		{
		public:
			int index;
			float dist;
			bool isIntersect;
		public:
			static bool CompareAsc(const Intermediate& a, const Intermediate& b);
			static bool CompareDesc(const Intermediate& a, const Intermediate& b);
		};
	public:
		Core::JRay ray;
	public:
		J_ACCELERATOR_TYPE spacitalType = J_ACCELERATOR_TYPE::BVH;
		J_ACCELERATOR_LAYER layer;
		J_ACCELERATOR_SORT_TYPE sortType;
	public:
		//untilFirst trigger가 false일때 유효한 값을 가짐
		std::vector<JUserPtr<JGameObject>> resultObjVec;		// out data
		//untilFirst trigger가 true일때 유효한 값을 가짐
		JUserPtr<JGameObject> firstResult;			// out data
	public:
		std::vector<Intermediate> intermediate;
	public:
		bool allowContainRayPos = true;
		bool untilFirst = false;
		bool findOtherAcceleratorIfTypeNull = true;
	public:
		JAcceleratorIntersectInfo(const Core::JRay& ray,
			const J_ACCELERATOR_LAYER layer,
			const J_ACCELERATOR_SORT_TYPE sortType,
			const bool allowContainRayPos,
			const bool untilFirst);
	public:
		/**
		* @brief check object validity
		*/
		bool CanAdd(const JUserPtr<JGameObject>& obj);
		bool CanAdd(const DirectX::BoundingOrientedBox& bbox);
	public:
		void EraseOverlap();
	public:
		void SortIntermediate(const uint count, const bool isAscending);
	};
	struct JAcceleratorContainInfo
	{
	public:
		enum class CONTAIN_TOOL
		{
			FRUSTUM,
			OBB,
		};
	public:
		J_ACCELERATOR_TYPE spacitalType = J_ACCELERATOR_TYPE::BVH;
		J_ACCELERATOR_LAYER layer;
		//minXY maxXY is always ASCENDING 
		CONTAIN_TOOL toolType;
	public:
		DirectX::BoundingFrustum frustum;
	public:
		DirectX::BoundingOrientedBox obb;
	public:
		bool allowIntersect = true;
		bool findOtherAcceleratorIfTypeNull = true;
	public:
		std::vector<JUserPtr<JGameObject>> resultObjVec;		// out data
	public:
		JAcceleratorContainInfo(const DirectX::BoundingFrustum& frustum,
			const J_ACCELERATOR_LAYER layer,
			const bool allowIntersect = true);
		JAcceleratorContainInfo(const DirectX::BoundingOrientedBox& obb,
			const J_ACCELERATOR_LAYER layer,
			const bool allowIntersect = true);
	public:
		void EraseOverlap();
	};
}