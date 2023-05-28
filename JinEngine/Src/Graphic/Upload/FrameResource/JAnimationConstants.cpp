#include"JAnimationConstants.h"
#include"../../../Utility/JMathHelper.h"

namespace JinEngine
{
	namespace Graphic
	{
		void JAnimationConstants::StuffIdentity()noexcept
		{
			const DirectX::XMFLOAT4X4 iden = JMathHelper::Identity4x4();
			for (uint i = 0; i < JSkeletonFixedData::maxJointCount; ++i)
				boneTransforms[i] = iden;
		}
	}
}