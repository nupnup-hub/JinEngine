#include"JResourceManager.h" 
#include"JResourceIO.h"
#include"JResourceObjectFactory.h" 
#include"Mesh/JDefaultGeometryGenerator.h"
#include"Mesh/JDefaultShapeType.h"  

#include"Mesh/JMeshGeometry.h" 
#include"Mesh/JMeshInterface.h"
#include"Mesh/JDefaultGeometryGenerator.h"
#include"Material/JMaterial.h" 
#include"Model/JModel.h" 
#include"Texture/JTexture.h" 
#include"Texture/EditorTextureEnum.h"  
#include"Skeleton/JSkeletonAsset.h"
#include"AnimationClip/JAnimationClip.h" 
#include"AnimationController/JAnimationController.h" 
#include"Scene/JScene.h"
#include"Scene/Preview/PreviewEnum.h"
#include"Shader/JShader.h"

#include"../Directory/JFile.h" 
#include"../Directory/JDirectory.h"
#include"../Directory/JDirectoryFactory.h"

#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Core/Guid/GuidCreator.h" 
#include"../../Utility/JCommonUtility.h"
#include"../../Application/JApplicationVariable.h"
#include"../../Core/DirectXEx/JDirectXCollisionEx.h"

#include"../../Graphic/JGraphic.h"
#include"../../Graphic/JGraphicDrawList.h"
#include"../../Graphic/JGraphicResourceManager.h"
 
using namespace DirectX;
namespace JinEngine
{
	//Storage
	JResourceObject* JResourceManager::Storage::Get(const size_t guid)noexcept
	{
		return rMap.Get(guid);
	}
	JResourceObject* JResourceManager::Storage::GetByIndex(const uint index)
	{
		return rVec.Get(index);
	}
	JResourceObject* JResourceManager::Storage::GetByPath(const std::string& path)noexcept
	{
		const uint count = rVec.Count();
		for (uint i = 0; i < count; ++i)
		{
			if (rVec.Get(i)->GetPath() == path)
				return rVec.Get(i);
		}
		return nullptr;
	}
	std::vector<JResourceObject*>::const_iterator JResourceManager::Storage::GetVectorIter(uint& count)
	{
		return rVec.GetCBegin();
	}
	bool JResourceManager::Storage::Has(const size_t guid)noexcept
	{
		return rMap.Get(guid) != nullptr;
	}
	uint JResourceManager::Storage::Count()const noexcept
	{
		return rVec.Count();
	}
	JResourceObject* JResourceManager::Storage::AddResource(JResourceObject* resource)noexcept
	{
		if (resource == nullptr)
			return nullptr;
		rVec.Add(resource);
		rMap.Add(resource, resource->GetGuid());

		return resource;
	}
	bool JResourceManager::Storage::EraseResource(JResourceObject* resource)noexcept
	{
		if (resource == nullptr)
			return false;

		if (resource->IsActivated())
			resource->OffReference();

		static auto equalLam = [](JResourceObject* a, JResourceObject* b) {return a->GetGuid() == b->GetGuid(); };

		rVec.Erase(resource, equalLam);
		rMap.Erase(resource->GetGuid());
		return resource;
	}
	void JResourceManager::Storage::Clear()
	{
		rVec.Clear();
		rMap.Clear();
	}

