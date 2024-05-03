#pragma once
#include"JCullingType.h" 
#include"../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	namespace Graphic
	{
		/*
		* JCullingManager에서만 생성가능한 Info
		* 자원을 JCullingManager 에서 할당받아 Culling을 수행하는 class에서 참조한다
		* 항상 유효한 객체이다
		*/
		class JCullingManager;
		class JCullingResultHolder;

		//record last updated information
		struct JCullingUpdatedInfo
		{
		public:
			uint updatedStartIndex = 0;
			uint updatedCount = 0;
		};
		class JCullingInfo final
		{ 
			REGISTER_CLASS_USE_ALLOCATOR(JCullingInfo) 
		private:
			friend class JCullingManager;
		private:
			JCullingManager* manager;  
		private:
			std::unique_ptr<JCullingResultHolder> resultHolder;
			std::vector<JCullingUpdatedInfo> updatedInfo;
		private: 
			const J_CULLING_TYPE cullingType;
			int arrayIndex = -1;
		private:
			float updateFrequency = 1.0f; 
		private:
			//culling이 분할 update을 할시 모든 부분이 update 될시 true
			bool isUpdateEnd = true;
			bool canCulling = true;  
		public:
			int GetArrayIndex()const noexcept; 
			uint GetResultBufferElementCount()const noexcept;
			uint GetResultBufferSize()const noexcept;
			uint GetUpdatedInfoCount()const noexcept;
			JCullingUpdatedInfo GetUpdateddInfo(const uint index)const noexcept;
			J_CULLING_TYPE GetCullingType()const noexcept; 
			J_CULLING_TARGET GetCullingTarget()const noexcept;
			float GetUpdateFrequency()const noexcept; 
		public: 
			void SetArrayIndex(const int newValue)noexcept; 
			void SetUpdatedInfo(const JCullingUpdatedInfo& info, const uint index)noexcept;
			void SetUpdateFrequency(const bool value)noexcept; 
			void SetUpdateEnd(const bool value)noexcept;
		public:
			/**
			* @brief occlusion culling can't set value in cpu
			*/
			void Culling(const uint index, const bool value)const noexcept; 
		public: 
			/**
			* @brief index is ordered by render item number
			*/
			bool IsCulled(const uint index)const noexcept;	 
			bool IsCullingResultInGpu()const noexcept;
			bool IsUpdateEnd()const noexcept;
			bool CanSetCullingValue()const noexcept;
		public:
			static bool Destroy(JCullingInfo* info);
		private:
			JCullingInfo(JCullingManager* manager, 
				const J_CULLING_TYPE cullingType, 
				const uint updatedInfoCount,
				std::unique_ptr<JCullingResultHolder>&& resultHolder); 
			~JCullingInfo();
		};
	}
}

/*
*			unuse
			//set update frequency
			//0.0sec ~ 1.0sec
			void SetUpdateFrequency(const float newUpdateFrequency, const float startTime = 0)noexcept;
			void Update()noexcept;
			bool CanCulling()const noexcept;
*/