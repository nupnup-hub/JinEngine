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

		/**
		* Caution!
		* Constants buffer는 256 byte단위로 Gpu에 upload되며
		* 이는 Cpu Constants structure size와 다를수있으므로
		* 복수의 data를 CopyData할시에는 Gpu에 upload되는 크기와 같은
		* structure을 할당하거나(권장), 하나씩 CopyData를 해야한다(비권장 너무느림)
		*/
		//144(real data) + 112(empty)
		struct JObjectCpuConstants
		{
			JMatrix4x4 world = JMatrix4x4::Identity();
			JMatrix4x4 texTransform = JMatrix4x4::Identity();
			uint materialIndex = 0;
			uint objPad00 = 0;
			uint objPad01 = 0;
			uint objPad02 = 0;
			uint objPad03 = 0;
			uint objPad04 = 0;
			uint objPad05 = 0;
			uint objPad06 = 0;
			uint objPad07 = 0;
			uint objPad08 = 0;
			uint objPad09 = 0;
			uint objPad10 = 0;
			uint objPad11 = 0;
			uint objPad12 = 0;
			uint objPad13 = 0;
			uint objPad14 = 0;
			uint objPad15 = 0;
			uint objPad16 = 0;
			uint objPad17 = 0;
			uint objPad18 = 0;
			uint objPad19 = 0;
			uint objPad20 = 0;
			uint objPad21 = 0;
			uint objPad22 = 0;
			uint objPad23 = 0;
			uint objPad24 = 0;
			uint objPad25 = 0;
			uint objPad26 = 0;
			uint objPad27 = 0; 
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

			uint pad00 = 0;
		};
 
		struct JObjectConstantsSet : public JFrameConstantsSet<ObjectFrameLayer::setCount>
		{ 
		public:
			//UploadConstants
			std::vector<JObjectCpuConstants> object;
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