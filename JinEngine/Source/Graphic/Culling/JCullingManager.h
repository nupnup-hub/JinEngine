#pragma once 
#include"JCullingType.h"
#include"../../Core/Reflection/JReflection.h"
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{
		class JCullingInfo;
		class JCullingManager
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCullingManager)
		private:
			std::vector<JOwnerPtr<JCullingInfo>> cullingInfo[(uint)J_CULLING_TYPE::COUNT];
		public:
			uint GetCullingInfoCount(const J_CULLING_TYPE type)const noexcept;
			JUserPtr<JCullingInfo> GetCullingInfo(const J_CULLING_TYPE type, const uint index)const noexcept;
			std::vector< JUserPtr<JCullingInfo>> GetCullingInfoVec(const J_CULLING_TYPE type)const noexcept;
		public:
			JUserPtr<JCullingInfo> CreateFrsutumData();
			JUserPtr<JCullingInfo> CreateOcclusionData();
		private:
			JUserPtr<JCullingInfo> CreateCullingInfo(const J_CULLING_TYPE type);
		public:
			bool DestroyCullingData(JCullingInfo* info); 
		public:
			void Clear();
		public:
			static void RegisterTypeData();
		};
	}
}