#pragma once 
#include"JPreviewEnum.h"
#include"../../Texture/JDefaulTextureType.h"
#include"../../../../Core/JDataType.h"
#include"../../../../Graphic/FrameResource/JFrameResourceConstant.h" 
#include"../../../../Core/Pointer/JOwnerPtr.h"
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
	class JPreviewScene
	{ 
		friend class JPreviewSceneGroup;
	protected: 
		const size_t guid;
		Core::JUserPtr<JObject> jobject;
		JCamera* previewCamera = nullptr;
		J_PREVIEW_DIMENSION previewDimension; 
		J_PREVIEW_FLAG previewFlag;  
	public:
		JPreviewScene(Core::JUserPtr<JObject> jobject, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewScene();
		JPreviewScene(JPreviewScene&& rhs) = default;
		JPreviewScene& operator=(JPreviewScene && rhs) = default; 
	public:
		size_t GetGuid()const noexcept;
		Core::JUserPtr<JObject> GetJObject()noexcept;
		Core::JUserPtr<JCamera> GetPreviewCamera()noexcept;
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