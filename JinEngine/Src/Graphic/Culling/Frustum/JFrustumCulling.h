#pragma once
#include"../../../Core/Pointer/JOwnerPtr.h"
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDrawTarget;
		class JCullingInfo;
		class JSceneFrustumCullingRequestor;

		class JFrustumCulling
		{
		private:
			std::vector<bool*> resultBuffer;
		public:
			void Initialize();
			void Clear();
		public:
			void BuildResultBuffer(const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo);
			void ReBuildResultBuffer(const size_t capcity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo);
			void DestroyResultBuffer(JCullingInfo* cullignInfo);
		public:
			void FrustumCulling(JGraphicDrawTarget* target, 
				JSceneFrustumCullingRequestor* requestor, 
				const JUserPtr<JCullingInfo>& cullingInfo);
		};
	}
}