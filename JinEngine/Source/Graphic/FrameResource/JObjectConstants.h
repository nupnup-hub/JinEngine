#pragma once  
#include"JOcclusionConstants.h"  
#include"JFrameConstantsSet.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace ObjectFrameLayer
		{
			static constexpr uint object = 0; 
			static constexpr uint bounding = object + 1;
			static constexpr uint hzb = bounding + 1;
			static constexpr uint refInfo = hzb + 1;
			static constexpr uint setCount = refInfo + 1;
		}

		struct JObjectConstants
		{
			JMatrix4x4 world = JMatrix4x4::Identity();
			JMatrix4x4 texTransform = JMatrix4x4::Identity();
			uint materialIndex = 0;
			uint objPad0 = 0;
			uint objPad1 = 0;
			uint objPad2 = 0;
		};

		struct JBoundingObjectConstants
		{
			JMatrix4x4 boundWorld = JMatrix4x4::Identity();
		};

		struct JObjectRefereneceInfoConstants
		{
			uint materialIndex = 0;
			uint verticesIndex = 0;					//heap srv index
			uint indicesIndex = 0;					//heap srv index
			uint verticesOffset = 0;				//submesh vertex start
			uint indicesOffset = 0;					//submesh index start
			uint verticesType = 0;					//static = 0, skinned = 1
			uint indicesType = 0;					//uint16 = 0, uint32 = 1
			uint pad02 = 0;
		};

		struct JObjectConstantsSet : public JFrameConstantsSet<ObjectFrameLayer::setCount>
		{ 
		public:
			std::vector<JObjectConstants> object;
			JBoundingObjectConstants bounding;
			JHzbOccObjectConstants hzb;
			std::vector<JObjectRefereneceInfoConstants> refInfo;
		public:
			uint subMeshCount = 0;
			bool hasCopy = false;
		public:
			void SetUpdated(const uint layerIndex, const uint _frameIndex)
			{ 
				frameIndex[layerIndex] = _frameIndex;
				isUpdated[layerIndex] = true;
			}
		};	 
	}
}