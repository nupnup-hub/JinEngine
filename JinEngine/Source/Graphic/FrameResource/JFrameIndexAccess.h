/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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