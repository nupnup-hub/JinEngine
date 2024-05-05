#pragma once
#include"JCullingType.h"
#include"../Buffer/JGraphicBuffer.h"
#include"../Buffer/JBufferType.h" 

namespace JinEngine
{
	namespace Graphic
	{ 
		class JGraphicDevice;
		class JCullingResultHolder
		{
		public:
			virtual ~JCullingResultHolder() = default;
		public:
			virtual void Culling(const uint index, const bool value)noexcept = 0;
		public:
			uint GetBufferSize()const noexcept;
			virtual uint GetElementSize()const noexcept = 0;
			virtual uint GetElementCount()const noexcept = 0;
			virtual J_GRAPHIC_BUFFER_TYPE GetBufferType()const noexcept = 0;
			virtual J_CULLING_TARGET GetCullingTarget()const noexcept = 0;
		public:
			virtual bool IsCulled(const uint index)const noexcept = 0;
			virtual bool IsGpuResource()const noexcept = 0;
			virtual bool CanSetValue()const noexcept = 0;
		};
		 
		//cpu only
		class JFrustumCullingResultHolder final : public JCullingResultHolder
		{
		private:
			bool* cullingResult = nullptr;
			size_t capacity = 0;
			const J_CULLING_TARGET target;
		public:
			JFrustumCullingResultHolder(const J_CULLING_TARGET target);
			~JFrustumCullingResultHolder();
		public:
			/*
			* @brief 0 is non culling 1 is culling
			*/
			void Culling(const uint index, const bool value)noexcept final;
		public:
			uint GetElementSize()const noexcept final;
			uint GetElementCount()const noexcept final;
			J_GRAPHIC_BUFFER_TYPE GetBufferType()const noexcept final;
			J_CULLING_TARGET GetCullingTarget()const noexcept final;
		public:
			bool IsCulled(const uint index)const noexcept final;
			bool IsGpuResource()const noexcept final;
			bool CanSetValue()const noexcept final;
		public:
			void Build(const uint newCapacity);
			void Clear();
		};
	}
}