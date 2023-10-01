#pragma once 
#include"JGraphicResourceType.h"
#include"JGraphicResourceInfo.h"
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../../Core/Math/JVector.h"
#include<string> 
 
namespace JinEngine
{
	class JScene;
	class JComponent;
	class JCamera; 
	namespace Core
	{
		struct JStaticMeshVertex;
		struct JSkinnedMeshVertex;
	}
	namespace Graphic
	{ 
		class JGraphicResourceUserInterface;
		class JCullingInterface;

		/**
		* 주로 impl class가 해당 interface을 상속한다
		* interface는 info를 소유하는 wrapper로 graphic manager와 impl간의 중간역할을 한다
		* impl이 직접 JGraphicResourceInfo를 상속하지 않은건 외부로 노출할 수 없기때문이다.
		*/
		enum class J_GRAPHIC_RESOURCE_HOLD_TYPE
		{
			SINGLE,
			TYPE_PER_SINGLE,
			MULTI
		};
		class JGraphicResourceInterface : public Core::JTypeImplInterfacePointerHolder<JGraphicResourceInterface>
		{
		protected:
			bool CreateSceneDepthStencil();
			bool CreateDebugDepthStencil();
			bool CreateLayerDepthDebugResource(const JVector2<uint> size = JVector2<uint>(0, 0));
			bool CreateHzbOcclusionResource(); 
			bool CreateOcclusionDepthDebug(const bool isHzb);
			bool Create2DTexture(const std::wstring& path, const std::wstring& oriFormat);
			bool CreateCubeMap(const std::wstring& path, const std::wstring& oriFormat);
			bool CreateRenderTargetTexture(const JVector2<uint> size = JVector2<uint>(0, 0));
			bool CreateShadowMapTexture(const uint size);
			bool CreateShadowMapArrayTexture(const uint size, const uint count);
			bool CreateShadowMapCubeTexture(const uint size);
			bool CreateVertexBuffer(const std::vector<Core::JStaticMeshVertex>& vertex);
			bool CreateVertexBuffer(const std::vector<Core::JSkinnedMeshVertex>& vertex);
			bool CreateIndexBuffer(std::vector<uint32>& index);
			bool CreateIndexBuffer(std::vector<uint16>& index);
		protected:
			bool DestroyTexture(JUserPtr<JGraphicResourceInfo>& info);
		private:
			virtual void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo) = 0; 
		public:
			uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceInvSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			Graphic::ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept;
			virtual uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
		public:
			int GetFirstResourceArrayIndex()const noexcept;
			JVector2F GetFirstResourceSize()const noexcept;
			JVector2F GetFirstResourceInvSize()const noexcept;
			Graphic::ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept;
		protected:
			virtual JUserPtr<JGraphicResourceInfo> GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
			virtual JUserPtr<JGraphicResourceInfo> GetFirstInfo()const noexcept = 0;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			bool HasHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			bool HasFirstHandle()const noexcept;
			virtual bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;  
			virtual bool AllowHoldMultiResourcePerType()const noexcept = 0;	//it is true multi resource
		};
		class JGraphicSingleResourceHolder : public JGraphicResourceInterface
		{ 
		private:
			JUserPtr<JGraphicResourceInfo> info = nullptr;
		protected:
			bool DestroyTexture();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo);
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			int GetResourceWidth()const noexcept;
			int GetResourceHeight()const noexcept;
		protected:
			JUserPtr<JGraphicResourceInfo> GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstInfo()const noexcept final;
		public: 
			bool HasGraphicResourceHandle()const noexcept;
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool AllowHoldMultiResourcePerType()const noexcept final;
		};
		class JGraphicTypePerSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		protected:
			void DestroyTexture(const J_GRAPHIC_RESOURCE_TYPE rType);
			void DestroyAllTexture();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo);
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstInfo()const noexcept final;
		public:
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool AllowHoldMultiResourcePerType()const noexcept final;
		};
		class JGraphicMultiResourceHolder : public JGraphicResourceInterface
		{
		private: 
			friend class JCullingInterface; 
		private:
			struct MultiResourceInfo
			{
			public:
				using InfoVec = std::vector<JUserPtr<JGraphicResourceInfo>>;
			public:
				InfoVec vec[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			public: 
				std::vector<JUserPtr<JGraphicResourceInfo>>& operator[](const J_GRAPHIC_RESOURCE_TYPE type)noexcept;
				const std::vector<JUserPtr<JGraphicResourceInfo>>& operator[](const J_GRAPHIC_RESOURCE_TYPE type)const noexcept;
			};
		private:
			MultiResourceInfo info; 
		protected:
			void DestroyTexture(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex);
			void DestroyTexture(const J_GRAPHIC_RESOURCE_TYPE rType);
			void DestroyAllTexture(); 
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo);
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstInfo()const noexcept final;
		public:
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool AllowHoldMultiResourcePerType()const noexcept final;
		}; 

		template<uint count>
		class JGraphicWideSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info[count];
		protected: 
			void DestroyTexture(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				int index = GetIndex(rType);
				if (index == invalidIndex)
					return;

				JGraphicResourceInfo::Destroy(info[index].Release()); 
				for (int i = index; i < count - 1; ++i)
					info[i] = std::move(info[i + 1]);
			}
			void DestroyAllTexture()
			{
				for (uint i = 0; i < count; ++i)
					JGraphicResourceInfo::Destroy(info[i].Release());
			}
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo)
			{ 
				for (uint i = 0; i < count; ++i)
				{
					if (info[i] == nullptr)
					{ 
						info[i] = newInfo;
						return;
					}
				}
			}
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				return GetIndex(rType) != invalidIndex ? 1 : 0;
			}
		protected:
			JUserPtr<JGraphicResourceInfo> GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final
			{
				int index = GetIndex(rType);
				return index != invalidIndex ? info[index] : nullptr;
			}
			JUserPtr<JGraphicResourceInfo> GetFirstInfo()const noexcept final
			{
				return info[0];
			}
		public:
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{ 
				return GetIndex(rType) == invalidIndex;
			}
			bool AllowHoldMultiResourcePerType()const noexcept final
			{
				return false;
			}
		private:
			int GetIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				for (uint i = 0; i < count; ++i)
				{
					if (info[i] != nullptr && info[i]->GetGraphicResourceType() == rType)
						return i;
				}
				return invalidIndex;
			}
		};

		template<>
		class JGraphicWideSingleResourceHolder<0>
		{};

		using JGraphicResourceInterfacePointer = Core::JTypeImplInterfacePointer<JGraphicResourceInterface>;
		class JGraphicResourceUserInterface final
		{
		private:
			JUserPtr<JGraphicResourceInterfacePointer> gPtrWrapper = nullptr; 
		public: 
			JGraphicResourceUserInterface() = default;
			JGraphicResourceUserInterface(JGraphicResourceInterface* gInterface);
			~JGraphicResourceUserInterface() = default;
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceInvSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			Graphic::ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept;
		public:
			int GetFirstResourceArrayIndex()const noexcept;
			JVector2F GetFirstResourceSize()const noexcept;
			JVector2F GetFirstResourceInvSize()const noexcept;
			Graphic::ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			bool HasHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			bool HasFirstHandle()const noexcept;
		};
	}
}
