#include<vector>
#include"JAnimationData.h"  
#include"../Geometry/Mesh/Loader/FbxLoader/JFbxUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JAnimationData::JAnimationData(Core::JFbxAnimationData&& jfbxData)
		{
			animationSample = std::move(jfbxData.animationSample);
			skeletonHash = jfbxData.skeletonHash;
			length = jfbxData.length;
			framePerSecond = jfbxData.framePerSecond;
		}
	}
}