#pragma once 
#include"../../JGraphicTaskInterface.h"  
#include"../../Thread/JGraphicMultiThreadDrawInterface.h"

namespace JinEngine
{
	///struct JGraphicShaderData; 
	namespace Graphic
	{
		struct JDrawHelper;
		class JOccCulling : public JGraphicTaskInterface, public JGraphicMultiThreadDrawInterface
		{     
		public:
			~JOccCulling() = default;
		public:
			//TestTest
			virtual void DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) = 0;
			virtual void DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) = 0;
			//void ComputeHZBOcclusionCulling(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper, const int dataIndex);
			//void ExtractHDOcclusionCullingData(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper, const int dataIndex);
		};
	}
}