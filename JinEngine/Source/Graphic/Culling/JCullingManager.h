#pragma once 
#include"JCullingType.h"
#include"JCullingInfo.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/Reflection/JReflection.h" 
#include<vector>
  
namespace JinEngine
{
	namespace Graphic
	{ 
		class JCullingResultHolder;
		class JGraphicDevice; 

		//Manage culling resource(buffer)
		class JCullingManager : public JGraphicDeviceUser
		{
		private:
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCullingManager)
		private: 
			std::vector<JOwnerPtr<JCullingInfo>> cullingInfo[(uint)J_CULLING_TYPE::COUNT];
		public:
			~JCullingManager() = default;
		public:
			uint GetCullingInfoCount(const J_CULLING_TYPE type)const noexcept;
			JUserPtr<JCullingInfo> GetCullingInfo(const J_CULLING_TYPE type, const uint index)const noexcept;   
			std::vector<JUserPtr<JCullingInfo>> GetCullingInfoVec(const J_CULLING_TYPE type)const noexcept;
		protected:
			JCullingResultHolder* GetHolder(JCullingInfo* info)const noexcept;
		public:
			void ReBuildBuffer(const J_CULLING_TYPE type, JGraphicDevice* device, const size_t capacity);
		protected:
			virtual void ReBuildBuffer(JCullingInfo* info, JGraphicDevice* device, const size_t capacity, const uint index) = 0;
		public:
			virtual JUserPtr<JCullingInfo> CreateFrsutumData(const size_t capacity) = 0;
			virtual JUserPtr<JCullingInfo> CreateHzbOcclusionData(JGraphicDevice* device, const size_t capacity) = 0;
			virtual JUserPtr<JCullingInfo> CreateHdOcclusionData(JGraphicDevice* device, const size_t capacity) = 0;
		protected:
			JUserPtr<JCullingInfo> PostCreation(JOwnerPtr<JCullingInfo>&& newInfo, const J_CULLING_TYPE type);
		public:
			bool DestroyCullingData(JCullingInfo* info, JGraphicDevice* device);
		protected:
			virtual void PostDestruction(const J_CULLING_TYPE type, const uint index) = 0;
		public:
			virtual void Clear();
		public:
			static void RegisterTypeData();
		};
	}
}