#include"JAnimationUpdateData.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		void JAnimationUpdateData::DiagramData::Clear()
		{		 
			nowState = nullptr;
			nextState = nullptr;
			nowTransition = nullptr;
			preTransition = nullptr;
			blender.Clear();
			weight = 0;

			const XMFLOAT4X4 identity = JMathHelper::Identity4x4();
			for (uint i = 0; i < JAnimationFixedData::defaultCrossFadingCount; ++i)
			{
				animationTimes[i].Clear();
				for (uint j = 0; j < JSkeletonFixedData::maxJointCount; ++j)
					worldTransform[i][j] = identity;
			}
		}
		void JAnimationUpdateData::Initialize()noexcept
		{ 
			for (uint i = 0; i < JAnimationFixedData::fsmDiagramMaxCount; ++i)
				diagramData[i].Clear();

			for (uint i = 0; i < 4; ++i)
			{
				if (ikJoint[i].size() == 0)
				{
					ikJoint[i].reserve(JAnimationFixedData::ikVectorSize);
					ikJoint[i].resize(JAnimationFixedData::ikVectorSize);
				}
			}
			if (ikRate.size() == 0)
			{
				ikRate.reserve(JSkeletonFixedData::maxJointCount);
				ikRate.resize(JSkeletonFixedData::maxJointCount);
			}

			paramValueMap.clear();
		}
		 
		void JAnimationUpdateData::ClearSkeletonBlendRate(const uint index)noexcept
		{
			if (index >= JAnimationFixedData::defaultCrossFadingCount)
				return;

			for (auto& data : skeletonBlendRate[index])
				data.second = 0;
		}
		bool JAnimationUpdateData::FindClipGuid(size_t& guid, const uint index)noexcept
		{
			for (auto& data : skeletonBlendRate[index])
			{
				if (data.second == 1)
				{
					guid = data.first;
					return true;
				}
			}
			return false;
		}
		void JAnimationUpdateData::SetIKRate(const uint ikNumber)noexcept
		{
			for (uint i = 0; i < ikCount[ikNumber]; ++i)
			{
				uint8 jointIndex = ikJoint[ikNumber][i].jointIndex;
				++ikRate[jointIndex].count;
				ikRate[jointIndex].rate = (float)(1.0f / ikRate[jointIndex].count);
			}
		}
		void JAnimationUpdateData::EnterCalculateIK()noexcept
		{
			for (uint i = 0; i < ikRate.size(); ++i)
			{
				ikRate[i].count = 0;
				ikRate[i].rate = 0;
			}

			for (uint i = 0; i < 4; ++i)
				ikCount[i] = 0;
		}
		void JAnimationUpdateData::StuffIdentity(const uint layerNumber, const uint updateNumber)noexcept
		{
			const XMFLOAT4X4 iden = JMathHelper::Identity4x4();
			for (uint i = 0; i < JSkeletonFixedData::maxJointCount; ++i)
				diagramData[layerNumber].worldTransform[updateNumber][i] = iden;
		}
		void JAnimationUpdateData::RegisterParameter(const size_t guid, const float value)noexcept
		{
			paramValueMap.emplace(guid, value);
		}
		float JAnimationUpdateData::GetParameterValue(const size_t guid)const noexcept
		{
			auto data = paramValueMap.find(guid);
			if (data != paramValueMap.end())
				return data->second;
			else
				return -1;
		}
		void JAnimationUpdateData::SetParameterValue(const size_t guid, const float value)noexcept
		{
			auto data = paramValueMap.find(guid);
			if (data != paramValueMap.end())
				data->second = value;
		}
	}
}