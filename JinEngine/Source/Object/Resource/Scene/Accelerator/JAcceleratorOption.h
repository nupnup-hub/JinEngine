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


#pragma once
#include"JAcceleratorType.h"
#include"../../../../Core/Pointer/JOwnerPtr.h" 
#include"../../../../Core/Geometry/JRay.h" 
#include"../../../../Core/Geometry/JGeometryEnum.h" 
#include"../../../../Core/JCoreEssential.h"
#include"../../../../Graphic/Culling/JCullingInterface.h"
#include"../../../../Graphic/Accelerator/JGpuAcceleratorType.h"
#include<fstream>
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject; 
	class JRenderItem;
	class JFileIOTool;
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
		void Store(JFileIOTool& tool);
		void Load(JFileIOTool& tool, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
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
		JVector3F pos;
	public:
		std::vector<JUserPtr<JGameObject>>* appAlignedObjVec;	//kd-tree has overlap
		int pushedCount = 0;
	public:
		bool hasCullingArea;
		//JFustum은 아직 미개발이므로 DirectX::BoundingFrustum 주로 사용한다
		bool useJFrustum = false;
		bool useBBox = false;
		bool allowPushVisibleObjVec = false;
		bool allowCullingOrderedByDistance = false;
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
		struct Result
		{
		public:
			JUserPtr<JGameObject> obj;
			float dist;
		};
	public:
		Core::JRay ray;
	public:
		J_ACCELERATOR_TYPE spacitalType = J_ACCELERATOR_TYPE::BVH;
		J_ACCELERATOR_LAYER layer;
		J_ACCELERATOR_SORT_TYPE sortType;
	public:  
		std::vector<Result> result;					// out data 
	public:
		//std::vector<Intermediate> intermediate;	//unuse
	public:
		bool allowContainRayPos = true;   
		bool untilFirst = false;
		bool findOtherAcceleratorIfTypeNull = false;
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
		void SortResult();
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

	struct JGpuAcceleratorOption
	{
	public:
		JUserPtr<JGameObject> root;
		J_ACCELERATOR_LAYER layer = J_ACCELERATOR_LAYER::COMMON_OBJECT;
		Graphic::J_GPU_ACCELERATOR_BUILD_OPTION flag = Graphic::J_GPU_ACCELERATOR_BUILD_OPTION_NONE;
	};
}