#include"JRenderItem.h"  
#include"JRenderItemPrivate.h"
#include"../JComponentHint.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../../JFrameUpdate.h"
#include"../../GameObject/JGameObject.h"
#include"../../Resource/JResourceManager.h" 
#include"../../Resource/JResourceObjectUserInterface.h"
#include"../../Resource/Shader/JShaderFunctionEnum.h" 
#include"../../Resource/Mesh/JMeshGeometry.h"
#include"../../Resource/Material/JMaterial.h" 
#include"../../Resource/Material/JMaterialPrivate.h" 
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Graphic/FrameResource/JObjectConstants.h"
#include"../../../Graphic/FrameResource/JBoundingObjectConstants.h"
#include"../../../Graphic/OcclusionCulling/JOcclusionConstants.h"
#include"../../../Utility/JCommonUtility.h" 
#include"../../../Utility/JMathHelper.h" 
#include<fstream>

//Debug  
//#include"../../../Debug/JDebugTextOut.h"
using namespace DirectX;
namespace JinEngine
{
	namespace
	{
		using RitemFrameUpdate = JFrameUpdate2<JFrameUpdateBase<Graphic::JObjectConstants&, const uint>,
			JFrameUpdateBase<Graphic::JBoundingObjectConstants&>>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JRenderItemPrivate rPrivate;
	}
 
