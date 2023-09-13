#pragma once
#include"../../Core/JCoreEssential.h"
#include"../Buffer/JGraphicBuffer.h"
#include"../Buffer/JBufferType.h"
#include<string>

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
			virtual uint GetBuffSize()const noexcept = 0;
		public:
			virtual bool IsCulled(const uint index)const noexcept = 0;
			virtual bool IsGpuResource()const noexcept = 0;
			virtual bool CanSetValue()const noexcept = 0;
		};

		class JFrustumCullingResultHolder final : public JCullingResultHolder
		{
		private:
			bool* cullingResult = nullptr;
			size_t capacity = 0;
		public:
			JFrustumCullingResultHolder();
			~JFrustumCullingResultHolder();
		public:
			/*
			* @brief 0 is non culling 1 is culling
			*/
			void Culling(const uint index, const bool value)noexcept final;
		public:
			uint GetBuffSize()const noexcept final;
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