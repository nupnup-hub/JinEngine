#pragma once 
#include"PreviewEnum.h"
#include"../../Texture/EditorTextureEnum.h"
#include"../../../../Core/JDataType.h"
#include"../../../../Graphic/FrameResource/JFrameResourceCash.h"
#include"../../JResourceUserInterface.h"
#include<memory>
#include<string>
#include<vector>
#include<DirectXMath.h>

namespace JinEngine
{   
	class JResourceObject;
	class JMaterial;
	class JScene;
	class JCamera; 
	/*
	* JModel & Prepab JGameObject를 소유하는 Resource는 각자 JScene을 소유하므로 하나의 scene을 관찰
	* 그외에 리소스는 JScene을 각 PreviewScene 별로 생성 관리
	*/
	class PreviewScene : public JResourceUserInterface
	{ 
		friend class PreviewSceneGroup;
	protected: 
		std::string name;
		size_t guid;
		JResourceObject* resource;
		JCamera* previewCamera;
		PREVIEW_DIMENSION previewDimension; 
		PREVIEW_FLAG previewFlag;  
	public:
		PreviewScene(const std::string& previewSceneName, JResourceObject* resource, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag);
		~PreviewScene();
		PreviewScene(PreviewScene&& rhs) = default;
		PreviewScene& operator=(PreviewScene&& rhs) = default;

		virtual void Clear()noexcept = 0;

		JResourceObject* GetResouceObject()noexcept; 
		JCamera* GetPreviewCamera()noexcept;
		PREVIEW_DIMENSION GetPreviewDimension()const noexcept; 
	protected:
		virtual JScene* GetScene()noexcept = 0; 
		void OnSceneReference();
		void OffSceneReference();
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