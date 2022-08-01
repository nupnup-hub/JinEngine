#pragma once

namespace JinEngine
{
	class JGameObject;
	class JSkeletonAsset;
	class JModel;
	struct SkeletonAssetSettingPageShareData
	{
	public:
		JModel* model = nullptr;
		JSkeletonAsset* skeletonAsset = nullptr;
		JGameObject* skeletonRoot = nullptr;
	};
}