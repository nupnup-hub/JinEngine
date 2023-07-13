#pragma once
#include"JCullingType.h"
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JCullingInfo;
		class JGraphicMultiResourceInterface;
		class JGraphic;
		class JCullingUserInterface;
		class JCullingUpdaterInterface;

		class JCullingInterface
		{
		private:
			friend class JCullingUserInterface;
		private:
			JUserPtr<JCullingInfo> info[(uint)J_CULLING_TYPE::COUNT];
		protected:
			bool CreateFrustumCullingData();
			bool CreateOccCullingData(JGraphicMultiResourceInterface* multiInterface);
		protected:
			bool DestroyCullingData(JGraphicMultiResourceInterface* multiInterface, const J_CULLING_TYPE type)noexcept;
			void DestroyAllCullingData(JGraphicMultiResourceInterface* multiInterface)noexcept;
		protected:
			bool HasCullingData(const J_CULLING_TYPE type)const noexcept;
			bool IsCulled(const uint index)const noexcept;		//culling array is ordered by render item mesh number
			bool UnsafeIsCulled(const uint index)const noexcept;		//Skip check valid index
		};

		class JCullingUserInterface final
		{
		private:
			friend class JCullingUpdaterInterface;
		private:
			JUserPtr<JCullingInfo> info[(uint)J_CULLING_TYPE::COUNT];
		public:
			JCullingUserInterface() = default;
			JCullingUserInterface(JCullingInterface* currInterface);
		public:
			int GetArrayIndex(const J_CULLING_TYPE type)const noexcept;
		public:
			void SetCulling(const J_CULLING_TYPE type, const uint index)noexcept; 
			void OffCulling(const J_CULLING_TYPE type, const uint index)noexcept;
		public:
			bool HasCullingData(const J_CULLING_TYPE type)const noexcept;
			bool IsCulled(const uint index)const noexcept;				//culling array is ordered by render item mesh number
			bool IsCulled(const J_CULLING_TYPE type, const uint index)const noexcept;				//culling array is ordered by render item mesh number
			bool UnsafeIsCulled(const uint index)const noexcept;		//Skip check valid index
			bool UnsafeIsCulled(const J_CULLING_TYPE type, const uint index)const noexcept;		//Skip check valid index
		};
	}
}