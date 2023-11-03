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
			bool Create2DTexture(const uint maxSize, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateCubeMap(const uint maxSize, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateRenderTargetTexture(const JVector2<uint> size = JVector2<uint>(0, 0));
			bool CreateShadowMapTexture(const uint size);
			bool CreateShadowMapTextureArray(const uint size, const uint count);
			bool CreateShadowMapTextureCube(const uint size);
			bool CreateVertexBuffer(const std::vector<Core::JStaticMeshVertex>& vertex);
			bool CreateVertexBuffer(const std::vector<Core::JSkinnedMeshVertex>& vertex);
			bool CreateIndexBuffer(std::vector<uint32>& index);
			bool CreateIndexBuffer(std::vector<uint16>& index);
			bool CreateOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE option);
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
			int GetOptionHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			Graphic::ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept;
			virtual uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
		public:
			int GetFirstResourceArrayIndex()const noexcept;
			JVector2F GetFirstResourceSize()const noexcept;
			JVector2F GetFirstResourceInvSize()const noexcept;
			J_GRAPHIC_RESOURCE_TYPE GetFirstResourceType()const noexcept;
			Graphic::ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept;
		protected:
			virtual JUserPtr<JGraphicResourceInfo> GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
			virtual JUserPtr<JGraphicResourceInfo> GetFirstInfo()const noexcept = 0;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			bool HasHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			bool HasFirstHandle()const noexcept;
			bool HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex);
			bool HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
			virtual bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;  
			virtual bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
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
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
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
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
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
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
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
		public:
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{ 
				return GetIndex(rType) == invalidIndex;
			}
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return false;
			}
		};
		template<>
		class JGraphicWideSingleResourceHolder<0>
		{};

		template< uint singleCount, bool multiTypeIsFirst, J_GRAPHIC_RESOURCE_TYPE ...multiType>
		class JGraphicWideSingleOneMultiResourceHolder : public JGraphicResourceInterface
		{ 
		private:
			static constexpr uint multiCount = sizeof...(multiType);
			static constexpr uint canHandleResourceCount = multiCount + singleCount;
		private:
			JUserPtr<JGraphicResourceInfo> info[singleCount];
			std::vector<JUserPtr<JGraphicResourceInfo>> multiInfo[multiCount];
		protected:
			void DestroyTexture(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				int multiIndex = GetMultiVecIndex(rType);
				if (multiIndex != invalidIndex)
				{
					const uint multiVecCount = (uint)multiInfo[multiIndex].size();
					for (uint i = 0; i < multiVecCount; ++i)
						JGraphicResourceInfo::Destroy(multiInfo[multiIndex][i].Release());
					multiInfo[multiIndex].clear();
				}
				else
				{
					int index = GetSingleInfoIndex(rType);
					if (index == invalidIndex)
						return;

					JGraphicResourceInfo::Destroy(info[index].Release());
					for (int i = index; i < singleCount - 1; ++i)
						info[i] = std::move(info[i + 1]);
				}
			}
			void DestroyMultiTexture(const J_GRAPHIC_RESOURCE_TYPE rType, const uint index)
			{
				int multiIndex = GetMultiVecIndex(rType);
				if (multiIndex != invalidIndex)
				{
					if (multiInfo[multiIndex].size() <= index)
						return;

					JGraphicResourceInfo::Destroy(multiInfo[multiIndex][index].Release());
					multiInfo[multiIndex].erase(multiInfo[multiIndex].begin() + index);
				}
			}
			void DestroyAllTexture()
			{
				for (uint i = 0; i < singleCount; ++i)
					JGraphicResourceInfo::Destroy(info[i].Release());

				for (uint i = 0; i < multiCount; ++i)
				{
					const uint multiVecCount = (uint)multiInfo[i].size();
					for (uint j = 0; j < multiVecCount; ++j)
						JGraphicResourceInfo::Destroy(multiInfo[i][j].Release());
					multiInfo[i].clear();
				}
			}
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo)
			{
				int multiIndex = GetMultiVecIndex(newInfo->GetGraphicResourceType());
				if (multiIndex != invalidIndex)
					multiInfo[multiIndex].push_back(newInfo);
				else
				{
					for (uint i = 0; i < singleCount; ++i)
					{
						if (info[i] == nullptr)
						{
							info[i] = newInfo;
							return;
						}
					}
				}
			}
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{ 
				return IsMultiType(rType) ? multiInfo[GetMultiVecIndex(rType)].size() : (GetSingleInfoIndex(rType) != invalidIndex ? 1 : 0);
			}
		protected:
			JUserPtr<JGraphicResourceInfo> GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final
			{
				int multiIndex = GetMultiVecIndex(rType);
				if (multiIndex != invalidIndex)
				{ 
					return multiIndex != invalidIndex ? (multiInfo[multiIndex].size() > dataIndex ? multiInfo[multiIndex][dataIndex] : nullptr) : nullptr;
				}
				else
				{
					int index = GetSingleInfoIndex(rType);
					return index != invalidIndex ? info[index] : nullptr;
				}	 
			}
			JUserPtr<JGraphicResourceInfo> GetFirstInfo()const noexcept final
			{ 
				int firstMultiVecIndex = FindFirstValidMultiVecIndex();
				if (multiTypeIsFirst && firstMultiVecIndex != invalidIndex)
					return multiInfo[firstMultiVecIndex][0];
				else
					return info[0];
			}
		private:
			int GetSingleInfoIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				if (!IsMultiType(rType))
				{
					for (uint i = 0; i < singleCount; ++i)
					{
						if (info[i] != nullptr && info[i]->GetGraphicResourceType() == rType)
							return i;
					}
				}
				return invalidIndex;
			} 
			int GetMultiVecIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{ 
				auto findMultiVecIndexLam = [](const J_GRAPHIC_RESOURCE_TYPE vecType, const J_GRAPHIC_RESOURCE_TYPE rType, int& index, bool& isFind)
				{
					if (isFind)
						return;
					 
					if (vecType == rType)
						isFind = true;
					else
						++index;
				};
				int index = 0;
				bool isFind = false;
				(findMultiVecIndexLam(multiType, rType, index, isFind), ...);

				return isFind ? index : invalidIndex;
			}
			int FindFirstValidMultiVecIndex()const noexcept
			{
				auto findMultiVecIndexLam = [](const J_GRAPHIC_RESOURCE_TYPE vecType, size_t size, int& index, bool& isFind)
				{
					if (isFind)
						return;
					 
					if (size > 0)
						isFind = true;
					else
						++index;
				};
				int index = 0;
				bool isFind = false;
				(findMultiVecIndexLam(multiType, multiInfo[index].size(), index, isFind), ...);

				return isFind ? index : invalidIndex;
			}
		public:
			bool IsMultiType(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return ((rType == multiType) || ...);
			}
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				return IsMultiType(rType) ? true : GetSingleInfoIndex(rType) == invalidIndex;
			}
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return IsMultiType(rType);
			}
		};

		template<bool multiTypeIsFirst, J_GRAPHIC_RESOURCE_TYPE ...multiType>
		class JGraphicWideSingleOneMultiResourceHolder<0, multiTypeIsFirst, multiType...>
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
			int GetOptionHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			Graphic::ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept;
		public:
			int GetFirstResourceArrayIndex()const noexcept;
			JVector2F GetFirstResourceSize()const noexcept;
			JVector2F GetFirstResourceInvSize()const noexcept;
			/**
			* return invalid index(cast J_GRAPHIC_RESOURCE_TYPE) if access invalid data 
			*/
			J_GRAPHIC_RESOURCE_TYPE GetFirstResourceType()const noexcept;
			Graphic::ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			bool HasHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			bool HasFirstHandle()const noexcept;
			bool HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex);
			bool HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
		};
	}
}
