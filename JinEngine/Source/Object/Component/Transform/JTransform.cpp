#include"JTransform.h"  
#include"JTransformPrivate.h"
#include"../JComponentHint.h"
#include"../../GameObject/JGameObject.h" 
#include"../../JObjectFileIOHelper.h"
#include"../../Resource/Scene/JScenePrivate.h"  
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/Frameresource/JObjectConstants.h"
#include"../../../Graphic/Frameresource/JFrameUpdate.h" 
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JTransformPrivate tPrivate;
	}

	class JTransform::JTransformImpl : public Core::JTypeImplBase,
		public Graphic::JFrameDirtyChain<Graphic::JFrameDirtyTrigger>
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JTransformImpl)
	public:
		JWeakPtr<JTransform> thisPointer = nullptr;
	public: 
		REGISTER_PROPERTY_EX(position, GetPosition, SetPosition, GUI_INPUT(false))
		mutable JVector3<float> position;
		REGISTER_PROPERTY_EX(rotation, GetRotation, SetRotation, GUI_INPUT(false))
		mutable JVector3<float> rotation;
		REGISTER_PROPERTY_EX(scale, GetScale, SetScale, GUI_INPUT(false))
		mutable JVector3<float> scale;
		mutable JMatrix4x4 world;
		mutable JVector3<float> tFront;
		mutable JVector3<float> tRight;
		mutable JVector3<float> tUp;
	public:
		JTransformImpl(const InitData& initData, JTransform* thisTransRaw)
		{}
		~JTransformImpl() {}
	public:
		JTransform* GetParent()const noexcept
		{
			return IsRoot() ? nullptr : thisPointer->GetOwner()->GetTransform().Get();
		}
		JVector3<float> GetPosition()const noexcept
		{ 
			return position;
		}
		JVector3<float> GetRotation()const noexcept
		{
			return rotation;
		}
		XMVECTOR GetQuaternion()const noexcept
		{ 
			return XMQuaternionRotationRollPitchYaw(rotation.x * (JMathHelper::Pi / 180),
				rotation.y * (JMathHelper::Pi / 180),
				rotation.z * (JMathHelper::Pi / 180));
		} 
		JVector3<float> GetScale()const noexcept
		{
			return scale;
		}
		JVector3<float> GetWorldPosition()const noexcept
		{
			return JVector3<float>(world._41, world._42, world._43);
		}
		JVector4<float> GetWorldQuaternion()const noexcept
		{ 
			XMVECTOR s;
			XMVECTOR q;
			XMVECTOR t;
			XMMatrixDecompose(&s, &q, &t, world.LoadXM());
			return q;
		}
		JMatrix4x4 GetWorldMatrix()const noexcept
		{
			return world;
		} 
		DirectX::XMMATRIX GetLocalMatrix()const noexcept
		{ 
			return XMMatrixAffineTransformation(scale.ToXmV(),
				XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
				GetQuaternion(),
				position.ToXmV());
		}
		JVector3<float> GetRight()const noexcept
		{
			return tRight;
		}
		JVector3<float> GetUp()const noexcept
		{
			return tUp;
		}
		JVector3<float> GetFront()const noexcept
		{
			return tFront;
		} 
		float GetDistance(const JUserPtr<JTransform>& t)const noexcept
		{ 
			return (t->GetWorldPosition() - GetWorldPosition()).Length(); 
		}
	public:
		void SetTransform(const JVector3<float>& nPosition, const JVector3<float>& nRotation, const JVector3<float>& nScale)noexcept
		{
			if (IsRoot() ||  (position == nPosition && rotation == nRotation && scale == nScale))
				return;

			SetPositionEx(nPosition, false);
			SetRotationEx(nRotation, false);
			SetScaleEx(nScale, false); 
			UpdateTopDown();
		}
		void SetPosition(const JVector3<float>& value)noexcept
		{
			SetPositionEx(value);
		}
		void SetPositionEx(const JVector3<float>& value, const bool updateTopDown = true)noexcept
		{
			position.SetNanToZero();
			if (IsRoot() || position == value)
				return;

			position = value;
			if (updateTopDown)
				UpdateTopDown();
		} 
		void SetRotation(const JVector3<float>& euler)noexcept
		{
			SetRotationEx(euler);
		}
		void SetRotationEx(const JVector3<float>& euler, const bool updateTopDown = true)noexcept
		{
			rotation.SetNanToZero();
			if (IsRoot() || rotation == euler)
				return;

			rotation = euler;
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

			const XMVECTOR q = GetQuaternion();

			tRight = XMVector3Rotate(JVector3F::Right().ToXmV(), q);
			tUp = XMVector3Rotate(JVector3F::Up().ToXmV(), q);
			tFront = XMVector3Rotate(JVector3F::Front().ToXmV(), q);
			if (updateTopDown)
				UpdateTopDown();
		}
		void SetScale(const JVector3<float>& value)noexcept
		{
			SetScaleEx(value);
		}
		void SetScaleEx(const JVector3<float>& value, const bool updateTopDown = true)noexcept
		{
			scale.SetNanToZero();
			if (thisPointer->GetOwner()->IsRoot() || scale == value)
				return;

			scale = value;
			if(updateTopDown)
				UpdateTopDown();
		}
		void SetFrameDirtyTrigger()
		{
			SetFrameDirty();
		}
	public:
		bool IsRoot()const noexcept
		{
			return thisPointer->GetOwner()->IsRoot();
		}
	public:
		void LookAt(const JVector3<float>& target, const JVector3<float>& worldUp)noexcept
		{
			if (IsRoot())
				return;
			 
			tFront = (target - position).Normalize();
			tRight = JVector3F::Cross(worldUp, tFront).Normalize();
			tUp = JVector3F::Cross(tFront, tRight).Normalize();		 
			
			JMatrix4x4 rotation4x4;
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
			const XMVECTOR newRotation = XMQuaternionRotationMatrix(rotation4x4.LoadXM());
			rotation = JMathHelper::ToEulerAngle(newRotation);
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
			JUserPtr<JGameObject> owner = thisPointer->GetOwner();
			if (owner->IsRoot())
				return;

			UpdateWorld();
			JScenePrivate::CompSettingInterface::UpdateTransform(thisPointer);
			const uint childrenCount = owner->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
				owner->GetChild(i)->GetTransform()->impl->UpdateTopDown();
			SetFrameDirty();
		}
		void UpdateWorld()noexcept
		{  
			world.StoreXM(XMMatrixMultiply(XMMatrixAffineTransformation(scale.ToXmV(), 
				XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
				GetQuaternion(), 
				position.ToXmV()),
				thisPointer->GetOwner()->GetParent()->GetTransform()->impl->world.LoadXM()));
		}
	public:
		void Initialize()
		{
			position = JVector3<float>(0, 0, 0);
			rotation = JVector3<float>(0, 0, 0);
			scale = JVector3<float>(1, 1, 1);

			tRight = JVector3<float>::Right();
			tUp = JVector3<float>::Up();
			tFront = JVector3<float>::Front();
			world = JMatrix4x4::Identity(); 

			if (!IsRoot())
				UpdateWorld();
		}
	public:
		void RegisterThisPointer(JTransform* trans)
		{
			thisPointer = Core::GetWeakPtr(trans);
		}
		void RegisterFrameDirtyListener(Graphic::JFrameDirtyTriggerBase* newListener, const size_t guid)
		{
			AddFrameDirtyListener(newListener, guid);
		}
		void DeRegisterFrameDirtyListener(const size_t guid)
		{
			RemoveFrameDirtyListener(guid);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JTransform::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](const Core::JTypeInfo& typeInfo, JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
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
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

			RegisterCTypeInfo(JTransform::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JTransform::StaticTypeInfo(), tPrivate);

			IMPL_REALLOC_BIND(JTransform::JTransformImpl, thisPointer)
		}
	};

	JTransform::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(JTransform::StaticTypeInfo(), owner)
	{}
	JTransform::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(JTransform::StaticTypeInfo(), GetDefaultName(JTransform::StaticTypeInfo()), guid, flag, owner)

	{}
	Core::JIdentifierPrivate& JTransform::PrivateInterface()const noexcept
	{
		return tPrivate;
	}
	J_COMPONENT_TYPE JTransform::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JVector3<float> JTransform::GetPosition()const noexcept
	{
		return impl->GetPosition();
	}
	JVector3<float> JTransform::GetRotation()const noexcept
	{
		return impl->GetRotation();
	}
	JVector4<float> JTransform::GetQuaternion()const noexcept
	{
		return impl->GetQuaternion();
	}
	JVector3<float> JTransform::GetScale()const noexcept
	{
		return impl->GetScale();
	}
	JVector3<float> JTransform::GetWorldPosition()const noexcept
	{
		return impl->GetWorldPosition();
	}
	JVector3<float> JTransform::GetWorldRotation()const noexcept
	{ 
		return JMathHelper::ToEulerAngle(GetWorldQuaternion());
	}
	JVector4<float> JTransform::GetWorldQuaternion()const noexcept
	{
		return impl->GetWorldQuaternion();
	}
	JMatrix4x4 JTransform::GetWorldMatrix()const noexcept
	{
		return impl->GetWorldMatrix();
	} 
	JVector3<float> JTransform::GetRight()const noexcept
	{
		return impl->GetRight();
	}
	JVector3<float> JTransform::GetUp()const noexcept
	{
		return impl->GetUp();
	}
	JVector3<float> JTransform::GetFront()const noexcept
	{
		return impl->GetFront();
	}
	JVector3<float> JTransform::GetWorldRight()const noexcept
	{ 
		return impl->IsRoot() ? impl->GetRight() : XMVector3Normalize(XMVector3Rotate(JVector3F::Right().ToXmV(), impl->GetParent()->GetWorldQuaternion().ToXmV()));
	}
	JVector3<float> JTransform::GetWorldUp()const noexcept
	{
		return impl->IsRoot() ? impl->GetUp() : XMVector3Normalize(XMVector3Rotate(JVector3F::Up().ToXmV(), impl->GetParent()->GetWorldQuaternion().ToXmV()));
	}
	JVector3<float> JTransform::GetWorldFront()const noexcept
	{
		return impl->IsRoot() ? impl->GetFront() : XMVector3Normalize(XMVector3Rotate(JVector3F::Front().ToXmV(), impl->GetParent()->GetWorldQuaternion().ToXmV()));
	}
	float JTransform::GetDistance(const JUserPtr<JTransform>& t)const noexcept
	{ 
		return impl->GetDistance(t);
	}
	void JTransform::GetWorldPQS(_Inout_ JVector3<float>& p, _Inout_ JVector4<float>& q, _Inout_ JVector3<float>& s)
	{
		XMVECTOR positionV;
		XMVECTOR rotationV;
		XMVECTOR scaleV;
		XMMatrixDecompose(&scaleV, &rotationV, &positionV, impl->GetWorldMatrix().LoadXM());

		p = positionV;
		q = rotationV;
		s = scaleV;
	}
	void JTransform::SetTransform(const JMatrix4x4& transform)noexcept
	{
		XMVECTOR positionV;
		XMVECTOR rotationV;
		XMVECTOR scaleV;
		XMMatrixDecompose(&scaleV, &rotationV, &positionV, transform.LoadXM());
		SetTransform(positionV, JMathHelper::ToEulerAngle(rotationV), scaleV);
	}
	void JTransform::SetTransform(const JVector3<float>& position, const JVector3<float>& rotation, const JVector3<float>& scale)noexcept
	{
		impl->SetTransform(position, rotation, scale);
	}
	void JTransform::SetTransform(const JVector3<float>& position, const JVector4<float>& quaternion, const JVector3<float>& scale)noexcept
	{
		impl->SetTransform(position, JMathHelper::ToEulerAngle(quaternion), scale);
	}
	void JTransform::SetPosition(const JVector3<float>& value)noexcept
	{
		impl->SetPosition(value);
	}
	void JTransform::SetRotation(const JVector3<float>& value)noexcept
	{
		impl->SetRotation(value);
	}
	void JTransform::SetRotation(const JVector4<float>& q)noexcept
	{
		impl->SetRotation(JMathHelper::ToEulerAngle(q));
	}
	void JTransform::SetScale(const JVector3<float>& value)noexcept
	{
		impl->SetScale(value);
	}
	void JTransform::CalTransformMatrix(_Out_ JMatrix4x4& m)
	{
		CalTransformMatrix(m, impl->thisPointer, impl->position, impl->tRight, impl->tUp, impl->tFront); 
	}
	void JTransform::CalTransformMatrix(_Out_ JMatrix4x4& m,
		const JUserPtr<JTransform>& t,
		const JVector3<float>& position,
		const JVector3<float>& right,
		const JVector3<float>& up,
		const JVector3<float>& front)
	{
		const XMVECTOR R = right.ToXmV();
		const XMVECTOR U = up.ToXmV();
		const XMVECTOR L = front.ToXmV();
		const XMVECTOR P = position.ToXmV();

		// Fill in the view matrix entries.
		const float x = -XMVectorGetX(XMVector3Dot(P, R));
		const float y = -XMVectorGetX(XMVector3Dot(P, U));
		const float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMFLOAT3 rightVector;
		XMFLOAT3 upVector;
		XMFLOAT3 lookVector;

		XMStoreFloat3(&rightVector, R);
		XMStoreFloat3(&upVector, U);
		XMStoreFloat3(&lookVector, L);

		m(0, 0) = rightVector.x;
		m(1, 0) = rightVector.y;
		m(2, 0) = rightVector.z;
		m(3, 0) = x;

		m(0, 1) = upVector.x;
		m(1, 1) = upVector.y;
		m(2, 1) = upVector.z;
		m(3, 1) = y;

		m(0, 2) = lookVector.x;
		m(1, 2) = lookVector.y;
		m(2, 2) = lookVector.z;
		m(3, 2) = z;

		m(0, 3) = 0.0f;
		m(1, 3) = 0.0f;
		m(2, 3) = 0.0f;
		m(3, 3) = 1.0f;

		if (!t->GetOwner()->IsRoot())
			m.StoreXM(XMMatrixMultiply(m.LoadXM(), t->GetOwner()->GetParent()->GetTransform()->impl->world.LoadXM()));
	}
	void JTransform::LookAt(const JVector3<float>& target, const JVector3<float>& worldUp)noexcept
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
	}
	JTransform::~JTransform()
	{ 
		impl.reset();
	}

	using CreateInstanceInterface = JTransformPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JTransformPrivate::AssetDataIOInterface;
	using UpdateWorldInterface = JTransformPrivate::UpdateWorldInterface;
	using FrameDirtyInterface = JTransformPrivate::FrameDirtyInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JTransform>(*static_cast<JTransform::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JTransform* trans = static_cast<JTransform*>(createdPtr);
		trans->impl->RegisterThisPointer(trans);
		trans->impl->Initialize();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JTransform::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JTransform::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JTransform::JTransformImpl::DoCopy(static_cast<JTransform*>(from.Get()), static_cast<JTransform*>(to.Get()));
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTransform::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;
		JVector3<float> pos;
		JVector3<float> rot;
		JVector3<float> scale;

		auto loadData = static_cast<JTransform::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		FILE_ASSERTION(JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated));
		FILE_ASSERTION(JObjectFileIOHelper::LoadVector3(tool, pos, "Pos:"));
		FILE_ASSERTION(JObjectFileIOHelper::LoadVector3(tool, rot, "Rot:"));
		FILE_ASSERTION(JObjectFileIOHelper::LoadVector3(tool, scale, "Scale:"));

		auto idenUser = tPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JTransform::InitData>(guid, flag, owner), &tPrivate);
		JUserPtr<JTransform> transUser = JUserPtr<JTransform>::ConvertChild(std::move(idenUser));

		transUser->SetPosition(pos);
		transUser->SetRotation(rot);
		transUser->SetScale(scale);
		transUser->impl->SetFrameDirtyTrigger();
		if (!isActivated)
			transUser->DeActivate();

		return transUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTransform::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JTransform::StoreData*>(data);
		if (!storeData->HasCorrectType(JTransform::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JTransform> transUser;
		transUser.ConnnectChild(storeData->obj);

		JTransform::JTransformImpl* impl = transUser->impl.get();
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, transUser.Get());
		JObjectFileIOHelper::StoreVector3(tool, impl->position, "Pos:");
		JObjectFileIOHelper::StoreVector3(tool, impl->rotation, "Rot:");
		JObjectFileIOHelper::StoreVector3(tool, impl->scale, "Scale:");

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void UpdateWorldInterface::UpdateWorld(const JUserPtr<JTransform>& transform)noexcept
	{
		transform->impl->UpdateTopDown();
	}

	void FrameDirtyInterface::RegisterFrameDirtyListener(JTransform* transform, Graphic::JFrameDirtyTriggerBase* listener, const size_t guid)noexcept
	{
		transform->impl->RegisterFrameDirtyListener(listener, guid);
	}
	void FrameDirtyInterface::DeRegisterFrameDirtyListener(JTransform* transform, const size_t guid)noexcept
	{
		transform->impl->DeRegisterFrameDirtyListener(guid);
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