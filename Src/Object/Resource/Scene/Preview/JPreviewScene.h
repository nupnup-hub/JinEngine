#pragma once 
#include"JPreviewEnum.h"
#include"../../Texture/EditorTextureEnum.h"
#include"../../../../Core/JDataType.h"
#include"../../../../Graphic/FrameResource/JFrameResourceCash.h" 
#include<memory>
#include<string>
#include<vector>
#include<DirectXMath.h>

namespace JinEngine
{   
	class JObject;
	class JMaterial;
	class JScene;
	class JCamera; 
	/*
	* JModel & Prepab JGameObject를 소유하는 Resource는 각자 JScene을 소유하므로 하나의 scene을 관찰
	* 그외에 리소스는 JScene을 각 PreviewScene 별로 생성 관리
	*/
	class JPreviewScene
	{ 
		friend class JPreviewSceneGroup;
	protected: 
		const size_t guid;
		JObject* jobject;
		JCamera* previewCamera;
		J_PREVIEW_DIMENSION previewDimension; 
		J_PREVIEW_FLAG previewFlag;  
	public:
		JPreviewScene(JObject* jobject, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewScene();
		JPreviewScene(JPreviewScene&& rhs) = default;
		JPreviewScene& operator=(JPreviewScene && rhs) = default;
	public:
		size_t GetGuid()const noexcept;
		JObject* GetJObject()noexcept; 
		JCamera* GetPreviewCamera()noexcept;
		J_PREVIEW_DIMENSION GetPreviewDimension()const noexcept; 
	public:
		virtual void Clear()noexcept = 0;
	protected:
		virtual JScene* GetScene()noexcept = 0;  
		void AdjustCamera(_In_ JScene* scene,
			_Inout_ JCamera* camera,
			_In_ const DirectX::XMFLOAT3& objCenter, 
			const float objRadius, 
			bool isQuad = false,
			const DirectX::XMFLOAT3 additionalPos = DirectX::XMFLOAT3(0,0,0))noexcept;
	private:
		void Adjust2DTextureCamera(_In_ JScene* scene,
			_Inout_ JCamera* camera,
			const DirectX::XMFLOAT3& objCenter,
			const float objRadius,
			const DirectX::XMFLOAT3 additionalPos)noexcept;
		void Adjust2DOtherCamera(_In_ JScene* scene, 
			_Inout_ JCamera* camera,
			const DirectX::XMFLOAT3& objCenter,
			const float objRadius,
			const DirectX::XMFLOAT3 additionalPos)noexcept;
		void Adjust3DFixedCamera(_In_ JScene* scene, 
			JCamera* camera, 
			const DirectX::XMFLOAT3& objCenter,
			const float objRadius,
			const DirectX::XMFLOAT3 additionalPos)noexcept;
		void Adjust3DNonFixedCamera(_In_ JScene* scene,
			_Inout_ JCamera* camera,
			const DirectX::XMFLOAT3& objCenter, 
			const float objRadius,
			const DirectX::XMFLOAT3 additionalPos)noexcept; 
	};
}