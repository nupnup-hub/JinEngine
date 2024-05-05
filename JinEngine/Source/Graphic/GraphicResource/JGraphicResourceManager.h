#pragma once 
#include"JGraphicResourceType.h" 
#include"JGraphicResourceConstants.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../JGraphicConstants.h"
#include"../JGraphicSubClassInterface.h"
#include"../Device/JGraphicDeviceUser.h"  
#include"../../Core/Pointer/JOwnerPtr.h" 
#include"../../Core/Geometry/Mesh/JMeshStruct.h"
#include"../../Core/Handle/JDataHandleStructure.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JGraphicResourceWatcher;
	}
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceInfo;  
		class JGraphicResourceUserAccess;

		//multi puerpose bind
		struct MPBInfo
		{
		public:
			JUserPtr<JGraphicResourceInfo> info; 
			int index[(uint)J_GRAPHIC_BIND_TYPE::COUNT] = { invalidIndex, invalidIndex ,invalidIndex ,invalidIndex};
		};
		//Manage graphic texture or buffer 
		class JGraphicResourceManager : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JGraphicResourceManager)
		protected:
			struct ResourceViewInfo
			{
			public:
				int count = 0;
				int capacity = 0;
				int offset = 0;
			public:
				bool canBind = false;
				bool isFixedBind = false;
			public:
				void ClearCount();
				void ClearAllData();
			public:
				int GetNextViewIndex()const noexcept;
			public:
				bool HasSpace()const noexcept;
			};
			struct ResourceTypeDesc
			{
			public:
				int count = 0; 
				int capacity = 0; 
			public:
				ResourceViewInfo viewInfo[(int)J_GRAPHIC_BIND_TYPE::COUNT];
			public:
				void ClearCount();
				void ClearAllData();
			public:
				bool HasSpace()const noexcept;
			}; 
		public:
			virtual void Initialize(JGraphicDevice* device) = 0;
			virtual void Clear() = 0;  
		public: 
			virtual uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
			virtual uint GetResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0; 
			virtual uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
			virtual uint GetViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
			virtual uint GetViewOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
			virtual uint GetTotalViewCount(const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
			virtual uint GetTotalViewCapacity(const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0; 
			virtual ResourceHandle GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept = 0;
			virtual ResourceHandle GetMPBResourceCpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
			virtual ResourceHandle GetMPBResourceGpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
			virtual JGraphicResourceInfo* GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept = 0;		
			virtual JGraphicResourceInfo* GetMPBInfo(const Core::JDataHandle& handle)const noexcept = 0;
			/**
			* @brief Try get JGraphicResourceInfo releated task
			*/
			JGraphicResourceInfo* GetInfo(JGraphicResourceUserAccess* access, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE task)const noexcept;
		protected:
			//has platform dependency!
			//추후에 window이외에 platform을 사용한다면 수정필요!
			HWND GetWindowHandle()noexcept;
		public:
			virtual bool CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
			virtual bool CanCreateOptionResource(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
		public:
			virtual JUserPtr<JGraphicResourceInfo> CreateResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType) = 0;
			virtual bool CreateOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo> info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType) = 0;
			virtual bool DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info) = 0;
			virtual bool DestroyGraphicOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype) = 0;
		public: 
			//reflect exist resource desc
			virtual bool ReAllocTypePerAllResource(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType) = 0;
			//reflect new desc
			virtual bool ReAllocTypePerAllResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType) = 0;
		public: 
			virtual bool CopyResource(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& from, const JUserPtr<JGraphicResourceInfo>& to) = 0;
		public:
			static constexpr uint MPBCapactiy()noexcept { return 48; }
			/**
			* @brief for recreation or debugging
			* @return handle number -1 is invalid number and return false if mipLevel < 2
			*/
			virtual bool SettingMipmapBind(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& info, const bool isReadOnly,_Out_ std::vector<Core::JDataHandle>& handle) = 0;
			virtual void DestroyMPB(JGraphicDevice* device, Core::JDataHandle& handle) = 0;
		public:
			virtual void ResizeWindow(JGraphicDevice* device) = 0;
		public:
			virtual void StoreTexture(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType, const int index, const std::wstring& path) = 0;
		};
	}
}