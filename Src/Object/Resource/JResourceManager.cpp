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
#include"Scene/Preview/JPreviewScene.h"
#include"Scene/Preview/JPreviewEnum.h"
#include"Scene/JSceneManager.h"
#include"Shader/JShader.h"

#include"../Directory/JFile.h" 
#include"../Directory/JDirectory.h"
#include"../Directory/JDirectoryFactory.h"
#include"../Component/Camera/JCameraState.h"
#include"../Component/Camera/JCamera.h"

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
	//ResourceStorage
	JResourceObject* JResourceManagerImpl::ResourceStorage::Get(const size_t guid)noexcept
	{
		return rMap.Get(guid);
	}
	JResourceObject* JResourceManagerImpl::ResourceStorage::GetByIndex(const uint index)
	{
		return rVec.Get(index);
	}
	JResourceObject* JResourceManagerImpl::ResourceStorage::GetByPath(const std::string& path)noexcept
	{
		const uint count = rVec.Count();
		for (uint i = 0; i < count; ++i)
		{
			if (rVec.Get(i)->GetPath() == path)
				return rVec.Get(i);
		}
		return nullptr;
	}
	std::vector<JResourceObject*>::const_iterator JResourceManagerImpl::ResourceStorage::GetVectorIter(uint& count)
	{
		return rVec.GetCBegin();
	}
	bool JResourceManagerImpl::ResourceStorage::Has(const size_t guid)noexcept
	{
		return rMap.Get(guid) != nullptr;
	}
	uint JResourceManagerImpl::ResourceStorage::Count()const noexcept
	{
		return rVec.Count();
	}
	JResourceObject* JResourceManagerImpl::ResourceStorage::AddResource(JResourceObject* resource)noexcept
	{
		if (resource == nullptr || rMap.Has(resource->GetGuid()))
			return nullptr;

		rVec.Add(resource);
		rMap.Add(resource, resource->GetGuid());

		JRI::RTypeHint rTypeHint = JRI::GetRTypeHint(resource->GetResourceType());

		if (rTypeHint.isFrameResource)
		{
			auto callable = JRI::GetSetFrameDirtyCallable(resource->GetResourceType());
			callable(nullptr, *resource);
		}

		if (rTypeHint.isGraphicBuffResource)
		{
			auto callable = JRI::GetSetBuffIndexCallable(resource->GetResourceType());
			callable(nullptr, *resource, rVec.Count());
		}

		return resource;
	}
	bool JResourceManagerImpl::ResourceStorage::RemoveResource(JResourceObject& resource)noexcept
	{
		static auto equalLam = [](JResourceObject* a, JResourceObject* b) {return a->GetGuid() == b->GetGuid(); };
		JRI::RTypeHint rTypeHint = JRI::GetRTypeHint(resource.GetResourceType());
		int index = rVec.GetIndex(&resource, equalLam);

		if (rTypeHint.isFrameResource)
		{
			auto callable = JRI::GetSetFrameDirtyCallable(resource.GetResourceType());
			rVec.ApplyFunc(index, callable);
		}

		if (rTypeHint.isGraphicBuffResource)
		{ 
			auto callable = JRI::GetSetBuffIndexCallable(resource.GetResourceType());
			rVec.ApplyFuncByIndex(index, callable);
		}
		 
		bool res00 = rVec.Remove(index);
		bool res01 = rMap.Remove(resource.GetGuid());
		return res00 && res01;
	}
	void JResourceManagerImpl::ResourceStorage::Clear()
	{
		rMap.Clear();
		rVec.Clear();
	}
	  
	uint JResourceManagerImpl::DirectoryStorage::Count()const noexcept
	{
		return dVec.Count();
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::Get(const uint index)
	{
		return dVec.Get(index);
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::GetByGuid(const size_t guid)
	{
		return dMap.Get(guid);
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::GetByPath(const std::string& path)
	{
		const uint count = dVec.Count();
		for (uint i = 0; i < count; ++i)
		{
			if (dVec[i]->GetPath() == path)
				return dVec[i];
		}
		return nullptr;
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::GetOpenDirectory()
	{
		const uint count = dVec.Count();
		for (uint i = 0; i < count; ++i)
		{
			if (dVec[i]->IsOpen())
				return dVec[i];
		}
		return nullptr;
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::Add(JDirectory* dir)noexcept
	{
		if (dir == nullptr || dMap.Has(dir->GetGuid()))
			return nullptr;

		dVec.Add(dir);
		dMap.Add(dir, dir->GetGuid());

		return dir;
	}
	bool JResourceManagerImpl::DirectoryStorage::Remove(JDirectory* dir)noexcept
	{
		if (dir == nullptr)
			return false;

		static auto equalLam = [](JDirectory* a, JDirectory* b) {return a->GetGuid() == b->GetGuid(); };

		dVec.Remove(dir, equalLam);
		dMap.Remove(dir->GetGuid());
		return dir;
	}
	void JResourceManagerImpl::DirectoryStorage::Clear()
	{
		dVec.Clear();
		dMap.Clear();
	}

	JMeshGeometry* JResourceManagerImpl::GetDefaultMeshGeometry(const J_DEFAULT_SHAPE type)noexcept
	{
		auto data = resourceData.basicMeshGuidMap.find(type);
		return data != resourceData.basicMeshGuidMap.end() ?
			static_cast<JMeshGeometry*>(rCash.find(JMeshGeometry::GetStaticResourceType())->second.Get(data->second)) : nullptr;
	}
	JMaterial* JResourceManagerImpl::GetDefaultMaterial(const J_DEFAULT_MATERIAL materialType)noexcept
	{
		const size_t guid = resourceData.basicMaterialGuidMap.find(materialType)->second;
		return static_cast<JMaterial*>(rCash.find(JMaterial::GetStaticResourceType())->second.Get(guid));
	}
	JTexture* JResourceManagerImpl::GetEditorTexture(const J_EDITOR_TEXTURE enumName)noexcept
	{
		auto data = resourceData.defaultTextureMap.find(enumName);
		if (data == resourceData.defaultTextureMap.end())
			return nullptr;

		return static_cast<JTexture*>(rCash.find(JTexture::GetStaticResourceType())->second.GetByIndex(data->second - 1));
	}
	JShader* JResourceManagerImpl::GetDefaultShader(const J_DEFAULT_SHADER shaderType)noexcept
	{
		auto data = resourceData.defaultShaderGuidMap.find(shaderType);
		return data != resourceData.defaultShaderGuidMap.end() ?
			static_cast<JShader*>(rCash.find(JShader::GetStaticResourceType())->second.Get(data->second)) : nullptr;
	}
	JDirectory* JResourceManagerImpl::GetDirectory(const size_t guid)noexcept
	{
		return dCash.GetByGuid(guid);
	}
	JDirectory* JResourceManagerImpl::GetDirectory(const std::string& path)noexcept
	{
		return dCash.GetByPath(path);
	}
	JDirectory* JResourceManagerImpl::GetEditorResourceDirectory()noexcept
	{
		return dCash.GetByPath(JApplicationVariable::GetProjectEditorResourcePath());
	}
	JDirectory* JResourceManagerImpl::GetActivatedDirectory()noexcept
	{  
		return dCash.GetOpenDirectory();
	}
	uint JResourceManagerImpl::GetResourceCount(const J_RESOURCE_TYPE type)noexcept
	{
		return rCash.find(type)->second.Count();
	}
	JResourceObject* JResourceManagerImpl::GetResource(const J_RESOURCE_TYPE type, const size_t guid)noexcept
	{
		return rCash.find(type)->second.Get(guid);
	}
	JResourceObject* JResourceManagerImpl::GetResourceByPath(const J_RESOURCE_TYPE type, const std::string& path)noexcept
	{
		return rCash.find(type)->second.GetByPath(path);
	}
	std::vector<JResourceObject*>::const_iterator JResourceManagerImpl::GetResourceVectorHandle(const J_RESOURCE_TYPE type, uint& resouceCount)noexcept
	{
		return rCash.find(type)->second.GetVectorIter(resouceCount);
	}
	bool JResourceManagerImpl::HasResource(const J_RESOURCE_TYPE rType, const size_t guid)noexcept
	{
		return rCash.find(rType)->second.Has(guid);
	}
	JResourceMangerOwnerInterface* JResourceManagerImpl::OwnerInterface()
	{
		return this;
	}
	JResourceRemoveInterface* JResourceManagerImpl::ResourceRemoveInterface()
	{
		return this;
	}
	JDirectoryRemoveInterface* JResourceManagerImpl::DirectoryRemoveInterface()
	{
		return this;
	}
	JResourceManagerImpl::JEventInterface* JResourceManagerImpl::EvInterface()
	{
		return this;
	}
	void JResourceManagerImpl::LoadSelectorResource()
	{
		J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE);
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
	void JResourceManagerImpl::LoadProjectResource()
	{
		J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE);
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

		if (JSceneManager::Instance().GetOpendSceneCount() == 0)
		{
			JDirectory* projectContentsDir = GetDirectory(JApplicationVariable::GetProjectContentPath());
			JDirectory* defaultSceneDir = projectContentsDir->GetChildDirctory(JApplicationVariable::GetProjectContentScenePath());
			JScene* newScene = JRFI<JScene>::Create(*defaultSceneDir);
			JSceneManager::Instance().TryOpenScene(newScene);
			newScene->SpaceSpatialInterface()->OnSceneSpatialStructure();
		} 
	}
	void JResourceManagerImpl::Terminate()
	{
		StoreResourceData();
		if (engineRootDir != nullptr)
		{
			engineRootDir->DestroyInterface()->BeginForcedDestroy();
			engineRootDir = nullptr;
		}
		if (projectRootDir != nullptr)
		{
			projectRootDir->DestroyInterface()->BeginForcedDestroy();
			projectRootDir = nullptr;
		}

		dCash.Clear();

		for (auto& data : rCash)
			data.second.Clear();
		rCash.clear();
	}
	JResourceObject* JResourceManagerImpl::AddResource(JResourceObject& newResource)
	{
		return rCash.find(newResource.GetResourceType())->second.AddResource(&newResource);
	}
	JDirectory* JResourceManagerImpl::AddDirectory(JDirectory& newDirectory)
	{
		return dCash.Add(&newDirectory);
	}
	bool JResourceManagerImpl::RemoveResource(JResourceObject& resource)noexcept
	{
		if (resource.IsActivated())
			NotifyEvent(resource.GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE, &resource);

		return rCash.find(resource.GetResourceType())->second.RemoveResource(resource);
	} 
	bool JResourceManagerImpl::RemoveJDirectory(JDirectory& dir)noexcept
	{ 
		return dCash.Remove(&dir);
	}
	void JResourceManagerImpl::CreateDefaultTexture(const std::vector<JResourceData::DefaultTextureInfo>& textureInfo)noexcept
	{
		resourceData.defaultTextureMap.clear();

		//수정필요
		uint handleIncrement = JGraphic::Instance().DeviceInterface()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		uint textureCount = (uint)textureInfo.size();
		JDirectory* defaultTextureDir = GetDirectory(JApplicationVariable::GetDefaultResourcePath());
	
		//1 is imgui preserved 
		for (uint i = 1; i < textureCount; ++i)
		{  
			J_OBJECT_FLAG objFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_DO_NOT_SAVE);
			if (textureInfo[i].type == J_EDITOR_TEXTURE::MISSING)
				objFlag = (J_OBJECT_FLAG)(objFlag | OBJECT_FLAG_HIDDEN);

			JResourcePathData pathData(defaultTextureDir->GetWPath() + L"\\" + textureInfo[i].name);
			JTexture* newTexture = JRFI<JTexture>::Load(*defaultTextureDir, pathData);
			//ThrowIfFailedNM(newTexture != nullptr, "Load default texture is error please retry again");

			resourceData.defaultTextureMap.emplace(textureInfo[i].type, i);
		}
	}
	void JResourceManagerImpl::CreateDefaultShader()noexcept
	{
		JDirectory* shaderDir = GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath());
		for (uint i = 0; i < (int)J_DEFAULT_SHADER::COUNTER; ++i)
		{
			J_SHADER_FUNCTION shaderF = DefaultShader::GetShaderFunction((J_DEFAULT_SHADER)i);
			J_OBJECT_FLAG objF = DefaultShader::GetObjectFlag((J_DEFAULT_SHADER)i);

			JShader* res = JRFI<JShader>::Create(std::to_string((int)shaderF),
				Core::MakeGuid(),
				objF,
				*shaderDir,
				0,
				shaderF);

			if (res == nullptr)
				MessageBox(0, L"Default JShader Creator Error", 0, 0);
			else
				resourceData.defaultShaderGuidMap.emplace((J_DEFAULT_SHADER)i, res->GetGuid());
		}
	}
	void JResourceManagerImpl::CreateDefaultMaterial()noexcept
	{
		const J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED |
			OBJECT_FLAG_UNDESTROYABLE |
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
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (J_OBJECT_FLAG)(flag),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetShadow(true);
				newMaterial->SetLight(true);
				break;
			}
			case J_DEFAULT_MATERIAL::DEFAULT_SKY:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (J_OBJECT_FLAG)(flag),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetSkyMaterial(true);
				break;
			}
			case J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetAlbedoOnly(true);
				newMaterial->SetShadowMap(true);
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetDebugMaterial(true);
				newMaterial->SetAlbedoColor(XMFLOAT4(0.75f, 0.1f, 0.1f, 0.6f));
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetDebugMaterial(true);
				newMaterial->SetAlbedoColor(XMFLOAT4(0.1f, 0.75f, 0.1f, 0.6f));
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
					*GetDirectory(JApplicationVariable::GetDefaultResourcePath()), formatIndex);
				newMaterial->SetDebugMaterial(true);
				newMaterial->SetAlbedoColor(XMFLOAT4(0.1f, 0.1f, 0.75f, 0.6f));
				break;
			}
			case J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
			{
				JMaterial* newMaterial = JRFI<JMaterial>::Create(name, guid, (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN),
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
	void JResourceManagerImpl::CreateDefaultMesh()
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
			J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED |
				OBJECT_FLAG_UNDESTROYABLE |
				OBJECT_FLAG_UNEDITABLE |
				OBJECT_FLAG_DO_NOT_SAVE);

			if (i >= JDefaultShape::debugTypeSt)
				flag = (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN);

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
	void JResourceManagerImpl::StoreResourceData()
	{
		for (auto& rData : rCash)
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
	void JResourceManagerImpl::LoadObject()
	{
		resourceIO->LoadProjectResource(projectRootDir);
	}
	void JResourceManagerImpl::RegisterJFunc()
	{
		JResourceObjectFactoryImplBase::RegisterAddStroage(&JResourceManagerImpl::AddResource);
		JDirectoryFactoryImpl::RegisterAddStroage(&JResourceManagerImpl::AddDirectory);
	}
	void JResourceManagerImpl::RegistEvCallable()
	{
		auto lam = [](const size_t& a, const size_t& b) {return a == b; };
		RegistIdenCompareCallable(lam);
	}
	JResourceManagerImpl::JResourceManagerImpl()
	{
		resourceIO = std::make_unique<JResourceIO>();
		RegistEvCallable();

		std::vector<JRI::RTypeHint> rinfo = JRI::GetRTypeHintVec(RESOURCE_ALIGN_TYPE::NONE);
		const uint rinfoCount = (uint)rinfo.size();
		for (uint i = 0; i < rinfoCount; ++i)
			rCash.emplace(rinfo[i].thisType, ResourceStorage());
	}
	JResourceManagerImpl::~JResourceManagerImpl()
	{

	}
}

/*
JMeshGeometry* JResourceManagerImpl::CreateMesh(const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JMeshGeometry* JResourceManagerImpl::CreateMesh(const std::string& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
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
	JMeshGeometry* JResourceManagerImpl::CreateMesh(const std::string& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
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
	JMaterial* JResourceManagerImpl::CreateMaterial(const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
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
	JMaterial* JResourceManagerImpl::CreateMaterial(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
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
	JTexture* JResourceManagerImpl::CreateTexture(const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JModel* JResourceManagerImpl::CreateModel(const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JSkeletonAsset* JResourceManagerImpl::CreateSkeletonAsset(const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JAnimationClip* JResourceManagerImpl::CreateAnimationClip(const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		return nullptr;
	}
	JAnimationController* JResourceManagerImpl::CreateAnimationController(const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
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
	JScene* JResourceManagerImpl::CreateScene(const SCENE_TYPE sceneType, const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
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
	JShader* JResourceManagerImpl::CreateShader(const J_SHADER_FUNCTION newFunctionFlag, J_OBJECT_FLAG flag)noexcept
	{
		flag = (J_OBJECT_FLAG)(OBJECT_FLAG_HIDDEN | (flag ^ (flag & OBJECT_FLAG_HIDDEN)));

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
	JShader* JResourceManagerImpl::CreateShader(const size_t guid, J_OBJECT_FLAG flag, const J_SHADER_FUNCTION newFunctionFlag)noexcept
	{
		if (shader->HasResource(guid))
			return nullptr;

		flag = (J_OBJECT_FLAG)(OBJECT_FLAG_HIDDEN | (flag ^ (flag & OBJECT_FLAG_HIDDEN)));

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
	JDirectory* JResourceManagerImpl::CreateDirectoryFolder(std::string name, const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
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