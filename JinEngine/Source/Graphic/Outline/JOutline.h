#pragma once 
#include"JOutlineConstants.h"  
#include"../JGraphicTaskInterface.h"
#include"../JGraphicSubClassInterface.h" 
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		class JDrawHelper;
		class JOutline : public JGraphicTaskInterface, public JGraphicSubClassInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
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