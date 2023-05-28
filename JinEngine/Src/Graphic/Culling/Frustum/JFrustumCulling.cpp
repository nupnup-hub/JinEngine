#include"JFrustumCulling.h"
#include"../JCullingInfo.h"

namespace JinEngine
{
	namespace Graphic
	{
		void JFrustumCulling::Initialize()
		{

		}
		void JFrustumCulling::Clear()
		{

		}
		void JFrustumCulling::BuildResultBuffer(const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo)
		{
			bool* resultVec = new bool[initCapacity]();
			resultBuffer.push_back(resultVec);
			cullingInfo->SetResultPtr(resultVec, initCapacity);
		}
		void JFrustumCulling::ReBuildResultBuffer(const size_t capcity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo)
		{
			for (const auto& data : cullingInfo)
			{
				const uint index = data->GetArrayIndex();
				const uint preCapacity = data->GetResultSize();

				bool* preVec = resultBuffer[index];
				bool* newVec = new bool[capcity]();

				uint copyEnd = preCapacity > capcity ? capcity : preCapacity;
				for (uint i = 0; i < copyEnd; ++i)
					newVec[i] = preVec[i];

				data->SetResultPtr(newVec, capcity);
				resultBuffer[index] = newVec;
				delete[] preVec;
			}
		}
		void JFrustumCulling::DestroyResultBuffer(JCullingInfo* cullingInfo)
		{ 	
			delete resultBuffer[cullingInfo->GetArrayIndex()];
			resultBuffer.erase(resultBuffer.begin() + cullingInfo->GetArrayIndex());
			cullingInfo->SetResultPtr(nullptr, 0);
		}
		void JFrustumCulling::FrustumCulling(JGraphicDrawTarget* target,
			JSceneFrustumCullingRequestor* requestor,
			const JUserPtr<JCullingInfo>& cullingInfo)
		{

		}
	}
}