#include"JRenderItem.h"  
#include"JRenderItemPrivate.h"
#include"../JComponentHint.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h"
#include"../../Resource/JResourceManager.h" 
#include"../../Resource/JResourceObjectUserInterface.h"
#include"../../Resource/Shader/JShaderFunctionEnum.h" 
#include"../../Resource/Mesh/JMeshGeometry.h"
#include"../../Resource/Material/JMaterial.h" 
#include"../../Resource/Material/JMaterialPrivate.h"  
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Utility/JCommonUtility.h" 
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/Frameresource/JObjectConstants.h"
#include"../../../Graphic/Frameresource/JBoundingObjectConstants.h"
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Frameresource/JOcclusionConstants.h" 
#include<fstream>
   
using namespace DirectX;
namespace JinEngine
{
	namespace
	{ 
		using RitemFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder3< 
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, Graphic::JObjectConstants&, const uint>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, Graphic::JBoundingObjectConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, Graphic::JHzbOccObjectConstants&>>,
			Graphic::JFrameDirty>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JRenderItemPrivate rPrivate;
	}
 
	class JRenderItem::JRenderItemImpl : public Core::JTypeImplBase,
		public RitemFrameUpdate,
		public JResourceObjectUserInterface
	{ 
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JRenderItemImpl)
	public:
		using ObjectFrame = JFrameInterface1;
		using BoundingObjectFrame = JFrameInterface2;
		using OccObjectFrame = JFrameInterface3;
	public:
		JWeakPtr<JRenderItem> thisPointer = nullptr;
	public:
		REGISTER_PROPERTY_EX(mesh, GetMesh, SetMesh, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		JUserPtr<JMeshGeometry> mesh;
		REGISTER_PROPERTY_EX(material, GetMaterialVec, SetMaterialVec, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		std::vector<JUserPtr<JMaterial>> material; 
	public:
		JMatrix4x4 textureTransform = JMatrix4x4::Identity();
		D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT; 
	public:
		J_RENDERITEM_ACCELERATOR_MASK acceleratorMask = ACCELERATOR_ALLOW_ALL;
	public:
		bool isActivated = false;
		REGISTER_PROPERTY_EX(isOccluder, IsOccluder, SetOccluder, GUI_CHECKBOX())
		bool isOccluder = false;
	public:
		JRenderItemImpl(const InitData& initData, JRenderItem* thisRitemRaw)
			:renderLayer(initData.layer), acceleratorMask(initData.acceleratorMask)
		{}
		~JRenderItemImpl(){ }
	public:
		JUserPtr<JMeshGeometry> GetMesh()const noexcept
		{ 
			sizeof(JRenderItemImpl);
			return mesh;
		}
		JUserPtr<JMaterial> GetValidMaterial(int index)const noexcept
		{
			if (material.size() <= index)
				return JUserPtr<JMaterial>{};
			else
			{
				if (material[index].IsValid())
					return material[index];
				else
					return mesh->GetSubmeshMaterial(index);
			}
		}
		std::vector<JUserPtr<JMaterial>> GetMaterialVec()const noexcept
		{
			return material;
		}
		REGISTER_METHOD(GetTotalVertexCount)
		REGISTER_METHOD_READONLY_GUI_WIDGET(VertexCount, GetTotalVertexCount, GUI_READONLY_TEXT())
		uint GetTotalVertexCount()const noexcept
		{
			return mesh.IsValid() ? mesh->GetTotalVertexCount() : 0;
		}
		REGISTER_METHOD(GetTotalIndexCount)
		REGISTER_METHOD_READONLY_GUI_WIDGET(IndexCount, GetTotalIndexCount, GUI_READONLY_TEXT())
		uint GetTotalIndexCount()const noexcept
		{
			return mesh.IsValid() ? mesh->GetTotalIndexCount() : 0;
		}
		uint GetSubmeshCount()const noexcept
		{
			return mesh.IsValid() ? mesh->GetTotalSubmeshCount() : 0;
		}
		DirectX::BoundingBox GetBoundingBox()const noexcept
		{
			if (mesh.IsValid())
			{
				DirectX::BoundingBox res;
				mesh->GetBoundingBox().Transform(res, thisPointer->GetOwner()->GetTransform()->GetWorldMatrix());
				return res;
			}
			else
				return DirectX::BoundingBox();
		}
		DirectX::BoundingOrientedBox GetOrientedBoundingBox()const noexcept
		{
			if (mesh.IsValid())
			{
				DirectX::BoundingOrientedBox oriBB;
				DirectX::BoundingOrientedBox res;
				DirectX::BoundingOrientedBox::CreateFromBoundingBox(oriBB, mesh->GetBoundingBox());
			 
				oriBB.Transform(res, thisPointer->GetOwner()->GetTransform()->GetWorldMatrix());
				return res;
			}
			else
				return DirectX::BoundingOrientedBox();
		}
		DirectX::BoundingSphere GetBoundingSphere()const noexcept
		{
			if (mesh.IsValid())
			{
				JTransform* ownerTransform = thisPointer->GetOwner()->GetTransform().Get(); 
				XMVECTOR s;
				XMVECTOR q;
				XMVECTOR t;
				XMMatrixDecompose(&s, &q, &t, ownerTransform->GetWorldMatrix());

				JVector3<float> pos = t;
				JVector3<float> scale = s;
				 
				JVector3<float> meshSphereCenter = mesh->GetBoundingSphereCenter();
				float meshSphereRad = mesh->GetBoundingSphereRadius();

				JVector3<float> gameObjSphereCenter = meshSphereCenter + pos;
				float gameObjSphereRad = meshSphereRad;

				if (scale.x >= scale.y && scale.x >= scale.z)
					gameObjSphereRad *= scale.x;
				else if (scale.y >= scale.x && scale.y >= scale.z)
					gameObjSphereRad *= scale.y;
				else
					gameObjSphereRad *= scale.z;

				return DirectX::BoundingSphere(gameObjSphereCenter.ToXmF(), gameObjSphereRad);
			}
			else
				return DirectX::BoundingSphere();
		}
	public:
		void SetMesh(JUserPtr<JMeshGeometry> newMesh)noexcept
		{ 
			if (thisPointer->IsActivated())
			{
				CallOffResourceReference(mesh.Get());
				if (isActivated)
				{
					DeRegisterComponent(thisPointer);
					DeActivate();
				}
			}
			mesh = newMesh;
			if (thisPointer->IsActivated())
			{
				CallOnResourceReference(mesh.Get());
				if (!isActivated && mesh != nullptr)
				{
					RegisterComponent(thisPointer);
					Activate();
				}
			}

			//material.clear();
			if (mesh.IsValid())
				material.resize(mesh->GetTotalSubmeshCount());
			SetFrameDirty();
		}
		void SetMaterial(int index, JUserPtr<JMaterial> newMaterial)noexcept
		{
			if (material.size() <= index)
				return;

			if (thisPointer->IsActivated() && material[index].IsValid())
				CallOffResourceReference(material[index].Get());
			material[index] = newMaterial;
			if (thisPointer->IsActivated() && material[index].IsValid())
				CallOnResourceReference(material[index].Get());
			SetFrameDirty();
		}
		void SetMaterialVec(const std::vector< JUserPtr<JMaterial>> newVec)noexcept
		{
			const uint vecCount = (uint)newVec.size();
			for (uint i = 0; i < vecCount; ++i)
				SetMaterial(i, newVec[i]);
		}
		void SetRenderLayer(const J_RENDER_LAYER newRenderLayer)noexcept
		{
			if (renderLayer != newRenderLayer)
			{
				if (thisPointer->IsActivated())
					DeRegisterComponent(thisPointer);
				renderLayer = newRenderLayer;
				if (thisPointer->IsActivated())
					RegisterComponent(thisPointer);
			}
		}
		void SetAcceleratorMask(const J_RENDERITEM_ACCELERATOR_MASK newAcceleratorMask)noexcept
		{
			if (acceleratorMask != newAcceleratorMask)
			{
				if (thisPointer->IsActivated())
					DeRegisterComponent(thisPointer);
				acceleratorMask = newAcceleratorMask;
				if (thisPointer->IsActivated())
					RegisterComponent(thisPointer);
			}
		}
		void SetOccluder(const bool value)noexcept
		{ 
			isOccluder = value;
		}
	public:
		bool IsOccluder()const noexcept
		{
			return isOccluder;
		}
	public:
		static bool DoCopy(JRenderItem* from, JRenderItem* to)
		{
			from->SetMesh(to->impl->mesh);
			from->impl->textureTransform = to->impl->textureTransform;
			from->SetPrimitiveType(to->impl->primitiveType);
			from->SetRenderLayer(to->impl->renderLayer);
			from->impl->SetFrameDirty();
			return true;
		}
	public:
		void OnResourceRef()
		{
			SetFrameDirty();
			CallOnResourceReference(mesh.Get());
			if (mesh.IsValid())
				material.resize(mesh->GetTotalSubmeshCount());
			const uint matCount = (uint)material.size();
			for (uint i = 0; i < matCount; ++i)
				CallOnResourceReference(material[i].Get());
		}
		void OffResourceRef()
		{
			SetFrameDirty();
			CallOffResourceReference(mesh.Get());
			const uint matCount = (uint)material.size();
			for (uint i = 0; i < matCount; ++i)
				CallOffResourceReference(material[i].Get());
		}
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (mesh.IsValid() && mesh->GetGuid() == jRobj->GetGuid())
					SetMesh(JUserPtr<JMeshGeometry>{});
				else
				{
					const uint matCount = (uint)material.size();
					for (uint i = 0; i < matCount; ++i)
					{
						if (material[i].IsValid() && material[i]->GetGuid() == jRobj->GetGuid())
							SetMaterial(i, JUserPtr<JMaterial>{});
					}
				}
			}
		}
	public:
		void Activate()
		{  
			RegisterRItemFrameData();
			isActivated = true;
		}
		void DeActivate()
		{
			DeRegisterRItemFrameData();
			isActivated = false;
		}
	public:
		void UpdateFrame(Graphic::JObjectConstants& constant, const uint submeshIndex)noexcept final
		{
			JTransform* transform = thisPointer->GetOwner()->GetTransform().Get();
			constant.world.StoreXM(XMMatrixTranspose(transform->GetWorldMatrix()));
			constant.texTransform.StoreXM(XMMatrixTranspose(textureTransform.LoadXM()));
			constant.materialIndex = JMaterialPrivate::FrameIndexInterface::GetMaterialFrameIndex(GetValidMaterial(submeshIndex).Get());
			ObjectFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JBoundingObjectConstants& constant)noexcept final
		{
			const BoundingBox meshBBox = mesh->GetBoundingBox();
			static const BoundingBox drawBBox = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE)->GetBoundingBox();
			//static const BoundingBox drawBBox = _JResourceManager::Instance().Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::CUBE)->GetBoundingBox();