	std::unique_ptr<JResourceManager> JResourceManager::instance = nullptr;
	size_t JResourceManager::managerGuid = 0;
	JResourceManager::JResourceManager()
	{
		resourceIO = std::make_unique<JResourceIO>();

		std::vector<JRI::RTypeHint> rinfo = JRI::GetRInfo(RESOURCE_ALIGN_TYPE::NONE);
		const uint rinfoCount = (uint)rinfo.size();
		for(uint i = 0; i < rinfoCount; ++i)
			rCashMap.emplace(rinfo[i].thisType, Storage());
	}
	JResourceManager::~JResourceManager() {}
	JResourceManager& JResourceManager::Instance()
	{ 
		if (instance == nullptr)
		{
			instance = std::make_unique<JResourceManager>();
			managerGuid = JCommonUtility::CalculateGuid(typeid(JResourceManager).name());
		}
		return *instance;
	}
#pragma region  초기화
	void JResourceManager::LoadSelectorResource()
	{
		JOBJECT_FLAG rootFlag = (JOBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_INERASABLE);
		std::string dirPath = JApplicationVariable::GetEnginePath();
		std::string dirFolderPath;
		std::string dirName;
		std::string dirFormat;
		JCommonUtility::DecomposeFilePath(dirPath, dirFolderPath, dirName, dirFormat);
		engineRootDir = JDFI::CreateRoot(dirName, Core::MakeGuid(), rootFlag);

		resourceIO->LoadEngineDirectory(engineRootDir);
		JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		JGraphic::Instance().CommandInterface()->StartCommand();
		CreateDefaultTexture(resourceData.selectorTexInfo);
		JGraphic::Instance().CommandInterface()->EndCommand();
		JGraphic::Instance().CommandInterface()->FlushCommandQueue();
	}
	void JResourceManager::LoadProjectResource()
	{
		JOBJECT_FLAG rootFlag = (JOBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_INERASABLE);
		std::string dirPath = JApplicationVariable::GetProjectPath();
		std::string dirFolderPath;
		std::string dirName;
		std::string dirFormat;
		JCommonUtility::DecomposeFilePath(dirPath, dirFolderPath, dirName, dirFormat);
		projectRootDir = JDFI::CreateRoot(dirName, Core::MakeGuid(), rootFlag);

		resourceIO->LoadProjectDirectory(projectRootDir);

		JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		JGraphic::Instance().CommandInterface()->StartCommand();
		CreateDefaultTexture(resourceData.projectTexInfo);
		JGraphic::Instance().CommandInterface()->EndCommand();
		JGraphic::Instance().CommandInterface()->FlushCommandQueue();

		CreateDefaultShader();
		CreateDefaultMaterial();

		JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		JGraphic::Instance().CommandInterface()->StartCommand();
		CreateDefaultMesh();
		JGraphic::Instance().CommandInterface()->EndCommand();
		JGraphic::Instance().CommandInterface()->FlushCommandQueue();
		LoadObject();

		if (nowMainScene == nullptr)
		{
			JDirectory* projectContentsDir = GetDirectory(JApplicationVariable::GetProjectContentPath());
			JDirectory* defaultSceneDir = projectContentsDir->GetChildDirctory(JApplicationVariable::GetProjectContentScenePath());
			JScene* newScene = JRFI<JScene>::Create(*defaultSceneDir);
			nowMainScene = newScene;
			//JScene* newScene = CreateScene(SCENE_TYPE::SCENE_TYPE_MAIN_SCENE, OBJECT_FLAG_NONE, defaultSceneDir);
			//resourceIO->LoadScene(newScene, true);
		}
		nowMainScene->OnReference();
		Graphic::JGraphicDrawList::AddDrawList(nowMainScene, Graphic::J_GRAPHIC_DRAW_FREQUENCY::ALWAYS, true);
		ISceneSpatialStructure* iscene = nowMainScene;
		//iscene->CreateDemoGameObject();
		iscene->OnSceneSpatialStructure();
		iscene->BuildBvh();
		iscene->OnDebugBoundingBox(false);
	}
#pragma endregion
	void JResourceManager::Terminate()
	{
		StoreResourceData();
		nowMainScene->DeActivate();

		const uint groupCount = (uint)previewSceneGroup.size();
		for (uint i = 0; i < groupCount; ++i)
			previewSceneGroup[i]->Clear();
		previewSceneGroup.clear();

		for (auto& data : rCashMap)
			data.second.Clear();
		dCash.Clear();
	}
	JMeshGeometry* JResourceManager::GetDefaultMeshGeometry(const J_DEFAULT_SHAPE type)noexcept
	{
		auto data = resourceData.basicMeshGuidMap.find(type);
		return data != resourceData.basicMeshGuidMap.end() ?
			static_cast<JMeshGeometry*>(rCashMap.find(JMeshGeometry::GetStaticResourceType())->second.Get(data->second)) : nullptr;
	}
	JMaterial* JResourceManager::GetDefaultMaterial(const J_DEFAULT_MATERIAL materialType)noexcept
	{
		const size_t guid = resourceData.basicMaterialGuidMap.find(materialType)->second;
		return static_cast<JMaterial*>(rCashMap.find(JMaterial::GetStaticResourceType())->second.Get(guid));
	}
	JTexture* JResourceManager::GetEditorTexture(const J_EDITOR_TEXTURE enumName)noexcept
	{
		auto data = resourceData.defaultTextureMap.find(enumName);
		if (data == resourceData.defaultTextureMap.end())
			return nullptr;

		return static_cast<JTexture*>(rCashMap.find(JTexture::GetStaticResourceType())->second.GetByIndex(data->second - 1));
	}
	JShader* JResourceManager::GetDefaultShader(const J_DEFAULT_SHADER shaderType)noexcept
	{
		auto data = resourceData.defaultShaderGuidMap.find(shaderType);
		return data != resourceData.defaultShaderGuidMap.end() ?
			static_cast<JShader*>(rCashMap.find(JShader::GetStaticResourceType())->second.Get(data->second)) : nullptr;
	}
	JDirectory* JResourceManager::GetDirectory(const std::string& path)noexcept
	{
		return dCash.GetByPath(path);
	}
	JDirectory* JResourceManager::GetEditorResourceDirectory()noexcept
	{
		return dCash.GetByPath(JApplicationVariable::GetProjectEditorResourcePath());
	}
	JDirectory* JResourceManager::GetActivatedDirectory()noexcept
	{
		return dCash.GetAcitvatedDirectory();
	}
	JScene* JResourceManager::GetMainScene()noexcept
	{
		return nowMainScene;
	}
	uint JResourceManager::GetResourceCount(const J_RESOURCE_TYPE type)noexcept
	{
		return rCashMap.find(type)->second.Count();
	}
	JResourceObject* JResourceManager::GetResource(const J_RESOURCE_TYPE type, const size_t guid)noexcept
	{
		return rCashMap.find(type)->second.Get(guid);
	}
	JResourceObject* JResourceManager::GetResourceByPath(const J_RESOURCE_TYPE type, const std::string& path)noexcept
	{
		return rCashMap.find(type)->second.GetByPath(path);
	}
	std::vector<JResourceObject*>::const_iterator JResourceManager::GetResourceVectorHandle(const J_RESOURCE_TYPE type, uint& resouceCount)noexcept
	{
		return rCashMap.find(type)->second.GetVectorIter(resouceCount);
	}
	bool JResourceManager::HasResource(const J_RESOURCE_TYPE rType, const size_t guid)noexcept
	{
		return rCashMap.find(rType)->second.Has(guid);
	}
	bool JResourceManager::EraseResource(JResourceObject* resource)noexcept
	{
		return rCashMap.find(resource->GetResourceType())->second.EraseResource(resource);
	}
	std::vector<PreviewSceneGroup*>::const_iterator JResourceManager::GetPreviewGroupVectorHandle(_Out_ uint& groupCount)noexcept
	{
		groupCount = (uint)previewSceneGroup.size();
		return previewSceneGroupCashVec.cbegin();
	}
	PreviewSceneGroup* JResourceManager::CreatePreviewGroup(const std::string& ownerName, const uint groupSceneCapacity)noexcept
	{
		const uint groupCount = (uint)previewSceneGroup.size();

		std::unique_ptr<PreviewSceneGroup> newPreviewGroup = std::make_unique<PreviewSceneGroup>(ownerName, groupSceneCapacity);

		PreviewSceneGroup* res = newPreviewGroup.get();
		previewSceneGroupCashVec.push_back(newPreviewGroup.get());
		previewSceneGroup.push_back(std::move(newPreviewGroup));
		return res;
	}
	bool JResourceManager::ClearPreviewGroup(PreviewSceneGroup* group)noexcept
	{
		const uint groupCount = (uint)previewSceneGroup.size();
		const size_t guid = group->GetGuid();

		for (uint i = 0; i < groupCount; ++i)
		{
			if (guid == previewSceneGroup[i]->GetGuid())
			{
				previewSceneGroup[i]->Clear();
				return true;
			}
		}
		return false;
	}
	bool JResourceManager::ErasePreviewGroup(PreviewSceneGroup* group)noexcept
	{
		const uint groupCount = (uint)previewSceneGroup.size();
		const size_t guid = group->GetGuid();
		for (uint i = 0; i < groupCount; ++i)
		{
			if (guid == previewSceneGroup[i]->GetGuid())
			{
				previewSceneGroup[i]->Clear();
				previewSceneGroup.erase(previewSceneGroup.begin() + i);
				return true;
			}
		}
		return false;
	}
	PreviewScene* JResourceManager::CreatePreviewScene(PreviewSceneGroup* group, JResourceObject* resource, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag)noexcept
	{
		if (resource == nullptr || resource->GetFlag() == OBJECT_FLAG_EDITOR_OBJECT)
			return nullptr;

		const size_t guid = group->GetGuid();
		const uint groupCount = (uint)previewSceneGroup.size();

		for (uint i = 0; i < groupCount; ++i)
		{
			if (guid == previewSceneGroup[i]->GetGuid())
			{
				return previewSceneGroup[i]->CreateResourcePreviewScene(resource, previewDimension, previewFlag);
			}
		}
		return nullptr;
	}
	bool JResourceManager::ErasePreviewScene(PreviewSceneGroup* group, JResourceObject* resource)noexcept
	{
		if (resource == nullptr)
			return false;

		const size_t guid = group->GetGuid();
		const uint groupCount = (uint)previewSceneGroup.size();
		for (uint i = 0; i < groupCount; ++i)
		{
			if (guid == previewSceneGroup[i]->GetGuid())
			{
				return previewSceneGroup[i]->Erase(resource);
			}
		}
		return false;
	}
	void JResourceManager::CreateDefaultTexture(const std::vector<JResourceData::DefaultTextureInfo>& textureInfo)noexcept
	{
		resourceData.defaultTextureMap.clear();

		//수정필요
		uint handleIncrement = JGraphic::Instance().DeviceInterface()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		uint textureCount = (uint)textureInfo.size();
		JDirectory* defaultTextureDir = GetDirectory(JApplicationVariable::GetDefaultResourcePath());
	
		//1 is imgui preserved 
		for (uint i = 1; i < textureCount; ++i)
		{  
			JOBJECT_FLAG objFlag = (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_DO_NOT_SAVE);
			if (textureInfo[i].type == J_EDITOR_TEXTURE::MISSING)
				objFlag = (JOBJECT_FLAG)(objFlag | OBJECT_FLAG_HIDDEN);

			JResourcePathData pathData(defaultTextureDir->GetWPath() + L"\\" + textureInfo[i].name);
			JTexture* newTexture = JRFI<JTexture>::Load(*defaultTextureDir, pathData);
			//ThrowIfFailedNM(newTexture != nullptr, "Load default texture is error please retry again");

			resourceData.defaultTextureMap.emplace(textureInfo[i].type, i);
		}
	}
	void JResourceManager::CreateDefaultShader()noexcept
	{
		J_SHADER_FUNCTION function[(int)J_DEFAULT_SHADER::COUNTER] =
		{
			(J_SHADER_FUNCTION)(SHADER_FUNCTION_SHADOW | SHADER_FUNCTION_LIGHT),
			SHADER_FUNCTION_SKY,
			(J_SHADER_FUNCTION)(SHADER_FUNCTION_ALPHA_CLIP | SHADER_FUNCTION_SHADOW_MAP),
			SHADER_FUNCTION_DEBUG,
		};
		JOBJECT_FLAG objFlag[(int)J_DEFAULT_SHADER::COUNTER] =
		{
			(JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_DO_NOT_SAVE),
			(JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_DO_NOT_SAVE),
			(JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE),
			(JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE),
		};
		JDirectory* shaderDir = GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath());
		for (uint i = 0; i < (int)J_DEFAULT_SHADER::COUNTER; ++i)
		{
			JShader* res = JRFI<JShader>::Create(std::to_string((int)function[i]),
				Core::MakeGuid(),
				objFlag[i],
				*shaderDir,
				0,
				function[i]);

			if (res == nullptr)
				MessageBox(0, L"Default JShader Creator Error", 0, 0);
			resourceData.defaultShaderGuidMap.emplace(resourceData.defaultShaderTypes[i], res->GetGuid());
		}
	}
	void JResourceManager::CreateDefaultMaterial()noexcept
	{
		const JOBJECT_FLAG flag = (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED |
			OBJECT_FLAG_INERASABLE |
			OBJECT_FLAG_UNEDITABLE |
			OBJECT_FLAG_DO_NOT_SAVE);

		constexpr int formatIndex = 0;
		for (uint i = 0; i < (int)J_DEFAULT_MATERIAL::COUNTER; ++i)
		{
			const std::string name = JDefaultMateiralType::ConvertBasicMateiralName(resourceData.defaultMaterialTypes[i]);

			const size_t shaderGuid = resourceData.defaultShaderGuidMap[JDefaultMateiralType::FindMatchBasicShaderType(resourceData.defaultMaterialTypes[i])];
			JShader* defaultShader = GetResource<JShader>(shaderGuid);
			size_t guid = Core::MakeGuid();

			switch (resourceData.defaultMaterialTypes[i])
			{
			case J_DEFAULT_MATERIAL::DEFAULT_STANDARD:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (JOBJECT_FLAG)(flag),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetShadow(true);
				newMaterial->SetLight(true);
				break;
			}
			case J_DEFAULT_MATERIAL::DEFAULT_SKY:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (JOBJECT_FLAG)(flag),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetSkyMaterial(true);
				break;
			}
			case J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (JOBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetAlbedoOnly(true);
				newMaterial->SetShadowMap(true);
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (JOBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetDebugMaterial(true);
				newMaterial->SetAlbedoColor(XMFLOAT4(0.75f, 0.1f, 0.1f, 0.6f));
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (JOBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetDebugMaterial(true);
				newMaterial->SetAlbedoColor(XMFLOAT4(0.1f, 0.75f, 0.1f, 0.6f));
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (JOBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetDebugMaterial(true);
				newMaterial->SetAlbedoColor(XMFLOAT4(0.1f, 0.1f, 0.75f, 0.6f));
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (JOBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetDebugMaterial(true);
				newMaterial->SetAlbedoColor(XMFLOAT4(0.75f, 0.75f, 0.05f, 0.6f));
				break;
			}
			default:
				break;
			}
			resourceData.basicMaterialGuidMap.emplace(resourceData.defaultMaterialTypes[i], guid);
		}
	}
	void JResourceManager::CreateDefaultMesh()
	{
		JDefaultGeometryGenerator geoGen;
		std::vector<JStaticMeshData> meshData{ geoGen.CreateCube(1, 1, 1, 3),
			geoGen.CreateGrid(40.0f, 60.0f, 60, 40),
			geoGen.CreateSphere(0.5f, 20, 20),
			geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20),
			geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f),
			geoGen.CreateBoundingBox(),
			geoGen.CreateBoundingFrustum() };

		std::vector<std::vector<JStaticMeshVertex>> vertices((int)J_DEFAULT_SHAPE::COUNT);
		std::vector<SubmeshGeometry> submesh((int)J_DEFAULT_SHAPE::COUNT);

		for (int i = 0; i < (int)J_DEFAULT_SHAPE::COUNT; ++i)
			vertices[i].resize(meshData[i].vertices.size());

		XMFLOAT3 vMinf3(+JMathHelper::Infinity, +JMathHelper::Infinity, +JMathHelper::Infinity);
		XMFLOAT3 vMaxf3(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);

		std::vector<BoundingBox> boundingBox((int)J_DEFAULT_SHAPE::COUNT);
		std::vector<BoundingSphere> boundingSphere((int)J_DEFAULT_SHAPE::COUNT);

		for (int i = 0; i < (int)J_DEFAULT_SHAPE::COUNT; ++i)
		{
			XMVECTOR vMin = XMLoadFloat3(&vMinf3);
			XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
			for (int j = 0; j < meshData[i].vertices.size(); ++j)
			{
				vertices[i][j].position = meshData[i].vertices[j].position;
				vertices[i][j].normal = meshData[i].vertices[j].normal;
				vertices[i][j].texC = meshData[i].vertices[j].texC;
				vertices[i][j].tangentU = meshData[i].vertices[j].tangentU;

				XMVECTOR P = XMLoadFloat3(&vertices[i][j].position);
				vMin = XMVectorMin(vMin, P);
				vMax = XMVectorMax(vMax, P);
			}

			XMStoreFloat3(&boundingBox[i].Center, 0.5f * (vMin + vMax));
			XMStoreFloat3(&boundingBox[i].Extents, 0.5f * (vMax - vMin));

			XMStoreFloat3(&boundingSphere[i].Center, 0.5f * (vMin + vMax));
			XMFLOAT3 dis;
			XMStoreFloat3(&dis, XMVector3Length((0.5f * (vMin + vMax)) - vMax));
			boundingSphere[i].Radius = (float)sqrt(pow(dis.x, 2) + pow(dis.y, 2) + pow(dis.z, 2));
		}

		std::vector<std::vector<uint16>> indices((int)J_DEFAULT_SHAPE::COUNT);
		for (int i = 0; i < (int)J_DEFAULT_SHAPE::COUNT; ++i)
		{
			meshData[i].Stuff8ByteDataTo4Byte();
			indices[i].insert(indices[i].end(), meshData[i].indices16.begin(), meshData[i].indices16.end());
		}

		constexpr int formatIndex = 0;
		for (int i = 0; i < (int)J_DEFAULT_SHAPE::COUNT; ++i)
		{
			const uint vbByteSize = (uint)vertices[i].size() * sizeof(JStaticMeshVertex);
			const uint ibByteSize = (uint)indices[i].size() * sizeof(std::uint16);

			size_t guid = Core::MakeGuid();
			JOBJECT_FLAG flag = (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED |
				OBJECT_FLAG_INERASABLE |
				OBJECT_FLAG_UNEDITABLE |
				OBJECT_FLAG_DO_NOT_SAVE);

			if (i >= JDefaultShape::debugTypeSt)
				flag = (JOBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN);

			JMeshGeometry* newMesh = JRFI<JMeshGeometry>::Create(JDefaultShape::ConvertDefaultShapeName(resourceData.defaultMeshTypes[i]),
				guid,
				flag,
				*GetDirectory(JApplicationVariable::GetDefaultResourcePath()),
				formatIndex);

			JMeshInterface* iMesh = newMesh;
			iMesh->StuffStaticMesh(meshData[i], boundingBox[i], boundingSphere[i]);
			resourceData.basicMeshGuidMap.emplace(resourceData.defaultMeshTypes[i], guid);
		}
	}

	JResourceObject* JResourceManager::GetResourceByType(const J_RESOURCE_TYPE type, const size_t guid)noexcept
	{
		return rCashMap.find(type)->second.Get(guid);
	}
	void JResourceManager::StoreResourceData()
	{
		for (auto& rData : rCashMap)
		{
			uint count;
			std::vector<JResourceObject*>::const_iterator begin = rData.second.GetVectorIter(count);
			for (uint i = 0; i < count; ++i)
			{
				JResourceObjectInterface* iR = *(begin + i);
				iR->CallStoreResource();
			}
		}
	}
	void JResourceManager::LoadObject()
	{
		resourceIO->LoadProjectResource(projectRootDir);
	}
}