	class JRenderItem::JRenderItemImpl : public Core::JIdentifierImplBase,
		public JFrameUpdate<RitemFrameUpdate, JFrameDirtyListener, FrameUpdate::dobuleBuff>,
		public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JRenderItemImpl)
	public:
		JRenderItem* thisRitem = nullptr;
	public:
		REGISTER_PROPERTY_EX(mesh, GetMesh, SetMesh, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		Core::JUserPtr<JMeshGeometry> mesh;
		REGISTER_PROPERTY_EX(material, GetMaterialVec, SetMaterialVec, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		std::vector<Core::JUserPtr<JMaterial>> material;
		DirectX::XMFLOAT4X4 textureTransform = JMathHelper::Identity4x4();
		D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		J_RENDER_LAYER renderLayer = J_RENDER_LAYER::OPAQUE_OBJECT;
		J_RENDER_VISIBILITY renderVisibility = J_RENDER_VISIBILITY::VISIBLE;
	public:
		J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask = SPACE_SPATIAL_ALLOW_ALL;
	public:
		JRenderItemImpl(const InitData& initData, JRenderItem* thisRitem)
			:JResourceObjectUserInterface(thisRitem->GetGuid()), thisRitem(thisRitem)
		{
			AddEventListener(*JResourceObject::EvInterface(), thisRitem->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisRitem->GetOwner()->GetTransform(), this);
		}
		~JRenderItemImpl()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisRitem->GetGuid());
			if (thisRitem->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisRitem->GetOwner()->GetTransform(), this);
		}
	public:
		Core::JUserPtr<JMeshGeometry> GetMesh()const noexcept
		{
			return mesh;
		}
		Core::JUserPtr<JMaterial> GetValidMaterial(int index)const noexcept
		{
			if (material.size() <= index)
				return Core::JUserPtr<JMaterial>{};
			else
			{
				if (material[index].IsValid())
					return material[index];
				else
					return mesh->GetSubmeshMaterial(index);
			}
		}
		std::vector<Core::JUserPtr<JMaterial>> GetMaterialVec()const noexcept
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
		DirectX::BoundingBox GetBoundingBox()noexcept
		{
			if (mesh.IsValid())
			{
				DirectX::BoundingBox res;
				mesh->GetBoundingBox().Transform(res, thisRitem->GetOwner()->GetTransform()->GetWorldMatrix());
				return res;
			}
			else
				return DirectX::BoundingBox();
		}
		DirectX::BoundingSphere GetBoundingSphere()noexcept
		{
			if (mesh.IsValid())
			{
				JTransform* ownerTransform = thisRitem->GetOwner()->GetTransform();
				XMMATRIX worldM = ownerTransform->GetWorldMatrix();
				XMVECTOR s;
				XMVECTOR q;
				XMVECTOR t;
				XMMatrixDecompose(&s, &q, &t, worldM);

				XMFLOAT3 pos;
				XMFLOAT3 scale;

				XMStoreFloat3(&pos, t);
				XMStoreFloat3(&scale, s);

				XMFLOAT3 meshSphereCenter = mesh->GetBoundingSphereCenter();
				float meshSphereRad = mesh->GetBoundingSphereRadius();

				XMFLOAT3 gameObjSphereCenter = XMFLOAT3(meshSphereCenter.x + pos.x,
					meshSphereCenter.y + pos.y,
					meshSphereCenter.z + pos.z);

				float gameObjSphereRad = meshSphereRad;

				if (scale.x >= scale.y && scale.x >= scale.z)
					gameObjSphereRad *= scale.x;
				else if (scale.y >= scale.x && scale.y >= scale.z)
					gameObjSphereRad *= scale.y;
				else
					gameObjSphereRad *= scale.z;

				return DirectX::BoundingSphere(gameObjSphereCenter, gameObjSphereRad);
			}
			else
				return DirectX::BoundingSphere();
		}
		JFrameBuff2* GetFrameBuffInterface()
		{
			return this;
		}
	public:
		void SetMesh(Core::JUserPtr<JMeshGeometry> newMesh)noexcept
		{
			if (thisRitem->IsActivated())
				CallOffResourceReference(mesh.Get());
			mesh = newMesh;
			if (thisRitem->IsActivated())
				CallOnResourceReference(mesh.Get());

			//material.clear();
			if (mesh.IsValid())
				material.resize(mesh->GetTotalSubmeshCount());

			if (thisRitem->IsActivated())
				thisRitem->DeRegisterComponent();
			if (thisRitem->IsActivated())
				thisRitem->RegisterComponent();
			//if (preMesh == nullptr && IsActivated())
			//	RegisterComponent();
			SetFrameDirty();
		}
		void SetMaterial(int index, Core::JUserPtr<JMaterial> newMaterial)noexcept
		{
			if (material.size() <= index)
				return;

			if (thisRitem->IsActivated() && material[index].IsValid())
				CallOffResourceReference(material[index].Get());
			material[index] = newMaterial;
			if (thisRitem->IsActivated() && material[index].IsValid())
				CallOnResourceReference(material[index].Get());
			SetFrameDirty();
		}
		void SetMaterialVec(const std::vector< Core::JUserPtr<JMaterial>> newVec)noexcept
		{
			const uint vecCount = (uint)newVec.size();
			for (uint i = 0; i < vecCount; ++i)
				SetMaterial(i, newVec[i]);
		}
		void SetRenderLayer(const J_RENDER_LAYER newRenderLayer)noexcept
		{
			if (renderLayer != newRenderLayer)
			{
				if (thisRitem->IsActivated())
					thisRitem->DeRegisterComponent();
				renderLayer = newRenderLayer;
				if (thisRitem->IsActivated())
					thisRitem->RegisterComponent();
			}
		}
		void SetSpaceSpatialMask(const J_RENDERITEM_SPACE_SPATIAL_MASK newSpaceSpatialMask)noexcept
		{
			if (spaceSpatialMask != newSpaceSpatialMask)
			{
				if ((spaceSpatialMask & SPACE_SPATIAL_ALLOW_CULLING) == 0)
					thisRitem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);

				if (thisRitem->IsActivated())
					thisRitem->DeRegisterComponent();
				spaceSpatialMask = newSpaceSpatialMask;
				if (thisRitem->IsActivated())
					thisRitem->RegisterComponent();
			}
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
			if (iden == thisRitem->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (mesh.IsValid() && mesh->GetGuid() == jRobj->GetGuid())
					SetMesh(Core::JUserPtr<JMeshGeometry>{});
				else
				{
					const uint matCount = (uint)material.size();
					for (uint i = 0; i < matCount; ++i)
					{
						if (material[i].IsValid() && material[i]->GetGuid() == jRobj->GetGuid())
							SetMaterial(i, Core::JUserPtr<JMaterial>{});
					}
				}
			}
		}
	public:
		void UpdateFrame(Graphic::JObjectConstants& constant, const uint submeshIndex)noexcept final
		{
			JTransform* transform = thisRitem->GetOwner()->GetTransform();
			XMStoreFloat4x4(&constant.World, XMMatrixTranspose(transform->GetWorldMatrix()));
			XMStoreFloat4x4(&constant.TexTransform, XMMatrixTranspose(XMLoadFloat4x4(&textureTransform)));
			constant.MaterialIndex = JMaterialPrivate::FrameBuffInterface::GetCBOffset(GetValidMaterial(submeshIndex).Get());
		}
		void UpdateFrame(Graphic::JBoundingObjectConstants& constant)noexcept final
		{
			const BoundingBox bbox = mesh->GetBoundingBox();
			static const BoundingBox drawBBox = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE)->GetBoundingBox();
			//static const BoundingBox drawBBox = _JResourceManager::Instance().Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE)->GetBoundingBox();

			JTransform* transform = thisRitem->GetOwner()->GetTransform();

			const XMFLOAT3 objScale = transform->GetScale();
			const XMFLOAT3 bboxScale = XMFLOAT3((bbox.Extents.x / drawBBox.Extents.x) * objScale.x,
				(bbox.Extents.y / drawBBox.Extents.y) * objScale.y,
				(bbox.Extents.z / drawBBox.Extents.z) * objScale.z);
			const XMVECTOR s = XMLoadFloat3(&bboxScale);

			const XMFLOAT3 bboxRotation = transform->GetRotation();
			const XMVECTOR q = XMQuaternionRotationRollPitchYaw(bboxRotation.x * (JMathHelper::Pi / 180),
				bboxRotation.y * (JMathHelper::Pi / 180),
				bboxRotation.z * (JMathHelper::Pi / 180));

			const XMFLOAT3 objPos = transform->GetPosition();
			const XMFLOAT3 bboxPos = XMFLOAT3(bbox.Center.x - drawBBox.Center.x + objPos.x,
				bbox.Center.y - drawBBox.Center.y + objPos.y,
				bbox.Center.z - drawBBox.Center.z + objPos.z);
			const XMVECTOR t = XMLoadFloat3(&bboxPos);

			const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			const XMMATRIX worldM = XMMatrixMultiply(XMMatrixAffineTransformation(s, zero, q, t), thisRitem->GetOwner()->GetParent()->GetTransform()->GetWorldMatrix());

			XMStoreFloat4x4(&constant.boundWorld, XMMatrixTranspose(worldM));
		}
	public:
		static void RegisterCallOnce()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JRenderItem::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](JGameObject* parent, InitUnq&& parentClassInitData) -> InitUnq
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
			static auto setFrameOffsetLam = [](JComponent* component, JComponent* refComp, const bool isCreated)
			{
				JRenderItem* rItem = static_cast<JRenderItem*>(component);
				JFrameBuff2* rFrameBase = rItem->impl->GetFrameBuffInterface();
				if (isCreated)
				{
					if (refComp == nullptr)
					{  
						rFrameBase->SetFirstFrameBuffOffset(0);
						rFrameBase->SetSecondFrameBuffOffset(0);
					}
					else
					{
						JRenderItem* refRItem = static_cast<JRenderItem*>(refComp);
						JFrameBuff2* refFrameBase = refRItem->impl->GetFrameBuffInterface();
						rFrameBase->SetFirstFrameBuffOffset(refFrameBase->GetFirstFrameBuffOffset() + refRItem->GetSubmeshCount());
						rFrameBase->SetSecondFrameBuffOffset(refFrameBase->GetSecondFrameBuffOffset() + 1);
					}
				}
				else
				{
					rFrameBase->SetFirstFrameBuffOffset(rFrameBase->GetFirstFrameBuffOffset() - static_cast<JRenderItem*>(refComp)->GetSubmeshCount());
					rFrameBase->SetSecondFrameBuffOffset(rFrameBase->GetSecondFrameBuffOffset() - 1);
				}
			};
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameDirtyLam };
			static SetCFrameOffsetCallable setFrameOffsetCallable{ setFrameOffsetLam };
			static CTypeHint cTypeHint{ GetStaticComponentType(), true, true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable,isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable, &setFrameOffsetCallable };

			RegisterCTypeInfo(JRenderItem::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JRenderItem::StaticTypeInfo(), rPrivate);
		}
	};

	JRenderItem::InitData::InitData(JGameObject* owner)
		:JComponent::InitData(JRenderItem::StaticTypeInfo(), owner)
	{}
	JRenderItem::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		: JComponent::InitData(JRenderItem::StaticTypeInfo(), GetDefaultName(JRenderItem::StaticTypeInfo()), guid, flag, owner)
	{}


	Core::JIdentifierPrivate& JRenderItem::GetPrivateInterface()const noexcept
	{
		return rPrivate;
	}
	JFrameUpdateUserAccess JRenderItem::GetFrameUserInterface() noexcept
	{
		return JFrameUpdateUserAccess(Core::GetUserPtr(this), impl.get());
	}
	J_COMPONENT_TYPE JRenderItem::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	Core::JUserPtr<JMeshGeometry> JRenderItem::GetMesh()const noexcept
	{
		return impl->GetMesh();
	}
	Core::JUserPtr<JMaterial> JRenderItem::GetValidMaterial(int index)const noexcept
	{
		return impl->GetValidMaterial(index);
	}
	std::vector<Core::JUserPtr<JMaterial>> JRenderItem::GetMaterialVec()const noexcept
	{
		return impl->GetMaterialVec();
	}
	DirectX::XMFLOAT4X4 JRenderItem::GetTextransform()const noexcept
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
	J_RENDERITEM_SPACE_SPATIAL_MASK JRenderItem::GetSpaceSpatialMask()const noexcept
	{
		return impl->spaceSpatialMask;
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
	DirectX::BoundingBox JRenderItem::GetBoundingBox()noexcept
	{
		return impl->GetBoundingBox();
	}
	DirectX::BoundingSphere JRenderItem::GetBoundingSphere()noexcept
	{
		return impl->GetBoundingSphere();
	}
	void JRenderItem::SetMesh(Core::JUserPtr<JMeshGeometry> newMesh)noexcept
	{
		impl->SetMesh(newMesh);
	}
	void JRenderItem::SetMaterial(int index, Core::JUserPtr<JMaterial> newMaterial)noexcept
	{
		impl->SetMaterial(index, newMaterial);
	}
	void JRenderItem::SetMaterialVec(const std::vector< Core::JUserPtr<JMaterial>> newVec)noexcept
	{
		impl->SetMaterialVec(newVec);
	}
	void JRenderItem::SetTextureTransform(const DirectX::XMFLOAT4X4& textureTransform)noexcept
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
	void JRenderItem::SetRenderVisibility(const J_RENDER_VISIBILITY renderVisibility)noexcept
	{
		impl->renderVisibility = renderVisibility;
	}
	void JRenderItem::SetSpaceSpatialMask(const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)noexcept
	{
		impl->SetSpaceSpatialMask(spaceSpatialMask);
	}
	bool JRenderItem::IsVisible()const noexcept
	{
		return impl->renderVisibility == J_RENDER_VISIBILITY::VISIBLE;
	}
	bool JRenderItem::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
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
		RegisterComponent();
		impl->OnResourceRef();
	}
	void JRenderItem::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
		impl->OffResourceRef();
		impl->OffFrameDirty();
	}
	JRenderItem::JRenderItem(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JRenderItemImpl>(initData, this))
	{ }
	JRenderItem::~JRenderItem()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JRenderItemPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JRenderItemPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JRenderItem>(*static_cast<JRenderItem::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JRenderItem::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JRenderItem::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JRenderItem::JRenderItemImpl::DoCopy(static_cast<JRenderItem*>(from), static_cast<JRenderItem*>(to));
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JRenderItem::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		D3D12_PRIMITIVE_TOPOLOGY primitiveType;
		J_RENDER_LAYER renderLayer;
		J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask;
		uint materialCount;

		auto loadData = static_cast<JRenderItem::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JGameObject* owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		Core::JUserPtr<JMeshGeometry> mesh = JFileIOHelper::LoadHasObjectIden<JMeshGeometry>(stream);
		JFileIOHelper::LoadEnumData(stream, primitiveType);
		JFileIOHelper::LoadEnumData(stream, renderLayer);
		JFileIOHelper::LoadEnumData(stream, spaceSpatialMask);
		JFileIOHelper::LoadAtomicData(stream, materialCount);

		std::vector<Core::JUserPtr<JMaterial>>materialVec(materialCount);
		for (uint i = 0; i < materialCount; ++i)
			materialVec[i] = JFileIOHelper::LoadHasObjectIden<JMaterial>(stream);

		auto rawPtr = rPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JRenderItem::InitData>(guid, flag, owner), &rPrivate);
		JRenderItem* newRItem = static_cast<JRenderItem*>(rawPtr);
		
		newRItem->SetMesh(mesh);
		newRItem->SetPrimitiveType(primitiveType);
		newRItem->SetRenderLayer(renderLayer);
		newRItem->SetSpaceSpatialMask(spaceSpatialMask);
		newRItem->impl->material.resize(materialCount);
		for (uint i = 0; i < materialCount; ++i)
			newRItem->SetMaterial(i, materialVec[i]);

		return newRItem;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JRenderItem::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JRenderItem::StoreData*>(data);
		if (!storeData->HasCorrectType(JRenderItem::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JRenderItem* rItem = static_cast<JRenderItem*>(storeData->obj);
		JRenderItem::JRenderItemImpl* impl = rItem->impl.get();
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, rItem);
		JFileIOHelper::StoreHasObjectIden(stream, impl->mesh.Get());
		JFileIOHelper::StoreEnumData(stream, L"PrimitiveType:", impl->primitiveType);
		JFileIOHelper::StoreEnumData(stream, L"RenderLayer:", impl->renderLayer);
		JFileIOHelper::StoreEnumData(stream, L"SpaceSpatialMask:", impl->spaceSpatialMask);
		JFileIOHelper::StoreAtomicData(stream, L"MaterialCount:", impl->material.size());

		for (uint i = 0; i < impl->material.size(); ++i)
			JFileIOHelper::StoreHasObjectIden(stream, impl->material[i].Get());

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JRenderItem* rItem, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			rItem->impl->SetFrameDirty();

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
	void FrameUpdateInterface::UpdateEnd(JRenderItem* rItem)noexcept
	{
		rItem->impl->UpdateEnd();
	}
	bool FrameUpdateInterface::IsHotUpdated(JRenderItem* rItem)noexcept
	{
		return rItem->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources;
	}
	uint FrameUpdateInterface::GetBoundingCBOffset(JRenderItem* rItem)noexcept
	{
		return rItem->impl->GetSecondFrameBuffOffset();
	}
	uint FrameUpdateInterface::GetObjectCBBuffOffset(JRenderItem* rItem)noexcept
	{
		return rItem->impl->GetFirstFrameBuffOffset();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JRenderItemPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JRenderItemPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}