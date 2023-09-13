#pragma once
#include"../../../Core/Pointer/JOwnerPtr.h"
#include<vector>

namespace JinEngine
{
	class JComponent;
	class JScene;
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
			void FrustumCulling(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& comp);
		};
	}
}