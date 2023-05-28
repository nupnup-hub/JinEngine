#pragma once 
#include"GraphicResource/JGraphicResourceType.h"
#include"Upload/JUploadType.h"
#include"Culling/JCullingInterface.h"
#include"../Core/Func/Callable/JCallable.h"
#include"../Application/JApplicationEngine.h"
#include<vector>
#include<memory>

namespace JinEngine
{
	class JScene;
	class JComponent;
	class JCamera;
	class JLight; 
	namespace Graphic
	{
		struct JGraphicInfo;
		struct JGraphicOption;
		class JGraphicDrawTarget;
		class JGraphic;
		struct JUpdateHelper
		{
		public:
			using GetElementCountT = Core::JStaticCallableType<uint>;
			using GetElementCapacityT = Core::JStaticCallableType<uint>;
			using ReBuildUploadDataT = Core::JStaticCallableType<uint>;
			using NotifyUpdateCapacityT = Core::JStaticCallableType<void>;
		public:
			using SetCapacityT = Core::JStaticCallableType<void>;
		public:
			struct UploadUpdateData
			{
			public:
				std::unique_ptr<GetElementCountT::Callable> getElement = nullptr;
				std::vector<std::unique_ptr<NotifyUpdateCapacityT::Callable>> notifyUpdateCapacity;
			public:
				uint count = 0;
				uint capacity = 0;
				uint offset = 0;
				uint setDirty = 0;
				J_UPLOAD_CAPACITY_CONDITION rebuildCondition;
			};
			struct BindingTextureData
			{
			public:
				std::unique_ptr<GetElementCountT::Callable> getTextureCount = nullptr;
				std::unique_ptr<GetElementCapacityT::Callable> getTextureCapacity = nullptr;
				std::unique_ptr<SetCapacityT::Callable> setCapacity = nullptr;
			public:
				uint count = 0;
				uint capacity = 0;
				J_UPLOAD_CAPACITY_CONDITION recompileCondition;
				bool hasCallable = false;
			public:
				bool HasCallable()const noexcept;
			};
		public:
			std::vector<UploadUpdateData> uData;	//frame upload resource 
			std::vector<BindingTextureData> bData;
			bool hasRebuildCondition;
			bool hasRecompileShader;
		public:
			void Clear();
			void RegisterCallable(J_UPLOAD_FRAME_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr);
			void RegisterCallable(J_GRAPHIC_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr, GetElementCapacityT::Ptr* getCapaPtr, SetCapacityT::Ptr* sPtr);
			void RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE type, std::unique_ptr<NotifyUpdateCapacityT::Callable>&& listner);
			void WriteGraphicInfo(JGraphicInfo& info)const noexcept;
			void NotifyUpdateFrameCapacity(JGraphic& grpahic);
		};
		//draw data
		struct JDrawHelper
		{
		public:
			JGraphicDrawTarget* drawTarget = nullptr;
			JCullingUserInterface cullUser;
		public:
			JUserPtr<JScene> scene = nullptr;
			JUserPtr<JCamera> cam = nullptr;
			JUserPtr<JLight> lit = nullptr;
		public:
			uint passOffset = 0;
		public:
			bool allowDrawDepthMap = false;
			//draw outline and debug layer object
			bool allowDrawDebug = false;
			bool allowFrustumCulling = true;
			bool allowOcclusionCulling = true;
			bool allowDrawOccMipMap = false;	//for debug
		public:
			bool RefelectOtherCamCullig(const uint rItemIndex)const noexcept;
		public: 
			void SettingOccCulling(const JUserPtr<JComponent>& comp)noexcept;
			void SettingDrawShadowMap(const JUserPtr<JLight>& lit)noexcept;
			void SettingDrawScene(const JUserPtr<JCamera>& cam)noexcept;
		};
		//draw detail condition
		struct JDrawCondition
		{
		public:
			bool allowAnimation = false;
		public: 
			bool allowCulling = false;
			bool allowHzbOcclusionCulling = false;
			bool allowHDOcclusionCulling = false;
		public:
			bool allowDebugOutline = false;
			bool allowAllCullingResult = false;	//for check other cam spacespatial 
		public:
			JDrawCondition() = default;
			JDrawCondition(const JGraphicOption& option,
				const JDrawHelper& helper,
				const bool newAllowAnimation,
				const bool newAllowCulling,
				const bool newAllowDebugOutline);
		}; 
	}
}