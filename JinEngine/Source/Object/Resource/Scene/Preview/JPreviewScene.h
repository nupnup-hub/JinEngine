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
#include"JPreviewEnum.h"
#include"../../Texture/JDefaulTextureType.h"
#include"../../../../Core/JCoreEssential.h"
#include"../../../../Graphic/JGraphicConstants.h" 
#include"../../../../Core/Pointer/JOwnerPtr.h"
#include"../../../../Core/Utility/JMacroUtility.h"
#include"../../../../Core/Math/JVector.h"  

namespace JinEngine
{   
	class JObject;
	class JMaterial;
	class JScene;
	class JCamera;  
	class JTransform;
	class JRenderItem;

	class JPreviewScene
	{ 
	private:
		friend class JPreviewSceneGroup;
	protected:
		struct AdjustSceneSettingData
		{
		public: 
			JVector2F pad = JVector2F::Zero();				//valid in 2d preview
			JVector3F additionalPos = JVector3F::Zero();	//valid in 3d preview
			JVector3F additionalPosRate = JVector3F::Zero();	//valid in 2d preview
		public:
			JUserPtr<JTransform> targetTransform;					//preview target
			JUserPtr<JRenderItem> targetRenderItem;					//preview target
		public:
			bool useFixedPad = false;			//valid in 2dTexture
		};
	private:
		const size_t guid;
	private:
		JUserPtr<JObject> jobject;
		JUserPtr<JScene> scene;
		JUserPtr<JMaterial> textureMaterial;
		JUserPtr<JCamera> camera = nullptr;
	private:
		J_PREVIEW_DIMENSION previewDimension; 
		J_PREVIEW_FLAG previewFlag;  
	private:
		bool useQuadShape = false;
	public:
		JPreviewScene(JUserPtr<JObject> jobject, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewScene();
		JPreviewScene(JPreviewScene&& rhs) = default;
		JPreviewScene& operator=(JPreviewScene && rhs) = default; 
	public:
		size_t GetGuid()const noexcept;
		JUserPtr<JObject> GetJObject()noexcept;
		JUserPtr<JCamera> GetPreviewCamera()noexcept;
		J_PREVIEW_DIMENSION GetPreviewDimension()const noexcept; 
		J_SIMPLE_GET(JUserPtr<JScene>, scene, Scene);
	protected:
		void SetUseQuadShapeTrigger(const bool value)noexcept;
		void SetScene(JUserPtr<JScene> newScene)noexcept;
		J_SIMPLE_GET_SET(JUserPtr<JMaterial>, textureMaterial, TextureMaterial);
	public:
		//Draw quad shape for texture rendering
		bool UseQuadShape()const noexcept;
	public:
		void Clear()noexcept;
	protected: 
		void AdjustScene(const AdjustSceneSettingData& data)noexcept;
	private:
		void Adjust2DTextureScene(const AdjustSceneSettingData& data)noexcept;
		void Adjust2DScene(const AdjustSceneSettingData& data)noexcept;
		void Adjust3DFixedScene(const AdjustSceneSettingData& data)noexcept;
		void Adjust3DNonFixedScene(const AdjustSceneSettingData& data)noexcept;
	};
}