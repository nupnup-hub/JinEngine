#pragma once
#include"JCameraInterface.h" 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JTransform; 
	class JCamera final : public JCameraInterface
	{
		REGISTER_CLASS(JCamera)
	private:
		//JTransform* ownerTransform;  
		J_CAMERA_STATE camState = J_CAMERA_STATE::IDEL; 

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
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		}
	public: 	  
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
	 
		void SetNear(float value)noexcept;
		void SetFar(float value) noexcept;
		void SetFov(float value) noexcept;
		void SetViewSize(int width, int height) noexcept;
		void SetOrthoCamera()noexcept;
		void SetPerspectiveCamera()noexcept;
		void SetMainCamera(bool value)noexcept;

		bool IsMainCamera()const noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	public:
		JCameraStateInterface* StateInterface() final;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		void SetAspect(float value) noexcept;
		void CalPerspectiveLens() noexcept;
		void CalOrthoLens() noexcept;
		void CreateRenderTarget()noexcept;
		void DestroyRenderTarget()noexcept;
	private:
		void UpdateViewMatrix()noexcept;
		bool UpdateFrame(Graphic::JCameraConstants& constant)final; 
	private:
		void SetCameraState(const J_CAMERA_STATE state)noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JCamera* camera);
		static JCamera* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterJFunc();
	private:
		JCamera(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
		~JCamera();
	};
}