#pragma once 
#include"../JComponent.h"  
#include<DirectXMath.h>
#include<vector>

namespace JinEngine
{
	class JTransform final : public JComponent
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JTransform)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JTransformPrivate;
		class JTransformImpl;
	private:
		std::unique_ptr<JTransformImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM;
		}
		DirectX::XMFLOAT3 GetPosition()const noexcept;
		//Euler
		DirectX::XMFLOAT3 GetRotation()const noexcept;
		DirectX::XMFLOAT4 GetQuaternion()const noexcept;
		DirectX::XMFLOAT3 GetScale()const noexcept;
		DirectX::XMVECTOR GetWorldQuaternion()const noexcept;
		DirectX::XMMATRIX GetWorldMatrix()const noexcept;
		DirectX::XMFLOAT4X4 GetWorld4x4f()const noexcept;
		DirectX::XMMATRIX GetLocal()const noexcept;
		DirectX::XMVECTOR GetRight()const noexcept;
		DirectX::XMVECTOR GetUp()const noexcept;
		DirectX::XMVECTOR GetFront()const noexcept;
	public:
		void SetTransform(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale)noexcept;
		void SetPosition(const DirectX::XMFLOAT3& value)noexcept;
		void SetRotation(const DirectX::XMFLOAT3& value)noexcept;
		void SetScale(const DirectX::XMFLOAT3& value)noexcept;
	public:
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final;
	public:
		void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& worldUp = DirectX::XMFLOAT3(0, 1, 0))noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JTransform(const InitData& initData);
		~JTransform();
	};
}

/*
#pragma once
#include"../JComponent.h"
#include"../../JFrameUpdate.h"

namespace JinEngine
{
	class JGameObject;
	class JRenderItem;
	namespace Graphic
	{
		struct JObjectConstants;
	}
	class JTransformGameObjectInterface
	{
	private:
		friend class JGameObject;
	protected:
		virtual void ChangeParent() = 0;
	};

	class JTransformInterface : public JComponent,
		public JFrameDirtyObserver<JFrameDirtyTrigger>,
		public JTransformGameObjectInterface
	{
	protected:
		JTransformInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner);
	};
}
*/

/*
class JTransform final : public JComponent
	{
		REGISTER_CLASS(JTransform)
	private:
		REGISTER_GUI_TABLE_GROUP(Transform, true, "Name", "x", "y", "z")
		REGISTER_PROPERTY_EX(position, GetPosition, SetPosition, GUI_INPUT(false, GUI_TABLE_GROUP_USER(Transform, 3, true)))
		mutable DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0, 0, 0);
		REGISTER_PROPERTY_EX(rotation, GetRotation, SetRotation, GUI_INPUT(false, GUI_TABLE_GROUP_USER(Transform, 3, true)))
		mutable DirectX::XMFLOAT3 rotation = DirectX::XMFLOAT3(0, 0, 0);
		REGISTER_PROPERTY_EX(scale, GetScale, SetScale, GUI_INPUT(false, GUI_TABLE_GROUP_USER(Transform, 3, true)))
		mutable DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1, 1, 1);
		mutable DirectX::XMFLOAT4X4 world;
		mutable DirectX::XMFLOAT3 tFront;
		mutable DirectX::XMFLOAT3 tRight;
		mutable DirectX::XMFLOAT3 tUp;
	public:
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM;
		}
		DirectX::XMFLOAT3 GetPosition()const noexcept;
		//Euler
		DirectX::XMFLOAT3 GetRotation()const noexcept;
		DirectX::XMFLOAT4 GetQuaternion()const noexcept;
		DirectX::XMFLOAT3 GetScale()const noexcept;

		DirectX::XMVECTOR GetWorldQuaternion()const noexcept;

		DirectX::XMMATRIX GetWorldMatrix()const noexcept;
		DirectX::XMFLOAT4X4 GetWorld4x4f()const noexcept;
		DirectX::XMMATRIX GetLocal()const noexcept;
		DirectX::XMVECTOR GetRight()const noexcept;
		DirectX::XMVECTOR GetUp()const noexcept;
		DirectX::XMVECTOR GetFront()const noexcept;

		void SetTransform(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale)noexcept;
		void SetPosition(const DirectX::XMFLOAT3& value)noexcept;
		void SetRotation(const DirectX::XMFLOAT3& value)noexcept;
		void SetScale(const DirectX::XMFLOAT3& value)noexcept;

		void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& worldUp = DirectX::XMFLOAT3(0,1,0))noexcept;

		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void Update()noexcept;
		void WorldUpdate()noexcept;
		void ChangeParent()noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JTransform* transform);
		static JTransform* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterTypeData();
	private:
		JTransform(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner);
		~JTransform();
	};
*/