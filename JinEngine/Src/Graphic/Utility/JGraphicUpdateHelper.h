#pragma once
#include"../JGraphicInfo.h"
#include"../Upload/JUploadType.h"
#include"../GraphicResource/JGraphicResourceType.h"
#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphic;
		struct JGraphicUpdateHelper
		{
		public:
			using GetElementCountT = Core::JStaticCallableType<uint>;
			using NotifyUpdateCapacityT = Core::JStaticCallableType<void, JGraphic&>;
		public:
			using GetTextureCountT = Core::JStaticCallableType<uint, const JGraphic&>;
			using GetTextureCapacityT = Core::JStaticCallableType<uint, const JGraphic&>;
			using SetCapacityT = Core::JStaticCallableType<void, JGraphic&>;
		public:
			struct UploadData
			{
			public:
				std::unique_ptr<GetElementCountT::Callable> getElementCountCallable = nullptr; 
				std::vector<std::unique_ptr<NotifyUpdateCapacityT::Callable>> notifyUpdateCapacityCallable;
			public:
				uint count = 0;
				uint capacity = 0;
				uint offset = 0;
				bool setFrameDirty = false;
				J_UPLOAD_CAPACITY_CONDITION rebuildCondition;
			};
			struct BindingTextureData
			{
			public:
				std::unique_ptr<GetTextureCountT::Callable> getTextureCountCallable = nullptr;
				std::unique_ptr<GetTextureCapacityT::Callable> getTextureCapacityCallable = nullptr;
				std::unique_ptr< SetCapacityT::Callable> setCapacityCallable = nullptr;
			public:
				uint count = 0;
				uint capacity = 0;
				J_UPLOAD_CAPACITY_CONDITION recompileCondition;
				bool hasCallable = false;
			public:
				bool HasCallable()const noexcept;
			};
		public:
			std::vector<UploadData> uData;
			std::vector<BindingTextureData> bData;
			bool hasRebuildCondition;
			bool hasRecompileShader;
		public:
			void RegisterCallable(J_UPLOAD_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr);
			void RegisterCallable(J_GRAPHIC_RESOURCE_TYPE type, GetTextureCountT::Ptr* getCountPtr, GetTextureCapacityT::Ptr* getCapaPtr, SetCapacityT::Ptr* sPtr);
			void RegisterListener(J_UPLOAD_RESOURCE_TYPE type, std::unique_ptr<NotifyUpdateCapacityT::Callable>&& listner);
			void WriteGraphicInfo(JGraphicInfo& info)const noexcept;
			void NotifyUpdateFrameCapacity(JGraphic& grpahic);
			void Clear();
		};

	}
}