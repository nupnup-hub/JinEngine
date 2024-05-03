#pragma once 
#include"../../JGraphicSubClassInterface.h"
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
		class JDrawHelper;

		class JFrustumCulling : public JGraphicSubClassInterface
		{ 
		public:
			~JFrustumCulling();
		public:
			void Initialize();
			void Clear();
		public:
			void FrustumCulling(const JUserPtr<JScene>& scene, const JDrawHelper& helper);
		};
	}
}