			JTransform* transform = thisPointer->GetOwner()->GetTransform().Get();

			const JVector3<float> meshCenter(meshBBox.Center);
			const JVector3<float> drawBBoxCenter(drawBBox.Center);
			const JVector3<float> meshExtents(meshBBox.Extents);
			const JVector3<float> drawBBoxExtents(drawBBox.Extents);

			const JVector3<float> scale = transform->GetScale();
			//const XMVECTOR s = ((JVector3<float>(bbox.Extents) / drawBBox.Extents) * (scale * 1.001f)).ToXmV();
			 
			const XMVECTOR s = ((meshExtents / drawBBoxExtents) * scale).ToXmV();
			const XMVECTOR q = transform->GetQuaternion();

			const JVector3<float> pos = transform->GetPosition(); 
			const XMVECTOR t = ((meshCenter - drawBBoxCenter) + pos).ToXmV();
			 
			const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			const XMMATRIX worldM = XMMatrixMultiply(XMMatrixAffineTransformation(s, zero, q, t), thisPointer->GetOwner()->GetParent()->GetTransform()->GetWorldMatrix());
			  
			constant.boundWorld.StoreXM(XMMatrixTranspose(worldM));
			BoundingObjectFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JHzbOccObjectConstants& constant)noexcept final
		{
			const DirectX::BoundingOrientedBox bbox = GetOrientedBoundingBox();
			bbox.GetCorners(constant.coners);
			constant.center = bbox.Center;
			constant.extents = bbox.Extents;
			constant.isValid = renderLayer == J_RENDER_LAYER::OPAQUE_OBJECT;
			constant.queryResultIndex = OccObjectFrame::GetFrameIndex();
			OccObjectFrame::MinusMovedDirty();
		}
	public:
		void NotifyReAlloc()
		{
			auto transform = thisPointer->GetOwner()->GetTransform();
			if (transform.IsValid())
			{
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(transform.Get(), thisPointer->GetGuid());
				JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(transform.Get(), this, thisPointer->GetGuid());
			}
			ObjectFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, (ObjectFrame*)this);
			BoundingObjectFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, (BoundingObjectFrame*)this);
			OccObjectFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, (OccObjectFrame*)this);
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void RegisterThisPointer(JRenderItem* rItem)
		{
			thisPointer = Core::GetWeakPtr(rItem);
		}
		void RegisterPostCreation()
		{
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), this, thisPointer->GetGuid());
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid());
			if (thisPointer->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), thisPointer->GetGuid());
		} 
		void RegisterRItemFrameData()
		{ 
			ObjectFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, (ObjectFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), mesh->GetTotalSubmeshCount());
			BoundingObjectFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, (BoundingObjectFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
			OccObjectFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, (OccObjectFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void DeRegisterRItemFrameData()
		{  
			ObjectFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, (ObjectFrame*)this);
			BoundingObjectFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, (BoundingObjectFrame*)this);
			OccObjectFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, (OccObjectFrame*)this);		
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JRenderItem::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](const Core::JTypeInfo& typeInfo, JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JRenderItem::InitData>(ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JRenderItem::InitData>(parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static auto setFrameDirtyLam = [](JComponent* component) {static_cast<JRenderItem*>(component)->impl->SetFrameDirty(); };
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameDirtyLam }; 
			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable,isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable};

			RegisterCTypeInfo(JRenderItem::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JRenderItem::StaticTypeInfo(), rPrivate);
			 
			IMPL_REALLOC_BIND(JRenderItem::JRenderItemImpl, thisPointer)
		}
	};

	JRenderItem::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(JRenderItem::StaticTypeInfo(), owner)
	{}
	JRenderItem::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(JRenderItem::StaticTypeInfo(), GetDefaultName(JRenderItem::StaticTypeInfo()), guid, flag, owner)
	{}
	JRenderItem::InitData::InitData(const size_t guid,
		const J_OBJECT_FLAG flag,
		const JUserPtr<JGameObject>& owner,
		const J_RENDER_LAYER layer,
		const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask)
		: JComponent::InitData(JRenderItem::StaticTypeInfo(), GetDefaultName(JRenderItem::StaticTypeInfo()), guid, flag, owner),
		layer(layer),
		acceleratorMask(acceleratorMask)
	{}
	Core::JIdentifierPrivate& JRenderItem::PrivateInterface()const noexcept
	{
		return rPrivate;
	}
	J_COMPONENT_TYPE JRenderItem::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JUserPtr<JMeshGeometry> JRenderItem::GetMesh()const noexcept
	{
		return impl->GetMesh();
	}
	JUserPtr<JMaterial> JRenderItem::GetValidMaterial(int index)const noexcept
	{
		return impl->GetValidMaterial(index);
	}
	std::vector<JUserPtr<JMaterial>> JRenderItem::GetMaterialVec()const noexcept
	{
		return impl->GetMaterialVec();
	}
	JMatrix4x4 JRenderItem::GetTextransform()const noexcept
	{
		return impl->textureTransform;
	}
	D3D12_PRIMITIVE_TOPOLOGY JRenderItem::GetPrimitiveType()const noexcept
	{
		return impl->primitiveType;
	}
	J_RENDER_LAYER JRenderItem::GetRenderLayer()const noexcept
	{
		return impl->renderLayer;
	}
	J_RENDERITEM_ACCELERATOR_MASK JRenderItem::GetAcceleratorMask()const noexcept
	{
		return impl->acceleratorMask;
	}
	uint JRenderItem::GetTotalVertexCount()const noexcept
	{
		return impl->GetTotalVertexCount();
	}
	uint JRenderItem::GetTotalIndexCount()const noexcept
	{
		return impl->GetTotalIndexCount();
	}
	uint JRenderItem::GetSubmeshCount()const noexcept
	{
		return impl->GetSubmeshCount();
	}
	DirectX::BoundingBox JRenderItem::GetBoundingBox()const noexcept
	{
		return impl->GetBoundingBox();
	}
	DirectX::BoundingOrientedBox JRenderItem::GetOrientedBoundingBox()const noexcept
	{
		return impl->GetOrientedBoundingBox();
	}
	DirectX::BoundingSphere JRenderItem::GetBoundingSphere()const noexcept
	{
		return impl->GetBoundingSphere();
	}
	void JRenderItem::SetMesh(JUserPtr<JMeshGeometry> newMesh)noexcept
	{
		impl->SetMesh(newMesh);
	}
	void JRenderItem::SetMaterial(int index, JUserPtr<JMaterial> newMaterial)noexcept
	{
		impl->SetMaterial(index, newMaterial);
	}
	void JRenderItem::SetMaterialVec(const std::vector< JUserPtr<JMaterial>> newVec)noexcept
	{
		impl->SetMaterialVec(newVec);
	}
	void JRenderItem::SetTextureTransform(const JMatrix4x4& textureTransform)noexcept
	{
		impl->textureTransform = textureTransform;
	}
	void JRenderItem::SetPrimitiveType(const D3D12_PRIMITIVE_TOPOLOGY primitiveType)noexcept
	{
		impl->primitiveType = primitiveType;
	}
	void JRenderItem::SetRenderLayer(const J_RENDER_LAYER renderLayer)noexcept
	{
		impl->SetRenderLayer(renderLayer);
	}
	void JRenderItem::SetAcceleratorMask(const J_RENDERITEM_ACCELERATOR_MASK acceleratorMask)noexcept
	{
		impl->SetAcceleratorMask(acceleratorMask);
	}
	void JRenderItem::SetOccluder(const bool value)noexcept
	{
		impl->SetOccluder(value);
	}
	bool JRenderItem::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
	}
	bool JRenderItem::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JRenderItem::IsOccluder()const noexcept
	{
		return impl->IsOccluder();
	}
	bool JRenderItem::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && impl->mesh.IsValid())
			return true;
		else
			return false;
	}
	void JRenderItem::DoActivate()noexcept
	{ 
		JComponent::DoActivate();
		if (!impl->isActivated && impl->mesh != nullptr && RegisterComponent(impl->thisPointer))
			impl->Activate();
		impl->OnResourceRef();
	}
	void JRenderItem::DoDeActivate()noexcept
	{
		if (impl->isActivated)
		{
			DeRegisterComponent(impl->thisPointer);
			impl->DeActivate();
		}
		impl->OffResourceRef();
		impl->OffFrameDirty();
		JComponent::DoDeActivate();
	}
	JRenderItem::JRenderItem(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JRenderItemImpl>(initData, this))
	{ }
	JRenderItem::~JRenderItem()
	{ 
		impl.reset();
	}

	using CreateInstanceInterface = JRenderItemPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JRenderItemPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JRenderItemPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JRenderItemPrivate::FrameIndexInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JRenderItem>(*static_cast<JRenderItem::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JRenderItem* rItem = static_cast<JRenderItem*>(createdPtr);
		rItem->impl->RegisterThisPointer(rItem);
		rItem->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JRenderItem::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JRenderItem::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JRenderItem::JRenderItemImpl::DoCopy(static_cast<JRenderItem*>(from.Get()), static_cast<JRenderItem*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JRenderItem*>(ptr)->impl->DeRegisterPreDestruction();
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JRenderItem::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;

		D3D12_PRIMITIVE_TOPOLOGY primitiveType;
		J_RENDER_LAYER renderLayer;
		J_RENDERITEM_ACCELERATOR_MASK acceleratorMask;
		uint materialCount;
		bool isOccluder;

		auto loadData = static_cast<JRenderItem::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(stream, guid, flag, isActivated);
		JUserPtr<JMeshGeometry> mesh = JObjectFileIOHelper::_LoadHasIden<JMeshGeometry>(stream);
		JObjectFileIOHelper::LoadEnumData(stream, primitiveType);
		JObjectFileIOHelper::LoadEnumData(stream, renderLayer);
		JObjectFileIOHelper::LoadEnumData(stream, acceleratorMask);
		JObjectFileIOHelper::LoadAtomicData(stream, materialCount);
		JObjectFileIOHelper::LoadAtomicData(stream, isOccluder);

		std::vector<JUserPtr<JMaterial>>materialVec(materialCount);
		for (uint i = 0; i < materialCount; ++i)
			materialVec[i] = JObjectFileIOHelper::_LoadHasIden<JMaterial>(stream);

		auto idenUser = rPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JRenderItem::InitData>(guid, flag, owner), &rPrivate);
		JUserPtr<JRenderItem> rUser;
		rUser.ConnnectChild(idenUser); 
		rUser->SetMesh(mesh);
		rUser->SetPrimitiveType(primitiveType);
		rUser->SetRenderLayer(renderLayer);
		rUser->SetAcceleratorMask(acceleratorMask);
		rUser->impl->material.resize(materialCount);
		for (uint i = 0; i < materialCount; ++i)
			rUser->SetMaterial(i, materialVec[i]);
		rUser->SetOccluder(isOccluder);
		if (!isActivated)
			rUser->DeActivate();

		return rUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JRenderItem::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JRenderItem::StoreData*>(data);
		if (!storeData->HasCorrectType(JRenderItem::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JRenderItem> rUser;
		rUser.ConnnectChild(storeData->obj);

		JRenderItem::JRenderItemImpl* impl = rUser->impl.get();
		std::wofstream& stream = storeData->stream;

		JObjectFileIOHelper::StoreComponentIden(stream, rUser.Get());
		JObjectFileIOHelper::_StoreHasIden(stream, impl->mesh.Get());
		JObjectFileIOHelper::StoreEnumData(stream, L"PrimitiveType:", impl->primitiveType);
		JObjectFileIOHelper::StoreEnumData(stream, L"RenderLayer:", impl->renderLayer);
		JObjectFileIOHelper::StoreEnumData(stream, L"AcceleratorMask:", impl->acceleratorMask);
		JObjectFileIOHelper::StoreAtomicData(stream, L"MaterialCount:", impl->material.size()); 
		JObjectFileIOHelper::StoreAtomicData(stream, L"IsOccluder:", impl->isOccluder);

		for (uint i = 0; i < impl->material.size(); ++i)
			JObjectFileIOHelper::_StoreHasIden(stream, impl->material[i].Get());

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JRenderItem* rItem, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			rItem->impl->SetFrameDirty();

		rItem->impl->SetLastFrameUpdatedTrigger(false);
		rItem->impl->SetLastFrameHotUpdatedTrigger(false);
		return rItem->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JRenderItem* rItem, Graphic::JObjectConstants& constant, const uint submeshIndex)noexcept
	{
		rItem->impl->UpdateFrame(constant, submeshIndex);
	}
	void FrameUpdateInterface::UpdateFrame(JRenderItem* rItem, Graphic::JBoundingObjectConstants& constant)noexcept
	{
		rItem->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JRenderItem* rItem, Graphic::JHzbOccObjectConstants& constant)noexcept
	{
		rItem->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateEnd(JRenderItem* rItem)noexcept
	{
		if (rItem->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			rItem->impl->SetLastFrameHotUpdatedTrigger(true);
		rItem->impl->SetLastFrameUpdatedTrigger(true);
		rItem->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetObjectFrameIndex(JRenderItem* rItem)noexcept
	{
		return rItem->impl->ObjectFrame::GetFrameIndex(); 
	} 
	int FrameUpdateInterface::GetBoundingFrameIndex(JRenderItem* rItem)noexcept
	{
		return rItem->impl->BoundingObjectFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetOccObjectFrameIndex(JRenderItem* rItem)noexcept
	{
		return rItem->impl->OccObjectFrame::GetFrameIndex();
	}
	bool FrameUpdateInterface::IsLastFrameHotUpdated(JRenderItem* rItem)noexcept
	{
		return rItem->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JRenderItem* rItem)noexcept
	{
		return rItem->impl->IsLastFrameUpdated();
	}
	bool FrameUpdateInterface::HasObjectRecopyRequest(JRenderItem* rItem)noexcept
	{
		return rItem->impl->ObjectFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasBoundingRecopyRequest(JRenderItem* rItem)noexcept
	{
		return rItem->impl->BoundingObjectFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasOccObjectRecopyRequest(JRenderItem* rItem)noexcept
	{
		return rItem->impl->OccObjectFrame::HasMovedDirty();
	}

	int FrameIndexInterface::GetObjectFrameIndex(JRenderItem* rItem)noexcept
	{
		return rItem->impl->ObjectFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetBoundingFrameIndex(JRenderItem* rItem)noexcept
	{
		return rItem->impl->BoundingObjectFrame::GetFrameIndex();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JRenderItemPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JRenderItemPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JRenderItemPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}