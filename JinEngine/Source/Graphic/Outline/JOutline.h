#pragma once 
#include"JOutlineConstants.h"  
#include"../JGraphicTaskInterface.h"
#include<memory> 
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		struct JDrawHelper;
		class JOutline : public JGraphicTaskInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void UpdatePassBuf(const uint width, const uint height, const uint stencilRefOffset) = 0;
		public: 
			virtual void DrawCamOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper) = 0;
		};
	}
}