#pragma once 
#include"../JComponent.h"   
#include"../../IFrameResourceControl.h"
#include<DirectXMath.h>
#include<vector>
 
namespace JinEngine
{ 
	class GameObjectDirty;
	class JTransform : public JComponent , public IFrameResourceControl
	{
		REGISTER_CLASS(JTransform)
		friend class JGameObject; 
	private: 
		mutable DirectX::XMFLOAT3 position;
		mutable DirectX::XMFLOAT3 rotation;
		mutable DirectX::XMFLOAT3 scale;
		mutable DirectX::XMFLOAT4X4 world;
		mutable DirectX::XMFLOAT3 tFront;
		mutable DirectX::XMFLOAT3 tRight;
		mutable DirectX::XMFLOAT3 tUp;

		JTransform* parent;
		std::vector<JTransform*> children;
		GameObjectDirty* gameObjectDirty = nullptr;
	public: 
		DirectX::XMFLOAT3 GetPosition()const noexcept;
		DirectX::XMFLOAT3 GetRotation()const noexcept;
		DirectX::XMFLOAT3 GetScale()const noexcept;

		DirectX::XMMATRIX GetWorld()const noexcept;
		DirectX::XMMATRIX GetLocal()const noexcept;
		DirectX::XMVECTOR GetRight()const noexcept;
		DirectX::XMVECTOR GetUp()const noexcept;
		DirectX::XMVECTOR GetFront()const noexcept;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static J_COMPONENT_TYPE GetStaticComponentType()noexcept;

		void SetTransform(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale)noexcept;
		void SetPosition(const DirectX::XMFLOAT3& value)noexcept;
		void SetRotation(const DirectX::XMFLOAT3& value)noexcept;
		void SetScale(const DirectX::XMFLOAT3& value)noexcept;
		  
		void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& worldUp = DirectX::XMFLOAT3(0,1,0))noexcept;

		bool IsDirtied()const noexcept;
		void SetDirty()noexcept;
		void OffDirty()noexcept;
		void MinusDirty()noexcept;

		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		void WorldUpdate()const noexcept; 
		void SetFrameDirty()noexcept; 
		void ChangeParent(int index ,JTransform* newParent)noexcept;
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JTransform* transform);
		static JTransform* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterFunc();
	private:
		JTransform(const size_t guid, const JOBJECT_FLAG flag, JGameObject* owner);
		~JTransform();
	};
}