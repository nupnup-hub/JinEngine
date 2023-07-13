#pragma once
#include"JCullingType.h"
#include"../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JCullingManager;
		class JCullingInfo
		{ 
			REGISTER_CLASS_USE_ALLOCATOR(JCullingInfo)
		private:
			using IsCullingPtr = bool(*)(void*, const uint)noexcept;
			using CullingPtr = void(*)(void*, const uint, const bool)noexcept;
		private:
			friend class JCullingManager;
		private:
			JCullingManager* manager; 
		private: 
			const J_CULLING_TYPE cullingType;
			int arrayIndex = -1;
		private:
			size_t resultSize = 0; 
			void* result = nullptr;
			CullingPtr cullingPtr = nullptr;
			IsCullingPtr isCullingPtr = nullptr;
		private:
			float updateFrequency = 0;	//0 is allways update
			float elapsedTime = 0;
		private:
			bool canCulling = true; 
		public:
			int GetArrayIndex()const noexcept;
			size_t GetResultSize()const noexcept;
			J_CULLING_TYPE GetCullingType()const noexcept; 
		public: 
			void SetArrayIndex(const int newValue)noexcept;
			//set update frequency
			//0.0sec ~ 1.0sec
			void SetUpdateFrequency(const float newUpdateFrequency, const float startTime = 0)noexcept; 
			void SetResultPtr(void* ptr, const size_t size)noexcept;
		public:
			void Culling(const uint index, const bool value)const noexcept; 
		public: 
			void Update()noexcept; 
		public: 
			bool IsCulled(const uint index)const noexcept;	//culling array is ordered by render item mesh number
			bool UnsafeIsCulled(const uint index)const noexcept;	//skip check valid index
			bool CanCulling()const noexcept; 		 
		public:
			static bool Destroy(JCullingInfo* info);
		private:
			JCullingInfo(JCullingManager* manager, const J_CULLING_TYPE cullingType);
			~JCullingInfo();
		};
	}
}