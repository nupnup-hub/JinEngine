#pragma once  
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	class JObject;
	class JRenderItem;
	class JAnimator;
	class JScene;
	class JCamera;
	class JLight;
	namespace Graphic
	{
		class JGraphic;
		class JDrawHelper;
		//for grapic subclass
		class JFrameIndexAccess
		{  
		private:
			friend class JGraphic;
			friend class JDrawHelper;
		public:
			//per scene
			int GetSceneFrameIndex(JScene* scene)const noexcept;
			//per cam
			int GetCamFrameIndex(JCamera* cam, const uint frameLayerIndex)const noexcept;
			//per light
			int GetLitFrameIndex(JLight* lit, const uint frameLayerIndex)const noexcept; 
			int GetLitShadowFrameIndex(JLight* lit)const noexcept;
		public:
			//per render item submesh
			int GetObjectFrameIndex(JRenderItem* rItem)const noexcept;
			//per render item
			int GetBoundingFrameIndex(JRenderItem* rItem)const noexcept;
			//per render item submesh
			int GetRefInfoFrameIndex(JRenderItem* rItem)const noexcept;
			//per animator
			int GetAnimationFrameIndex(JAnimator* ani)const noexcept;
		private:
			JFrameIndexAccess() = default; 
		};
	}
}