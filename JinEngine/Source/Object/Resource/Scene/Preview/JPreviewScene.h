#pragma once 
#include"JPreviewEnum.h"
#include"../../Texture/JDefaulTextureType.h"
#include"../../../../Core/JCoreEssential.h"
#include"../../../../Graphic/JGraphicConstants.h" 
#include"../../../../Core/Pointer/JOwnerPtr.h"
#include"../../../../Core/Utility/JMacroUtility.h"
#include"../../../../Core/Math/JVector.h" 
#include<memory>
#include<string>
#include<vector> 

namespace JinEngine
{   
	class JObject;
	class JMaterial;
	class JScene;
	class JCamera;  

	class JPreviewScene
	{ 
	private:
		friend class JPreviewSceneGroup;
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
		void AdjustCamera(_In_ const JVector3<float>& objCenter, 
			const float objRadius, 
			const JVector3<float>& additionalPos = JVector3<float>(0,0,0))noexcept;
	private:
		void Adjust2DTextureCamera(const JVector3<float>& objCenter,
			const float objRadius,
			const JVector3<float>& additionalPos)noexcept;
		void Adjust2DOtherCamera(const JVector3<float>& objCenter,
			const float objRadius,
			const JVector3<float>& additionalPos)noexcept;
		void Adjust3DFixedCamera(const JVector3<float>& objCenter,
			const float objRadius,
			const JVector3<float>& additionalPos)noexcept;
		void Adjust3DNonFixedCamera(const JVector3<float>& objCenter, 
			const float objRadius,
			const JVector3<float>& additionalPos)noexcept;
	};
}