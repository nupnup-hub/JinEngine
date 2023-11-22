#pragma once 
#include"JGraphicResourceType.h" 
#include"../JGraphicConstants.h"
#include"../Device/JGraphicDeviceUser.h" 
#include"../../Core/JCoreEssential.h"  
#include"../../Core/Pointer/JOwnerPtr.h" 
#include"../../Core/Geometry/Mesh/JMeshStruct.h"
#include"../../Core/Handle/JDataHandleStructure.h"
#include<string>

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
		struct JGraphicBaseDataSet; 

		struct MPBInfo
		{
		public:
			JUserPtr<JGraphicResourceInfo> info; 
			int index[(uint)J_GRAPHIC_BIND_TYPE::COUNT] = { invalidIndex, invalidIndex ,invalidIndex ,invalidIndex};
		};
		//Manage graphic texture or buffer 
		class JGraphicResourceManager : public JGraphicDeviceUser
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
				void ClearCount();
				void ClearAllData();
			public:
				int GetNextViewIndex()const noexcept;
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
			JGraphicResourceManager() = default;
			virtual ~JGraphicResourceManager() = default;
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
		public:
			static uint GetOcclusionMipmapViewCapacity()noexcept;
			static uint GetOcclusionMinSize()noexcept;
		protected:
			//has platform dependency!
			//���Ŀ� window�̿ܿ� platform�� ����Ѵٸ� �����ʿ�!
			HWND GetWindowHandle()noexcept;
		public:
			virtual bool CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
			virtual bool CanCreateOptionResource(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
		public:
			virtual JUserPtr<JGraphicResourceInfo> CreateSceneDepthStencilResource(JGraphicDevice* device, const uint viewWidth, const uint viewHeight) = 0;
			//for display scene depth stencil map
			//for debug ui or debug object
			virtual JUserPtr<JGraphicResourceInfo> CreateDebugDepthStencilResource(JGraphicDevice* device, const uint viewWidth, const uint viewHeight) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateLayerDepthDebugResource(JGraphicDevice* device, const uint viewWidth, const uint viewHeight) = 0;
			virtual void CreateHZBOcclusionResource(JGraphicDevice* device,
				const uint occWidth,
				const uint occHeight,
				_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo,
				_Out_ JUserPtr<JGraphicResourceInfo>& outOccMipmapInfo) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateOcclusionResourceDebug(JGraphicDevice* device,
				const uint occWidth,
				const uint occHeight,
				const bool isHzb) = 0;
			virtual JUserPtr<JGraphicResourceInfo> Create2DTexture(JGraphicDevice* device, const JTextureCreateDesc& createDesc) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateCubeMap(JGraphicDevice* device, const JTextureCreateDesc& createDesc) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateRenderTargetTexture(JGraphicDevice* device, const uint width, const uint height) = 0; 
			virtual JUserPtr<JGraphicResourceInfo> CreateShadowMapTexture(JGraphicDevice* device, const uint width, const uint height) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateShadowMapTextureArray(JGraphicDevice* device, const uint width, const uint height, const uint count) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateShadowMapTextureCube(JGraphicDevice* device, const uint width, const uint height) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateVertexBuffer(JGraphicDevice* device, const std::vector<Core::JStaticMeshVertex>& vertex) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateVertexBuffer(JGraphicDevice* device, const std::vector<Core::JSkinnedMeshVertex>& vertex) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateIndexBuffer(JGraphicDevice* device, const std::vector<uint32>& index) = 0;
			virtual JUserPtr<JGraphicResourceInfo> CreateIndexBuffer(JGraphicDevice* device, const std::vector<uint16>& index) = 0;
			virtual bool CreateOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType) = 0;
			virtual bool DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info) = 0;
			virtual bool DestroyGraphicOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype) = 0;
		public:
			virtual bool CopyResource(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& from, const JUserPtr<JGraphicResourceInfo>& to) = 0;
		public:
			static constexpr uint MPBCapactiy()noexcept { return 32; }
			/**
			* @brief for recreation or debugging
			* @return handle number -1 is invalid number and return false if mipLevel < 2
			*/
			virtual bool SettingMipmapBind(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& info, const bool isReadOnly,_Out_ std::vector<Core::JDataHandle>& handle) = 0;
			virtual void DestroyMPB(JGraphicDevice* device, Core::JDataHandle& handle) = 0;
		public:
			virtual void ResizeWindow(const JGraphicBaseDataSet& base, JGraphicDevice* device) = 0;
		public:
			virtual void StoreTexture(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType, const int index, const std::wstring& path) = 0;
		};
	}
}