#pragma once 
#include"JOutlineConstants.h"  
#include"../DataSet/JGraphicDataSet.h"
#include"../Device/JGraphicDeviceUser.h"
#include<memory> 
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		struct JDrawHelper;
		class JOutline : public JGraphicDeviceUser
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0;
		public:
			virtual void UpdatePassBuf(const uint width, const uint height, const uint stencilRefOffset) = 0;
		public: 
			virtual void DrawCamOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper) = 0;
			virtual void DrawOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper) = 0;
		};
	}
}