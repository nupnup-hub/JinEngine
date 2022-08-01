#include"JRenderItem.h" 
#include"../JComponentFactory.h"
#include"../Transform/JTransform.h"
#include"../../GameObject/JGameObject.h"
#include"../../GameObject/GameObjectDirty.h" 
#include"../../Resource/Mesh/JMeshGeometry.h"
#include"../../Resource/Material/JMaterial.h"
#include"../../Resource/JResourceManager.h" 
#include"../../Resource/Shader/JShaderFunctionEnum.h"  
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Application/JApplicationVariable.h"

using namespace DirectX;
namespace JinEngine
{
	JMeshGeometry* JRenderItem::GetMesh()const noexcept
	{
		return meshGeo;
	}
	JMaterial* JRenderItem::GetMaterial()const noexcept
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
	uint JRenderItem::GetObjCBIndex() const noexcept
	{
		return objCBIndex;
	}
	uint JRenderItem::GetIndexCount()const noexcept
	{
		return meshGeo != nullptr ? meshGeo->GetMeshIndexCount() : 0;
	}
	uint JRenderItem::GetStartIndexLocation()const noexcept
	{
		return startIndexLocation;
	}
	int JRenderItem::GetBaseVertexLocation()const noexcept
	{
		return baseVertexLocation;
	}
	DirectX::BoundingBox JRenderItem::GetBoundingBox()noexcept
	{
		if (meshGeo != nullptr)
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

			XMFLOAT3 meshBoxCenter = meshGeo->GetBoundingBoxCenter();
			XMFLOAT3 meshBoxExtent = meshGeo->GetBoundingBoxExtent();

			XMFLOAT3 gameObjBoxCenter = XMFLOAT3(meshBoxCenter.x + pos.x,
				meshBoxCenter.y + pos.y,
				meshBoxCenter.z + pos.z);

			XMFLOAT3 gameObjBoxExtent = XMFLOAT3(meshBoxExtent.x * scale.x,
				meshBoxExtent.y * scale.y,
				meshBoxExtent.z * scale.z);

			return DirectX::BoundingBox(gameObjBoxCenter, gameObjBoxExtent);
		}
		else
			return DirectX::BoundingBox();
	}
	DirectX::BoundingSphere JRenderItem::GetBoundingSphere()noexcept
	{
		if (meshGeo != nullptr)
		{
			JTransform* ownerTransform = GetOwner()->GetTransform();
			XMFLOAT3 pos = ownerTransform->GetPosition();
			XMFLOAT3 scale = ownerTransform->GetScale();

			XMFLOAT3 meshSphereCenter = meshGeo->GetBoundingSphereCenter();
			float meshSphereRad = meshGeo->GetBoundingSphereRadius();

			XMFLOAT3 gameObjSphereCenter = XMFLOAT3(meshSphereCenter.x * scale.x + pos.x,
				meshSphereCenter.y * scale.y + pos.y,
				meshSphereCenter.z * scale.z + pos.z);

			float gameObjSphereRad = meshSphereRad;

			if (scale.x >= scale.y && scale.x >= scale.z)
				gameObjSphereRad *= scale.x;
			else if (scale.y >= scale.x && scale.y >= scale.z)
				gameObjSphereRad *= scale.y;
			else
				gameObjSphereRad *= scale.z;

			return DirectX::BoundingSphere(meshSphereCenter, gameObjSphereRad);
		}
		else
			return DirectX::BoundingSphere();
	}
	void JRenderItem::SetMeshGeometry(JMeshGeometry* meshGeo)noexcept
	{
		JMeshGeometry* pre = JRenderItem::meshGeo;
		JMeshGeometry* af = meshGeo;

		if (meshGeo != nullptr)
		{
			if (IsActivated())
			{
				if (JRenderItem::meshGeo != nullptr)
					JRenderItem::meshGeo->OffReference();
				JRenderItem::meshGeo = meshGeo;
				JRenderItem::meshGeo->OnReference();
			}
			else
				JRenderItem::meshGeo = meshGeo; 
			JRenderItem::startIndexLocation = 0;
			JRenderItem::baseVertexLocation = 0;
		}
		else
		{
			if (JRenderItem::meshGeo != nullptr)
				JRenderItem::meshGeo->OffReference();
			JRenderItem::meshGeo = meshGeo;
		}
		ReRegisterComponent<JRenderItem>(this);
		if (gameObjectDirty != nullptr)
			SetDirty();
	}
	void JRenderItem::SetMaterial(JMaterial* material)noexcept
	{
		if (material != nullptr)
		{
			if (IsActivated())
			{
				if (JRenderItem::material != nullptr)
					JRenderItem::material->OffReference();
				JRenderItem::material = material;
				JRenderItem::material->OnReference();
			}
			else
				JRenderItem::material = material;
		}
		else
		{
			if (JRenderItem::material != nullptr)
				JRenderItem::material->OffReference();
			JRenderItem::material = material;
		}
		ReRegisterComponent<JRenderItem>(this);
		if (gameObjectDirty != nullptr)
			SetDirty();
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
			JRenderItem::renderLayer = renderLayer;
			if (IsActivated())
				ReRegisterComponent<JRenderItem>(this);
		}
	}
	void JRenderItem::SetRenderVisibility(const J_RENDER_VISIBILITY renderVisibility)noexcept
	{
		JRenderItem::renderVisibility = renderVisibility;
	}
	void JRenderItem::SetObjCBIndex(const uint objCBIndex)noexcept
	{
		JRenderItem::objCBIndex = objCBIndex;
		SetDirty();
	}
	bool JRenderItem::HasMaterial()const noexcept
	{
		return material != nullptr;
	}
	bool JRenderItem::IsDirtied()const noexcept
	{
		return gameObjectDirty->GetRenderItemDirty() > 0;
	}
	bool JRenderItem::IsVisible()const noexcept
	{
		return renderVisibility == J_RENDER_VISIBILITY::VISIBLE;
	}
	void JRenderItem::SetDirty()noexcept
	{
		gameObjectDirty->SetRenderItemDirty();
	}
	void JRenderItem::OffDirty()noexcept
	{
		gameObjectDirty->OffRenderItemDirty();
	}
	void JRenderItem::MinusDirty()noexcept
	{
		gameObjectDirty->RenderItemUpdate();
	}
	J_COMPONENT_TYPE JRenderItem::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	J_COMPONENT_TYPE JRenderItem::GetStaticComponentType()noexcept
	{
		return J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM;
	}
	bool JRenderItem::IsAvailableOverlap()const noexcept
	{
		return false;
	}
	bool JRenderItem::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && meshGeo != nullptr && material != nullptr)
			return true;
		else
			return false;
	}
	void JRenderItem::DoActivate()noexcept
	{
		JComponent::DoActivate();
		if (meshGeo != nullptr)
			meshGeo->OnReference();

		if (material != nullptr)
			material->OnReference();
		ReRegisterComponent<JRenderItem>(this);
		SetDirty();
	}
	void JRenderItem::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		if (meshGeo != nullptr)
			meshGeo->OffReference();

		if (material != nullptr)
			material->OffReference();
		DeRegisterComponent<JRenderItem>(this);
		OffDirty();
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

		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, renderItem);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;
		 

		bool hasMaterial = renderItem->material != nullptr;
		size_t materialGuid = hasMaterial ? renderItem->material->GetGuid() : 0;

		stream << hasMaterial << '\n' << renderItem->meshGeo->GetGuid() << '\n' << materialGuid << '\n'
			<< renderItem->objCBIndex << '\n' << renderItem->primitiveType << '\n' << (int)renderItem->renderLayer << '\n';

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JRenderItem* JRenderItem::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open())
			return nullptr;
		 
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);

		JRenderItem* newRenderItem;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newRenderItem = new JRenderItem(metadata.guid, metadata.flag, owner);
		else
			newRenderItem = new JRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);

		bool hasMaterial;
		size_t meshGuid;
		size_t materialGuid;
		int renderItemIndex;
		int primitiveType;
		int renderLayer;

		stream >> hasMaterial >> meshGuid >> materialGuid >> renderItemIndex >> primitiveType >> renderLayer;

		JMeshGeometry* mesh = JResourceManager::Instance().GetResource<JMeshGeometry>(meshGuid);
		JMaterial* material = nullptr;
		if (hasMaterial)
			material = JResourceManager::Instance().GetResource<JMaterial>(materialGuid);

		newRenderItem->SetMeshGeometry(mesh);
		newRenderItem->SetMaterial(material);

		return newRenderItem;
	}
	void JRenderItem::RegisterFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			return new JRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
		};
		auto initC = [](const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			return new JRenderItem(guid, objFlag, owner);
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			JRenderItem* oriR = static_cast<JRenderItem*>(ori);
			JRenderItem* newR = new JRenderItem(Core::MakeGuid(), oriR->GetFlag(), owner);

			newR->SetMeshGeometry(oriR->meshGeo);
			newR->SetMaterial(oriR->material);
			newR->textureTransform = oriR->textureTransform;
			newR->SetPrimitiveType(oriR->primitiveType);
			newR->SetRenderLayer(oriR->renderLayer);

			return newR;
		};
		JCFI<JRenderItem>::Regist(defaultC, initC, loadC, copyC);
	}
	JRenderItem::JRenderItem(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(TypeName(), guid, objFlag, owner)
	{
		JRenderItem::startIndexLocation = 0;
		JRenderItem::baseVertexLocation = 0;

		gameObjectDirty = owner->GetGameObjectDirty();
	}
	JRenderItem::~JRenderItem()
	{

	}
}