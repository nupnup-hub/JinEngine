#pragma once 
#include"../JGraphicTaskInterface.h" 
#include"../JGraphicSubClassInterface.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		class JGraphicResourceManager;
		class JGraphicResourceInfo;

		class JShareDataHolderInterface : public JGraphicTaskInterface
		{ 
		public:
			virtual void UpdateBegin() = 0;
			virtual void UpdateEnd() = 0;
		};

		//shared data by shader pass
		//ex) ssao, imageProcessing(bloom, toneMapping, aa...)
		//share data is created by use resource creation
		//ex) creation ssao -> try create related resource(interleave, depth, intermediate ...)
		class JGraphicResourceShareData : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{
		public: 
			virtual void Clear() = 0;
		public:  
			virtual JShareDataHolderInterface* GetResourceDependencyData(const J_GRAPHIC_TASK_TYPE taskType, JGraphicResourceInfo* info) = 0;
		public:
			virtual void NotifyGraphicResourceCreation(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* newInfo) = 0;
			virtual void NotifyGraphicResourceDestruction(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* info) = 0;
		};
	}
}