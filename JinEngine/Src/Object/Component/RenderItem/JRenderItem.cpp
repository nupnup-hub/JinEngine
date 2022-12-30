#include"JRenderItem.h" 
#include"../JComponentFactory.h"
#include"../Transform/JTransform.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/JResourceManager.h" 
#include"../../Resource/Shader/JShaderFunctionEnum.h"  
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileConstant.h"
#include"../../../Graphic/FrameResource/JObjectConstants.h"
#include"../../../Graphic/FrameResource/JBoundingObjectConstants.h"
#include"../../../Graphic/OcclusionCulling/JOcclusionConstants.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Application/JApplicationVariable.h"
#include<fstream>

//Debug
#include"../../../Debug/JDebugTextOut.h"
using namespace DirectX;
namespace JinEngine
{
	static auto isAvailableoverlapLam = []() {return false; };

	J_COMPONENT_TYPE JRenderItem::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JMeshGeometry* JRenderItem::GetMesh()const noexcept
	{
		return mesh;
	}
	JMaterial* JRenderItem::GetValidMaterial(int index)const noexcept
	{
		if (material.size() <= index)
			return nullptr;
		else
		{
			if (material[index])
				return material[index];
			else
				return mesh->GetSubmeshMaterial(index);
		}
	}
	std::vector<JMaterial*> JRenderItem::GetMaterialVec()const noexcept
	{
		return material;
	}
	DirectX::XMFLOAT4X4 JRenderItem::GetTextransform()const noexcept
	{
		return textureTransform;
	}
	D3D12_PRIMITIVE_TOPOLOGY JRenderItem::GetPrimitiveType()const noexcept
	{
		return primitiveType;
	}
	J_RENDER_LAYER JRenderItem::GetRenderLayer()const noexcept
	{
		return renderLayer;
	}
	J_RENDERITEM_SPACE_SPATIAL_MASK JRenderItem::GetSpaceSpatialMask()const noexcept
	{
		return spaceSpatialMask;
	}
	uint JRenderItem::GetVertexTotalCount()const noexcept
	{
		return mesh != nullptr ? mesh->GetTotalVertexCount() : 0;
	}
	uint JRenderItem::GetIndexTotalCount()const noexcept
	{
		return mesh != nullptr ? mesh->GetTotalIndexCount() : 0;
	}
	uint JRenderItem::GetSubmeshCount()const noexcept
	{
		return mesh != nullptr ? mesh->GetTotalSubmeshCount() : 0;
	}
	DirectX::BoundingBox JRenderItem::GetBoundingBox(bool onDebug)noexcept
	{
		if (mesh != nullptr)
		{
			DirectX::BoundingBox res;
			mesh->GetBoundingBox().Transform(res, GetOwner()->GetTransform()->GetWorld());
			return res;
		}
		else
			return DirectX::BoundingBox();
	}
	DirectX::BoundingSphere JRenderItem::GetBoundingSphere()noexcept
	{
		if (mesh != nullptr)
		{
			JTransform* ownerTransform = GetOwner()->GetTransform();
			XMMATRIX worldM = ownerTransform->GetWorld();
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
	void JRenderItem::SetMesh(JMeshGeometry* newMesh)noexcept
	{
		JMeshGeometry* preMesh = mesh;
		if (IsActivated())
			CallOffResourceReference(mesh);
		mesh = newMesh;
		if (IsActivated())
			CallOnResourceReference(mesh);

		//material.clear();
		if (mesh != nullptr)
			material.resize(mesh->GetTotalSubmeshCount());

		if (IsActivated())
			DeRegisterComponent();
		if (IsActivated())
			RegisterComponent();
		//if (preMesh == nullptr && IsActivated())
		//	RegisterComponent();
		SetFrameDirty();
	}
	void JRenderItem::SetMaterial(int index, JMaterial* newMaterial)noexcept
	{
		if (material.size() <= index)
			return;

		if (IsActivated())
			CallOffResourceReference(material[index]);
		material[index] = newMaterial;
		if (IsActivated())
			CallOnResourceReference(material[index]);
		SetFrameDirty();
	}
	void JRenderItem::SetMaterialVec(const std::vector<JMaterial*>& newVec)noexcept
	{
		const uint vecCount = (uint)newVec.size();
		for (uint i = 0; i < vecCount; ++i)
			SetMaterial(i, newVec[i]);
	}
	void JRenderItem::SetTextureTransform(const DirectX::XMFLOAT4X4& textureTransform)noexcept
	{
		JRenderItem::textureTransform = textureTransform;
	}
	void JRenderItem::SetPrimitiveType(const D3D12_PRIMITIVE_TOPOLOGY primitiveType)noexcept
	{
		JRenderItem::primitiveType = primitiveType;
	}
	void JRenderItem::SetRenderLayer(const J_RENDER_LAYER renderLayer)noexcept
	{
		if (JRenderItem::renderLayer != renderLayer)
		{
			if (IsActivated())
				DeRegisterComponent();
			JRenderItem::renderLayer = renderLayer;
			if (IsActivated())
				RegisterComponent();
		}
	}
	void JRenderItem::SetRenderVisibility(const J_RENDER_VISIBILITY renderVisibility)noexcept
	{
		JRenderItem::renderVisibility = renderVisibility;
	}
	void JRenderItem::SetSpaceSpatialMask(const J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask)noexcept
	{
		if (JRenderItem::spaceSpatialMask != spaceSpatialMask)
		{
			if ((JRenderItem::spaceSpatialMask & SPACE_SPATIAL_ALLOW_CULLING) == 0)
				SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);

			if (IsActivated())
				DeRegisterComponent();
			JRenderItem::spaceSpatialMask = spaceSpatialMask;
			if (IsActivated())
				RegisterComponent();
		}
	}
	bool JRenderItem::IsVisible()const noexcept
	{
		return renderVisibility == J_RENDER_VISIBILITY::VISIBLE;
	}
	bool JRenderItem::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JRenderItem::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && mesh != nullptr)
			return true;
		else
			return false;
	}
	void JRenderItem::DoCopy(JObject* ori)
	{
		JRenderItem* oriR = static_cast<JRenderItem*>(ori);
		SetMesh(oriR->mesh);
		textureTransform = oriR->textureTransform;
		SetPrimitiveType(oriR->primitiveType);
		SetRenderLayer(oriR->renderLayer);
		SetFrameDirty();
	}
	void JRenderItem::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent();
		SetFrameDirty();
		CallOnResourceReference(mesh);
		if (mesh != nullptr)
			material.resize(mesh->GetTotalSubmeshCount());
		const uint matCount = (uint)material.size();
		for (uint i = 0; i < matCount; ++i)
			CallOnResourceReference(material[i]);
	}
	void JRenderItem::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
		OffFrameDirty();
		CallOffResourceReference(mesh);
		const uint matCount = (uint)material.size();
		for (uint i = 0; i < matCount; ++i)
			CallOffResourceReference(material[i]);
	}
	void JRenderItem::UpdateFrame(Graphic::JObjectConstants& constant, const uint submeshIndex)
	{
		JTransform* transform = GetOwner()->GetTransform();
		XMStoreFloat4x4(&constant.World, XMMatrixTranspose(transform->GetWorld()));
		XMStoreFloat4x4(&constant.TexTransform, XMMatrixTranspose(XMLoadFloat4x4(&textureTransform)));
		constant.MaterialIndex = CallGetFrameBuffOffset(*GetValidMaterial(submeshIndex));
	}
	void JRenderItem::UpdateFrame(Graphic::JBoundingObjectConstants& constant)
	{
		const BoundingBox bbox = mesh->GetBoundingBox();
		static const BoundingBox drawBBox = JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE)->GetBoundingBox();
		//static const BoundingBox drawBBox = JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE)->GetBoundingBox();

		JTransform* transform = GetOwner()->GetTransform();

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
		const XMMATRIX worldM = XMMatrixMultiply(XMMatrixAffineTransformation(s, zero, q, t), GetOwner()->GetParent()->GetTransform()->GetWorld());

		XMStoreFloat4x4(&constant.boundWorld, XMMatrixTranspose(worldM));
	}
	void JRenderItem::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			if (mesh != nullptr && mesh->GetGuid() == jRobj->GetGuid())
				SetMesh(nullptr);
			else
			{
				const uint matCount = (uint)material.size();
				for (uint i = 0; i < matCount; ++i)
				{
					if (material[i] != nullptr && material[i]->GetGuid() == jRobj->GetGuid())
						SetMaterial(i, nullptr);
				}
			}
		}
	}
	Core::J_FILE_IO_RESULT JRenderItem::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JRenderItem::StoreObject(std::wofstream& stream, JRenderItem* renderItem)
	{
		if (renderItem == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)renderItem->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::StoreObjectIden(stream, renderItem);
		JFileIOHelper::StoreHasObjectIden(stream, renderItem->mesh);
		JFileIOHelper::StoreEnumData(stream, L"PrimitiveType:", renderItem->primitiveType);
		JFileIOHelper::StoreEnumData(stream, L"RenderLayer:", renderItem->renderLayer);
		JFileIOHelper::StoreEnumData(stream, L"SpaceSpatialMask:", renderItem->spaceSpatialMask);
		JFileIOHelper::StoreAtomicData(stream, L"MaterialCount:", renderItem->material.size());

		for (uint i = 0; i < renderItem->material.size(); ++i)
			JFileIOHelper::StoreHasObjectIden(stream, renderItem->material[i]);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JRenderItem* JRenderItem::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open() || stream.eof())
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		D3D12_PRIMITIVE_TOPOLOGY primitiveType;
		J_RENDER_LAYER renderLayer;
		J_RENDERITEM_SPACE_SPATIAL_MASK spaceSpatialMask;
		uint materialCount;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		Core::JIdentifier* mesh = JFileIOHelper::LoadHasObjectIden(stream);
		JFileIOHelper::LoadEnumData(stream, primitiveType);
		JFileIOHelper::LoadEnumData(stream, renderLayer);
		JFileIOHelper::LoadEnumData(stream, spaceSpatialMask);
		JFileIOHelper::LoadAtomicData(stream, materialCount);

		std::vector<Core::JIdentifier*>materialVec(materialCount);
		for (uint i = 0; i < materialCount; ++i)
			materialVec[i] = JFileIOHelper::LoadHasObjectIden(stream);

		Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JRenderItem>(guid, flag, owner);
		JRenderItem* newRenderItem = ownerPtr.Get();
		if (!AddInstance(std::move(ownerPtr)))
			return nullptr;

		if (mesh != nullptr && mesh->GetTypeInfo().IsChildOf(JMeshGeometry::StaticTypeInfo()))
			newRenderItem->SetMesh(static_cast<JMeshGeometry*>(mesh));

		newRenderItem->SetPrimitiveType(primitiveType);
		newRenderItem->SetRenderLayer(renderLayer);
		newRenderItem->SetSpaceSpatialMask(spaceSpatialMask);
		newRenderItem->material.resize(materialCount);
		for (uint i = 0; i < materialCount; ++i)
		{
			if (materialVec[i] != nullptr && materialVec[i]->GetTypeInfo().IsA(JMaterial::StaticTypeInfo()))
				newRenderItem->SetMaterial(i, static_cast<JMaterial*>(materialVec[i]));
		}
		return newRenderItem;
	}
	void JRenderItem::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JRenderItem>(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
			JRenderItem* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JRenderItem>(guid, objFlag, owner);
			JRenderItem* newComp = ownerPtr.Get();
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
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JRenderItem>(Core::MakeGuid(), ori->GetFlag(), owner);
			JRenderItem* newComp = ownerPtr.Get();
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
		JCFI<JRenderItem>::Register(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JRenderItem::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JRenderItem::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

		static auto setFrameLam = [](JComponent& component) {static_cast<JRenderItem*>(&component)->SetFrameDirty(); };
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{ getTypeNameCallable, getTypeInfoCallable,isAvailableOverlapCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JRenderItem::JRenderItem(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JRenderItemInterface(TypeName(), guid, objFlag, owner)
	{
		AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		RegisterFrameDirtyListener(*GetOwner()->GetTransform());
	}
	JRenderItem::~JRenderItem()
	{
		RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
		if (GetOwner()->GetTransform() != nullptr)
			DeRegisterFrameDirtyListener(*GetOwner()->GetTransform());
	}
}