/*
JMeshGeometry* JResourceManager::CreateMesh(const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JMeshGeometry* JResourceManager::CreateMesh(const std::string& name,
		const size_t guid,
		const JOBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex,
		JStaticMeshData& meshData,
		const BoundingBox& boundingBox,
		const BoundingSphere& boundingSphere)noexcept
	{
		if (mesh->HasResource(guid))
			return nullptr;

		JMeshGeometry* res = mesh->AddResource(std::make_unique<JMeshGeometry>(directory->MakeUniqueFileName(name), guid, flag, directory, formatIndex,
			meshData, boundingBox, boundingSphere));

		if ((flag & OBJECT_FLAG_DO_NOT_SAVE) == 0)
			;// resourceIO->StoreMesh(res);	수정필요
		NotifyEvent(managerGuid, RESOURCE_EVENT::CREATE_RESOURCE, res);
		if (directory != nullptr)
			directory->AddFile(std::make_unique<JFile>(res));
		return res;
	}
	JMeshGeometry* JResourceManager::CreateMesh(const std::string& name,
		const size_t guid,
		const JOBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex,
		JSkinnedMeshData& meshData,
		const BoundingBox& boundingBox,
		const BoundingSphere& boundingSphere)noexcept
	{
		if (mesh->HasResource(guid))
			return nullptr;

		JMeshGeometry* res = mesh->AddResource(std::make_unique<JMeshGeometry>(directory->MakeUniqueFileName(name), guid, flag, directory, formatIndex,
			meshData, boundingBox, boundingSphere));

		if ((flag & OBJECT_FLAG_DO_NOT_SAVE) == 0)
			;// resourceIO->StoreMesh(res);	수정필요
		NotifyEvent(managerGuid, RESOURCE_EVENT::CREATE_RESOURCE, res);
		if (directory != nullptr)
			directory->AddFile(std::make_unique<JFile>(res));
		return res;
	}
	JMaterial* JResourceManager::CreateMaterial(const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		if (selectedDirctory == nullptr)
		{
			if ((flag & OBJECT_FLAG_EDITOR_OBJECT) == 0)
				selectedDirctory = GetResourceByPath<JDirectory>(JApplicationVariable::GetProjectEditorResourcePath());
			else
				selectedDirctory = GetActivatedDirectory();
		}

		return CreateMaterial(JResourceObject::GetDefaultName<JMaterial>(), Core::MakeGuid()(), flag, selectedDirctory);
	}
	JMaterial* JResourceManager::CreateMaterial(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		const size_t shaderGuid = resourceData.defaultShaderGuidMap[J_DEFAULT_SHADER::DEFAULT_STANDARD_SHADER];

		JMaterial* res = material->AddResource(std::make_unique<JMaterial>(selectedDirctory->MakeUniqueFileName(name),
			guid,
			flag,
			selectedDirctory,
			material->GetResourceCount()));

		if (flag & OBJECT_FLAG_DO_NOT_SAVE == 0)
			;// resourceIO->StoreMaterial(res);	수정필요

		NotifyEvent(managerGuid, RESOURCE_EVENT::CREATE_RESOURCE, res);
		if (selectedDirctory != nullptr)
			selectedDirctory->AddFile(std::make_unique<JFile>(res));
		return res;
	}
	JTexture* JResourceManager::CreateTexture(const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JModel* JResourceManager::CreateModel(const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JSkeletonAsset* JResourceManager::CreateSkeletonAsset(const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JAnimationClip* JResourceManager::CreateAnimationClip(const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JAnimationController* JResourceManager::CreateAnimationController(const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		if (selectedDirctory == nullptr)
		{
			if ((flag & OBJECT_FLAG_EDITOR_OBJECT) == 0)
				selectedDirctory = GetResourceByPath<JDirectory>(JApplicationVariable::GetProjectEditorResourcePath());
			else
				selectedDirctory = GetActivatedDirectory();
		}

		JAnimationController* res = animationController->AddResource(
			std::make_unique<JAnimationController>(selectedDirctory->MakeUniqueFileName(JResourceObject::GetDefaultName<JAnimationController>()),
				Core::MakeGuid< JAnimationController>(),
				flag,
				selectedDirctory));

		//resourceIO->StoreAnimationController(res);		수정필요

		NotifyEvent(managerGuid, RESOURCE_EVENT::CREATE_RESOURCE, res);
		if (selectedDirctory != nullptr)
			selectedDirctory->AddFile(std::make_unique<JFile>(res));

		return res;
	}
	JScene* JResourceManager::CreateScene(const SCENE_TYPE sceneType, const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		if (selectedDirctory == nullptr)
		{
			if ((flag & OBJECT_FLAG_EDITOR_OBJECT) == 0)
				selectedDirctory = GetResourceByPath<JDirectory>(JApplicationVariable::GetProjectEditorResourcePath());
			else
				selectedDirctory = GetActivatedDirectory();
		}

		std::string newPath;
		std::string newName;
		size_t guid;

		JScene* res = scene->AddResource(std::make_unique<JScene>(selectedDirctory->MakeUniqueFileName(JResourceObject::GetDefaultName<JScene>()),
			guid,
			flag,
			selectedDirctory,
			sceneType));

		res->MakeDefaultObject();
		res->Activate();
		//resourceIO->StoreScene(res);수정필요
		res->DeActivate();

		NotifyEvent(managerGuid, RESOURCE_EVENT::CREATE_RESOURCE, res);
		if (selectedDirctory != nullptr)
			selectedDirctory->AddFile(std::make_unique<JFile>(res));
		return res;
	}
	JShader* JResourceManager::CreateShader(const J_SHADER_FUNCTION newFunctionFlag, JOBJECT_FLAG flag)noexcept
	{
		flag = (JOBJECT_FLAG)(OBJECT_FLAG_HIDDEN | (flag ^ (flag & OBJECT_FLAG_HIDDEN)));

		uint count;
		std::vector<JShader*>::const_iterator st = shader->GetResourceVector(count);

		for (uint i = 0; i < count; ++i)
		{
			if (newFunctionFlag == (*(st + i))->GetShaderFunctionFlag())
				return (*(st + i));
		}

		JShader* newShader = shader->AddResource(std::make_unique<JShader>(std::to_string((int)newFunctionFlag),
			Core::MakeGuid<JShader>(),
			flag,
			GetResourceByPath<JDirectory>(JApplicationVariable::GetProjectShaderMetafilePath()),
			newFunctionFlag));

		NotifyEvent(managerGuid, RESOURCE_EVENT::CREATE_RESOURCE, newShader);
		return newShader;
	}
	JShader* JResourceManager::CreateShader(const size_t guid, JOBJECT_FLAG flag, const J_SHADER_FUNCTION newFunctionFlag)noexcept
	{
		if (shader->HasResource(guid))
			return nullptr;

		flag = (JOBJECT_FLAG)(OBJECT_FLAG_HIDDEN | (flag ^ (flag & OBJECT_FLAG_HIDDEN)));

		uint count;
		std::vector<JShader*>::const_iterator st = shader->GetResourceVector(count);

		for (uint i = 0; i < count; ++i)
		{
			if (newFunctionFlag == (*(st + i))->GetShaderFunctionFlag())
				return (*(st + i));
		}

		JShader* newShader = shader->AddResource(std::make_unique<JShader>(std::to_string((int)newFunctionFlag),
			guid,
			flag,
			GetResourceByPath<JDirectory>(JApplicationVariable::GetProjectShaderMetafilePath()),
			newFunctionFlag));

		NotifyEvent(managerGuid, RESOURCE_EVENT::CREATE_RESOURCE, newShader);
		return newShader;
	}
	JDirectory* JResourceManager::CreateDirectoryFolder(std::string name, const JOBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		if (selectedDirctory == nullptr)
			return nullptr;

		if (name == "")
			name = JResourceObject::GetDefaultName<JDirectory>();

		name = selectedDirctory->MakeUniqueFileName(name);
		const std::string path = selectedDirctory->GetPath() + "\\" + name;

		return directory->AddResource(std::make_unique<JDirectory>(name, Core::MakeGuid<JDirectory>(), flag, selectedDirctory));
	}
*/