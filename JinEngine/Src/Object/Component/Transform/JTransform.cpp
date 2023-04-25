#include"JTransform.h"  
#include"JTransformPrivate.h"
#include"../JComponentHint.h"
#include"../../JFrameUpdate.h" 
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScenePrivate.h" 
#include"../../../Utility/JMathHelper.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Graphic/FrameResource/JObjectConstants.h"
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JTransformPrivate tPrivate;
	}
 
	class JTransform::JTransformImpl : public Core::JIdentifierImplBase,
		public JFrameDirtyChain<JFrameDirtyTrigger>
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JTransformImpl)
	public:
		JTransform* thisTrans = nullptr;
	public:
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
		JTransformImpl(const InitData& initData, JTransform* thisTrans)
			:thisTrans(thisTrans)
		{}
		~JTransformImpl(){}
	public:
		void Initialize()
		{
			XMStoreFloat3(&tRight, JMathHelper::VectorRight());
			XMStoreFloat3(&tUp, JMathHelper::VectorUp());
			XMStoreFloat3(&tFront, JMathHelper::VectorForward());
			XMStoreFloat4x4(&world, JMathHelper::IdentityMatrix4());

			if (!thisTrans->GetOwner()->IsRoot())
				UpdateWorld();
		}
	public:
		XMFLOAT3 GetPosition()const noexcept
		{
			return position;
		}
		XMFLOAT3 GetRotation()const noexcept
		{
			return rotation;
		}
		DirectX::XMFLOAT4 GetQuaternion()const noexcept
		{
			return JMathHelper::EulerToQuaternion(rotation);
		}
		XMFLOAT3 GetScale()const noexcept
		{
			return scale;
		}
		DirectX::XMVECTOR GetWorldQuaternion()const noexcept
		{
			XMVECTOR s;
			XMVECTOR q;
			XMVECTOR t;
			XMMatrixDecompose(&s, &q, &t, XMLoadFloat4x4(&world));
			return q;
		}
		XMMATRIX GetWorldMatrix()const noexcept
		{
			return XMLoadFloat4x4(&world);
		}
		DirectX::XMFLOAT4X4 GetWorld4x4f()const noexcept
		{
			return world;
		}
		XMMATRIX GetLocal()const noexcept
		{
			XMVECTOR s = XMLoadFloat3(&scale);
			XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x * (JMathHelper::Pi / 180),
				rotation.y * (JMathHelper::Pi / 180),
				rotation.z * (JMathHelper::Pi / 180));
			XMVECTOR t = XMLoadFloat3(&position);
			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			return XMMatrixAffineTransformation(s, zero, q, t);
		}
		XMVECTOR GetRight()const noexcept
		{
			return XMLoadFloat3(&tRight);
		}
		XMVECTOR GetUp()const noexcept
		{
			return XMLoadFloat3(&tUp);
		}
		XMVECTOR GetFront()const noexcept
		{
			return XMLoadFloat3(&tFront);
		}
	public:
		void SetTransform(const XMFLOAT3& nPosition, const XMFLOAT3& nRotation, const XMFLOAT3& nScale)noexcept
		{
			if (thisTrans->GetOwner()->IsRoot())
				return;

			position = nPosition;
			rotation = nRotation;
			scale = nScale;
			UpdateTopDown();
		}
		void SetPosition(const XMFLOAT3& value)noexcept
		{
			if (thisTrans->GetOwner()->IsRoot())
				return;

			position = value;
			UpdateTopDown();
		}
		void SetRotation(const XMFLOAT3& value)noexcept
		{
			if (thisTrans->GetOwner()->IsRoot())
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

			UpdateTopDown();
		}
		void SetScale(const XMFLOAT3& value)noexcept
		{
			if (thisTrans->GetOwner()->IsRoot())
				return;

			scale = value;
			UpdateTopDown();
		}
		void SetFrameDirtyTrigger()
		{
			SetFrameDirty();
		}
	public:
		void RegisterFrameDirtyListener(JFrameDirtyListener* newListener)
		{
			AddFrameDirtyListener(newListener);
		}
		void DeRegisterFrameDirtyListener(JFrameDirtyListener* oldListener)
		{
			RemoveFrameDirtyListener(oldListener);
		}
	public:
		void LookAt(const XMFLOAT3& target, const XMFLOAT3& worldUp)noexcept
		{
			if (thisTrans->GetOwner()->IsRoot())
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
			UpdateTopDown();
		}
	public:
		static bool DoCopy(JTransform* from, JTransform* to)
		{ 
			to->impl->position = from->impl->position;
			to->impl->rotation = from->impl->rotation;
			to->impl->scale = from->impl->scale;
			to->impl->tFront = from->impl->tFront;
			to->impl->tRight = from->impl->tRight;
			to->impl->tUp = from->impl->tUp;
			to->impl->UpdateTopDown();
			return true;
		}
	public:
		void UpdateTopDown()noexcept
		{
			JGameObject* owner = thisTrans->GetOwner();
			if (owner->IsRoot())
				return;

			UpdateWorld();
			JScenePrivate::CompSettingInterface::UpdateTransform(owner); 
			SetFrameDirty();
			const uint childrenCount = owner->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
				owner->GetChild(i)->GetTransform()->impl->UpdateTopDown();
		}
		void UpdateWorld()noexcept
		{
			const XMVECTOR s = XMLoadFloat3(&scale);
			const XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x * (JMathHelper::Pi / 180),
				rotation.y * (JMathHelper::Pi / 180),
				rotation.z * (JMathHelper::Pi / 180));
			const XMVECTOR t = XMLoadFloat3(&position);
			const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			const XMMATRIX worldM = XMMatrixMultiply(XMMatrixAffineTransformation(s, zero, q, t), XMLoadFloat4x4(&thisTrans->GetOwner()->GetParent()->GetTransform()->impl->world));
			XMStoreFloat4x4(&world, worldM);
		}
	public:
		static void RegisterCallOnce()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JTransform::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](JGameObject* parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JTransform::InitData>(ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JTransform::InitData>(parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static auto setFrameLam = [](JComponent* component) {static_cast<JTransform*>(component)->impl->SetFrameDirtyTrigger(); };
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable,isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable, nullptr };

			RegisterCTypeInfo(JTransform::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JTransform::StaticTypeInfo(), tPrivate);
		}
	};

	JTransform::InitData::InitData(JGameObject* owner)
		:JComponent::InitData(JTransform::StaticTypeInfo(), owner)
	{}
	JTransform::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		: JComponent::InitData(JTransform::StaticTypeInfo(), GetDefaultName(JTransform::StaticTypeInfo()), guid, flag, owner)

	{}
	Core::JIdentifierPrivate& JTransform::GetPrivateInterface()const noexcept
	{
		return tPrivate;
	}
	J_COMPONENT_TYPE JTransform::GetComponentType()const noexcept
	{  
		return GetStaticComponentType();
	}
	XMFLOAT3 JTransform::GetPosition()const noexcept
	{ 
		return impl->GetPosition();
	}
	XMFLOAT3 JTransform::GetRotation()const noexcept
	{
		return impl->GetRotation();
	}
	DirectX::XMFLOAT4 JTransform::GetQuaternion()const noexcept
	{
		return impl->GetQuaternion();
	}
	XMFLOAT3 JTransform::GetScale()const noexcept
	{
		return impl->GetScale();
	}
	DirectX::XMVECTOR JTransform::GetWorldQuaternion()const noexcept
	{
		return impl->GetWorldQuaternion();
	}
	XMMATRIX JTransform::GetWorldMatrix()const noexcept
	{
		return impl->GetWorldMatrix();
	}
	DirectX::XMFLOAT4X4 JTransform::GetWorld4x4f()const noexcept
	{
		return impl->GetWorld4x4f();
	}
	XMMATRIX JTransform::GetLocal()const noexcept
	{
		return impl->GetLocal();
	}
	XMVECTOR JTransform::GetRight()const noexcept
	{
		return impl->GetRight();
	}
	XMVECTOR JTransform::GetUp()const noexcept
	{
		return impl->GetUp();
	}
	XMVECTOR JTransform::GetFront()const noexcept
	{
		return impl->GetFront();
	}
	void JTransform::SetTransform(const XMFLOAT3& position, const XMFLOAT3& rotation, const XMFLOAT3& scale)noexcept
	{
		impl->SetTransform(position, rotation, scale);
	}
	void JTransform::SetPosition(const XMFLOAT3& value)noexcept
	{
		impl->SetPosition(value);
	}
	void JTransform::SetRotation(const XMFLOAT3& value)noexcept
	{
		impl->SetRotation(value);
	}
	void JTransform::SetScale(const XMFLOAT3& value)noexcept
	{
		impl->SetScale(value);
	}
	void JTransform::LookAt(const XMFLOAT3& target, const XMFLOAT3& worldUp)noexcept
	{
		impl->LookAt(target, worldUp);
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
	void JTransform::DoActivate()noexcept
	{
		JComponent::DoActivate();
		impl->SetFrameDirtyTrigger();
	}
	void JTransform::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate(); 
	}
	JTransform::JTransform(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JTransformImpl>(initData, this))
	{ 
		impl->Initialize();
	}
	JTransform::~JTransform()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JTransformPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JTransformPrivate::AssetDataIOInterface;
	using UpdateWorldInterface = JTransformPrivate::UpdateWorldInterface;
	using FrameDirtyInterface = JTransformPrivate::FrameDirtyInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JTransform>(*static_cast<JTransform::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JTransform::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JTransform::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JTransform::JTransformImpl::DoCopy(static_cast<JTransform*>(from), static_cast<JTransform*>(to));
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTransform::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		XMFLOAT3 pos;
		XMFLOAT3 rot;
		XMFLOAT3 scale;

		auto loadData = static_cast<JTransform::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JGameObject* owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadXMFloat3(stream, pos);
		JFileIOHelper::LoadXMFloat3(stream, rot);
		JFileIOHelper::LoadXMFloat3(stream, scale);
 
		auto rawPtr = tPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JTransform::InitData>(guid, flag, owner), &tPrivate);
		JTransform* newTransform = static_cast<JTransform*>(rawPtr);

		newTransform->SetPosition(pos);
		newTransform->SetRotation(rot);
		newTransform->SetScale(scale);
		newTransform->impl->SetFrameDirtyTrigger();

		return newTransform;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTransform::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JTransform::StoreData*>(data);
		if (!storeData->HasCorrectType(JTransform::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JTransform* transform = static_cast<JTransform*>(storeData->obj);
		JTransform::JTransformImpl* impl = transform->impl.get();
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, transform);
		JFileIOHelper::StoreXMFloat3(stream, L"Pos:", impl->position);
		JFileIOHelper::StoreXMFloat3(stream, L"Rot:", impl->rotation);
		JFileIOHelper::StoreXMFloat3(stream, L"Scale:", impl->scale);
		 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void UpdateWorldInterface::UpdateWorld(JTransform* transform)noexcept
	{
		transform->impl->UpdateTopDown();
	}

	void FrameDirtyInterface::RegisterFrameDirtyListener(JTransform* transform, JFrameDirtyListener* listener)noexcept
	{
		transform->impl->RegisterFrameDirtyListener(listener);
	}
	void FrameDirtyInterface::DeRegisterFrameDirtyListener(JTransform* transform, JFrameDirtyListener* listener)noexcept
	{
		transform->impl->DeRegisterFrameDirtyListener(listener);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JTransformPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JTransformPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}