#pragma once
#include<vector>
#include"JAnimationData.h"  
#include"../../../Core/Loader/FbxLoader/JFbxUtility.h"

namespace JinEngine
{
	JAnimationData::JAnimationData(Core::JFbxAnimationData&& jfbxData)
	{
		animationSample = std::move(jfbxData.animationSample);
		skeletonHash = jfbxData.skeletonHash;
		clipLength = jfbxData.clipLength;
		framePerSecond = jfbxData.framePerSecond;
	}
}