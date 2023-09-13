#include"JFrustumCulling.h"
#include"../JCullingInfo.h" 
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Scene/JScenePrivate.h"

namespace JinEngine
{
	namespace Graphic
	{
		void JFrustumCulling::Initialize()
		{}
		void JFrustumCulling::Clear()
		{}
		void JFrustumCulling::FrustumCulling(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& comp)
		{ 
			JScenePrivate::CullingInterface::ViewCulling(scene, comp);
		}
	}
}