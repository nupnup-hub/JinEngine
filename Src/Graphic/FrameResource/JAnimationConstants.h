#pragma once
#include<DirectXMath.h>
#include"../../Object/Resource/Skeleton/JSkeletonFixedData.h"

namespace JinEngine
{
    namespace Graphic
    {
        struct JAnimationConstants
        {
            DirectX::XMFLOAT4X4 boneTransforms[JSkeletonFixedData::maxJointCount];
        };
    }
}