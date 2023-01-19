#include"JTransform.h"    
#include"../JComponentFactory.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"
#include"../../../Utility/JMathHelper.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Graphic/FrameResource/JObjectConstants.h"
#include<fstream>

namespace JinEngine
{
	static auto isAvailableoverlapLam = []() {return false; };

	using namespace DirectX;
	J_COMPONENT_TYPE JTransform::GetComponentType()const noexcept
	{  
		return GetStaticComponentType();
	}
	XMFLOAT3 JTransform::GetPosition()const noexcept
	{ 
		return position;
	}
	XMFLOAT3 JTransform::GetRotation()const noexcept
	{
		return rotation;
	}
	DirectX::XMFLOAT4 JTransform::GetQuaternion()const noexcept
	{
		return JMathHelper::EulerToQuaternion(rotation);
	}
	XMFLOAT3 JTransform::GetScale()const noexcept
	{
		return scale;
	}
	DirectX::XMVECTOR JTransform::GetWorldQuaternion()const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t;
		XMMatrixDecompose(&s, &q, &t, XMLoadFloat4x4(&world));
		return q;
	}
	XMMATRIX JTransform::GetWorld()const noexcept
	{
		return XMLoadFloat4x4(&world);
	}
	XMMATRIX JTransform::GetLocal()const noexcept
	{
		XMVECTOR s = XMLoadFloat3(&scale);
		XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x * (JMathHelper::Pi / 180),
			rotation.y * (JMathHelper::Pi / 180),
			rotation.z * (JMathHelper::Pi / 180));
		XMVECTOR t = XMLoadFloat3(&position);
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		return XMMatrixAffineTransformation(s, zero, q, t);
	}
	XMVECTOR JTransform::GetRight()const noexcept
	{
		return XMLoadFloat3(&tRight);
	}
	XMVECTOR JTransform::GetUp()const noexcept
	{
		return XMLoadFloat3(&tUp);
	}
	XMVECTOR JTransform::GetFront()const noexcept
	{
		return XMLoadFloat3(&tFront);
	}
	void JTransform::SetTransform(const XMFLOAT3& position, const XMFLOAT3& rotation, const XMFLOAT3& scale)noexcept
	{
		if (GetOwner()->IsRoot())
			return;

		JTransform::position = position;
		JTransform::rotation = rotation;
		JTransform::scale = scale;
		Update();  
	}
	void JTransform::SetPosition(const XMFLOAT3& value)noexcept
	{
		if (GetOwner()->IsRoot())
			return;

		position = value;
		Update(); 
	}
	void JTransform::SetRotation(const XMFLOAT3& value)noexcept
	{
		if (GetOwner()->IsRoot())
			return;

		rotation = value;

		if (rotation.x >= 360)
			rotation.x -= 360;
		if (rotation.y >= 360)
			rotation.y -= 360;
		if (rotation.z >= 360)
			rotation.z -= 360;

		if (rotation.x <= -360)
			rotation.x += 360;
		if (rotation.y <= -360)
			rotation.y += 360;
		if (rotation.z <= -360)
			rotation.z += 360;

		const XMVECTOR q = XMQuaternionRotationRollPitchYaw(JMathHelper::DegToRad * rotation.x,
			JMathHelper::DegToRad * rotation.y,
			JMathHelper::DegToRad * rotation.z);

		XMVECTOR newRight = XMVector3Rotate(JMathHelper::VectorRight(), q);
		XMVECTOR newUp = XMVector3Rotate(JMathHelper::VectorUp(), q);
		XMVECTOR newFront = XMVector3Rotate(JMathHelper::VectorForward(), q);
  
		XMStoreFloat3(&tRight, XMVector3Normalize(newRight));
		XMStoreFloat3(&tUp, XMVector3Normalize(newUp));
		XMStoreFloat3(&tFront, XMVector3Normalize(newFront));

		Update(); 
	}
	void JTransform::SetScale(const XMFLOAT3& value)noexcept
	{
		if (GetOwner()->IsRoot())
			return;

		scale = value;
		Update(); 
	}
	void JTransform::LookAt(const XMFLOAT3& target, const XMFLOAT3& worldUp)noexcept
	{
		if (GetOwner()->IsRoot())
			return;

		const XMVECTOR positionVec = XMLoadFloat3(&position);
		const XMVECTOR targetVec = XMLoadFloat3(&target);
		const XMVECTOR upVec = XMLoadFloat3(&worldUp);

		const XMVECTOR l = XMVector3Normalize(XMVectorSubtract(targetVec, positionVec));
		const XMVECTOR r = XMVector3Normalize(XMVector3Cross(upVec, l));
		const XMVECTOR u = XMVector3Cross(l, r);

		XMStoreFloat3(&tFront, l);
		XMStoreFloat3(&tRight, r);
		XMStoreFloat3(&tUp, u);

		DirectX::XMFLOAT4X4 rotation4x4;
		rotation4x4(0, 0) = tRight.x;
		rotation4x4(1, 0) = tRight.y;
		rotation4x4(2, 0) = tRight.z;
		rotation4x4(3, 0) = 0;

		rotation4x4(0, 1) = tUp.x;
		rotation4x4(1, 1) = tUp.y;
		rotation4x4(2, 1) = tUp.z;
		rotation4x4(3, 1) = 0;

		rotation4x4(0, 2) = tFront.x;
		rotation4x4(1, 2) = tFront.y;
		rotation4x4(2, 2) = tFront.z;
		rotation4x4(3, 2) = 0;

		rotation4x4(0, 3) = 0.0f;
		rotation4x4(1, 3) = 0.0f;
		rotation4x4(2, 3) = 0.0f;
		rotation4x4(3, 3) = 1.0f;

		//수정필요
		const XMVECTOR newRotation = XMQuaternionRotationMatrix(XMLoadFloat4x4(&rotation4x4));
		//rotation = JMathHelper::ToEulerAngle(newRotation);
		Update();
	}
	bool JTransform::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JTransform::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	void JTransform::DoCopy(JObject* ori)
	{
		JTransform* oriT = static_cast<JTransform*>(ori);
		position = oriT->position;
		rotation = oriT->rotation;
		scale = oriT->scale;
		tFront = oriT->tFront;
		tRight = oriT->tRight;
		tUp = oriT->tUp;
		Update();
	}
	void JTransform::DoActivate()noexcept
	{
		JComponent::DoActivate();
		SetFrameDirty();
	}
	void JTransform::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate(); 
	}
	void JTransform::Update()noexcept
	{
		JGameObject* owner = GetOwner();
		if (owner->IsRoot())
			return;

		WorldUpdate();
		GetOwner()->GetOwnerScene()->CompInterface()->UpdateTransform(GetOwner());
		SetFrameDirty();
		const uint childrenCount = owner->GetChildrenCount();
		for (uint i = 0; i < childrenCount; ++i)
			owner->GetChild(i)->GetTransform()->Update();
	}
	void JTransform::WorldUpdate()noexcept
	{
		const XMVECTOR s = XMLoadFloat3(&scale);
		const XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x * (JMathHelper::Pi / 180),
			rotation.y * (JMathHelper::Pi / 180),
			rotation.z * (JMathHelper::Pi / 180));
		const XMVECTOR t = XMLoadFloat3(&position);
		const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const XMMATRIX worldM = XMMatrixMultiply(XMMatrixAffineTransformation(s, zero, q, t), XMLoadFloat4x4(&GetOwner()->GetParent()->GetTransform()->world));
		XMStoreFloat4x4(&world, worldM);
	}
	void JTransform::ChangeParent()noexcept
	{
		Update();
	}
	Core::J_FILE_IO_RESULT JTransform::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JTransform::StoreObject(std::wofstream& stream, JTransform* transform)
	{
		if (transform == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)transform->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::StoreObjectIden(stream, transform);
		JFileIOHelper::StoreXMFloat3(stream, L"Pos:", transform->position);
		JFileIOHelper::StoreXMFloat3(stream, L"Rot:", transform->rotation);
		JFileIOHelper::StoreXMFloat3(stream, L"Scale:", transform->scale);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JTransform* JTransform::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open() || stream.eof())
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		XMFLOAT3 pos;
		XMFLOAT3 rot;
		XMFLOAT3 scale;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadXMFloat3(stream, pos);
		JFileIOHelper::LoadXMFloat3(stream, rot);
		JFileIOHelper::LoadXMFloat3(stream, scale);

		Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTransform>(guid, flag, owner);
		JTransform* newTransform = ownerPtr.Get();
		if (!AddInstance(std::move(ownerPtr)))
			return nullptr;

		newTransform->SetPosition(pos);
		newTransform->SetRotation(rot);
		newTransform->SetScale(scale);
		newTransform->SetFrameDirty();
		return newTransform;
	}
	void JTransform::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			const J_OBJECT_FLAG defaultFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE);
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTransform>(Core::MakeGuid(),
				Core::AddSQValueEnum(owner->GetFlag(), defaultFlag), owner);
			JTransform* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{ 
			const J_OBJECT_FLAG defaultFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE);
			const J_OBJECT_FLAG finalFlag = Core::AddSQValueEnum(owner->GetFlag(), Core::AddSQValueEnum(objFlag, defaultFlag));
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTransform>(guid, finalFlag, owner);
			JTransform* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTransform>(Core::MakeGuid(), ori->GetFlag(), owner);
			JTransform* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				if (newComp->Copy(ori))
					return newComp;
				else
				{
					BegineForcedDestroy(newComp);
					return nullptr;
				}
			}
			else
				return nullptr;
		};
		JCFI<JTransform>::Register(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JTransform::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JTransform::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

		static auto setFrameLam = [](JComponent& component) {static_cast<JTransform*>(&component)->SetFrameDirty(); };
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{ getTypeNameCallable, getTypeInfoCallable,isAvailableOverlapCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ &setFrameDirtyCallable, nullptr };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JTransform::JTransform(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		:JTransformInterface(TypeName(), guid, flag, owner)
	{
		XMStoreFloat3(&tRight, JMathHelper::VectorRight());
		XMStoreFloat3(&tUp, JMathHelper::VectorUp());
		XMStoreFloat3(&tFront, JMathHelper::VectorForward());
		XMStoreFloat4x4(&world, JMathHelper::IdentityMatrix4());

		if (!owner->IsRoot())
			WorldUpdate();
	}
	JTransform::~JTransform() {}
}