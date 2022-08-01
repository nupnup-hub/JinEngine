#pragma once
#include<DirectXCollision.h> 
#include"JCameraState.h"
#include"../JComponent.h"
#include"../../IFrameResourceControl.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicTextureHandle;
		struct JCameraConstants;
	}

	class JTransform;
	class GameObjectDirty;
	class JCamera : public JComponent, public IFrameResourceControl
	{
		REGISTER_CLASS(JCamera)
	private:
		JTransform* ownerTransform;
		GameObjectDirty* gameObjectDirty;
		Graphic::JGraphicTextureHandle* graphicTextureHandle = nullptr;
		J_CAMERA_STATE camState = J_CAMERA_STATE::IDEL;
		uint camCBIndex = 0; 

		// Cache frustum properties.
		float cameraNear = 0.0f;
		float cameraFar = 0.0f;
		float cameraAspect = 0.0f;
		float cameraFov = 0.0f;
		float cameraNearViewHeight = 0.0f;
		float cameraFarViewHeight = 0.0f;

		int viewWidth;
		int viewHeight;
		 
		bool isOrtho = false;
		bool isMainCamera = false; 

		// Cache View/Proj matrices.
		DirectX::XMFLOAT4X4 mView;
		DirectX::XMFLOAT4X4 mProj;
		DirectX::BoundingFrustum mCamFrustum;
	public: 	  
		bool IsMainCamera()const noexcept;

		JTransform* GetTransform()noexcept;
		DirectX::XMMATRIX GetView()const noexcept;
		DirectX::XMMATRIX GetProj()const noexcept;
		DirectX::XMFLOAT4X4 GetView4x4f()const noexcept;
		DirectX::XMFLOAT4X4 GetProj4x4f()const noexcept; 
		DirectX::BoundingFrustum GetBoundingFrustum()const noexcept;
		float GetNear()const noexcept;
		float GetFar()const noexcept;
		float GetAspect()const noexcept;
		float GetFovY()const noexcept;
		float GetFovX()const noexcept;
		int GetViewWidth()const noexcept;
		int GetViewHeight()const noexcept; 
		float GetNearViewWidth()const noexcept;
		float GetNearViewHeight()const noexcept;
		float GetFarViewWidth()const noexcept;
		float GetFarViewHeight()const noexcept;
		J_CAMERA_STATE GetCameraState()const noexcept;
		uint GetCameraCBIndex()const noexcept;
		uint GetRsSrvHeapIndex()const noexcept;
		uint GetRsRtvHeapIndex()const noexcept;
		uint GetRsVectorIndex()const noexcept;
	 
		void SetNear(float value)noexcept;
		void SetFar(float value) noexcept;
		void SetFov(float value) noexcept;
		void SetViewSize(int width, int height) noexcept;
		void SetOrthoCamera()noexcept;
		void SetPerspectiveCamera()noexcept;
		void SetMainCamera()noexcept;  
		void SetCameraState(const J_CAMERA_STATE state)noexcept;
		void SetCameraCBIndex(const uint index)noexcept;

		// After modifying camera position/orientation, call to rebuild the view matrix.
		void UpdateViewMatrix()noexcept;
		void StuffCameraConstant(Graphic::JCameraConstants& constant)noexcept;
 
		bool IsDirtied()const noexcept;
		void SetDirty()noexcept;
		void OffDirty()noexcept;
		void MinusDirty()noexcept;

		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static J_COMPONENT_TYPE GetStaticComponentType()noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		void SetAspect(float value) noexcept;
		void CalPerspectiveLens() noexcept;
		void CalOrthoLens() noexcept;
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JCamera* camera);
		static JCamera* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterFunc();
	private:
		JCamera(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner);
		~JCamera();
	};
}