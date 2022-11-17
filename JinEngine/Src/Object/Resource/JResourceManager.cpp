#include"JResourceManager.h" 
#include"JResourceIO.h"
#include"JResourceObjectFactory.h" 
#include"JResourcePathData.h"
#include"JResourceData.h"

#include"Mesh/JDefaultGeometryGenerator.h"
#include"Mesh/JDefaultShapeType.h"  
#include"Mesh/JMeshGeometry.h"  
#include"Mesh/JStaticMeshGeometry.h"
#include"Mesh/JSkinnedMeshGeometry.h"
#include"Mesh/JDefaultGeometryGenerator.h"
#include"Material/JMaterial.h"  
#include"Texture/JTexture.h" 
#include"Texture/JDefaulTextureType.h"   
#include"Scene/JScene.h" 
#include"Scene/JSceneManager.h"
#include"Shader/JShader.h"

#include"../Directory/JFile.h" 
#include"../Directory/JDirectory.h"
#include"../Directory/JDirectoryFactory.h" 
#include"../Component/Camera/JCamera.h"

#include"../../Core/File/JFileConstant.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Core/Guid/GuidCreator.h" 
#include"../../Utility/JCommonUtility.h"
#include"../../Application/JApplicationVariable.h"
#include"../../Core/Geometry/JDirectXCollisionEx.h"

#include"../../Graphic/JGraphic.h"
#include"../../Graphic/JGraphicDrawList.h"
#include"../../Graphic/JGraphicResourceManager.h"

#define DIMX
#define DIMY
#define DIMZ

#if defined (DIMX) && defined (DIMY) && defined (DIMZ)
int a = 20;
#endif
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
	JResourceObject* JResourceManagerImpl::ResourceStorage::GetByPath(const std::wstring& path)noexcept
	{
		const uint count = rVec.Count();
		for (uint i = 0; i < count; ++i)
		{
			if (rVec.Get(i)->GetPath() == path)
				return rVec.Get(i);
		}
		return nullptr;
	}
	std::vector<JResourceObject*>& JResourceManagerImpl::ResourceStorage::GetVector()
	{
		return rVec.GetVector();
	}
	std::vector<JResourceObject*>::const_iterator JResourceManagerImpl::ResourceStorage::GetVectorIter(uint& count)
	{
		count = rVec.Count();
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
	bool JResourceManagerImpl::ResourceStorage::AddResource(JResourceObject* resource)noexcept
	{
		if (resource == nullptr || rMap.Has(resource->GetGuid()))
			return false;

		JRI::RTypeHint rTypeHint = JRI::GetRTypeHint(resource->GetResourceType());
		if (rTypeHint.isFrameResource)
		{
			auto setFrameDirtyCallable = JRI::GetSetFrameDirtyCallable(resource->GetResourceType());
			setFrameDirtyCallable(nullptr, *resource);

			auto setFrameBuffIndexCallable = JRI::GetSetFrameBuffIndexCallable(resource->GetResourceType());
			setFrameBuffIndexCallable(nullptr, *resource, rVec.Count());
		}

		rVec.Add(resource);
		rMap.Add(resource, resource->GetGuid());
		return true;
	}
	bool JResourceManagerImpl::ResourceStorage::RemoveResource(JResourceObject& resource)noexcept
	{
		static auto equalLam = [](JResourceObject* a, JResourceObject* b) {return a->GetGuid() == b->GetGuid(); };
		JRI::RTypeHint rTypeHint = JRI::GetRTypeHint(resource.GetResourceType());
		int index = rVec.GetIndex(&resource, equalLam);

		if (index == -1)
			return false;

		bool res00 = rVec.Remove(index);
		bool res01 = rMap.Remove(resource.GetGuid());

		if (rTypeHint.isFrameResource)
		{
			auto setFrameDirtyCallable = JRI::GetSetFrameDirtyCallable(resource.GetResourceType());
			rVec.ApplyFunc(index, setFrameDirtyCallable);

			auto setFrameBuffIndexCallable = JRI::GetSetFrameBuffIndexCallable(resource.GetResourceType());
			rVec.ApplyFuncByIndex(index, setFrameBuffIndexCallable);
		}

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
	JDirectory* JResourceManagerImpl::DirectoryStorage::GetByPath(const std::wstring& path)
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
	bool JResourceManagerImpl::DirectoryStorage::Add(JDirectory* dir)noexcept
	{
		if (dir == nullptr || dMap.Has(dir->GetGuid()))
			return false;

		dVec.Add(dir);
		dMap.Add(dir, dir->GetGuid());
		return true;
	}
	bool JResourceManagerImpl::DirectoryStorage::Remove(JDirectory* dir)noexcept
	{
		if (dir == nullptr)
			return false;

		if (dir->IsActivated())
			dir->DeActivate();

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
		JResourceObject* resource = resourceData->GetDefaultResource(type);
		if (resource != nullptr)
			return static_cast<JMeshGeometry*>(resource);
		else
		{
			JFile* file = GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath())->GetFile(JDefaultShape::ConvertToName(type));
			if (file != nullptr)
				return static_cast<JMeshGeometry*>(file->GetResource());
			else
			{
				assert("GetDefaultMeshGeometry Error");
				return nullptr;
			}
		}

	}
	JMaterial* JResourceManagerImpl::GetDefaultMaterial(const J_DEFAULT_MATERIAL type)noexcept
	{
		JResourceObject* resource = resourceData->GetDefaultResource(type);
		if (resource != nullptr)
			return static_cast<JMaterial*>(resource);
		else
		{
			JFile* file = GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath())->GetFile(JDefaultMateiral::ConvertToName(type));
			if (file != nullptr)
				return static_cast<JMaterial*>(file->GetResource());
			else
			{
				assert("GetDefaultMaterial Error");
				return nullptr;
			}
		}
	}
	JTexture* JResourceManagerImpl::GetDefaultTexture(const J_DEFAULT_TEXTURE type)noexcept
	{
		JResourceObject* resource = resourceData->GetDefaultResource(type);
		if (resource != nullptr)
			return static_cast<JTexture*>(resource);
		else
		{ 
			JFile* file = GetDirectory(JApplicationVariable::GetEngineDefaultResourcePath())->GetFile(JDefaultTexture::GetName(type));
			if (file != nullptr)
				return static_cast<JTexture*>(file->GetResource());
			else
			{
				assert("GetDefaultTexture Error");
				return nullptr;
			}
		}

	}
	JShader* JResourceManagerImpl::GetDefaultShader(const J_DEFAULT_GRAPHIC_SHADER type)noexcept
	{
		JResourceObject* resource = resourceData->GetDefaultResource(type);
		if (resource != nullptr)
			return static_cast<JShader*>(resource);
		else
		{
			J_GRAPHIC_SHADER_FUNCTION shaderF = JDefaultShader::GetShaderFunction(type);
			JFile* file = GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath())->GetFile(JShaderType::ConvertToName(shaderF));
			if (file != nullptr)
				return static_cast<JShader*>(file->GetResource());
			else
			{
				assert("GetDefaultShader Error");
				return nullptr;
			}
		}
	}
	JShader* JResourceManagerImpl::GetDefaultShader(const J_DEFAULT_COMPUTE_SHADER type)noexcept
	{
		JResourceObject* resource = resourceData->GetDefaultResource(type);
		if (resource != nullptr)
			return static_cast<JShader*>(resource);
		else
		{
			J_COMPUTE_SHADER_FUNCTION shaderF = JDefaultShader::GetComputeShaderFunction(type);
			JFile* file = GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath())->GetFile(JShaderType::ConvertToName(shaderF));
			if (file != nullptr)
				return static_cast<JShader*>(file->GetResource());
			else
			{
				assert("GetDefaultShader Error");
				return nullptr;
			}
		}
	}
	JDirectory* JResourceManagerImpl::GetDirectory(const size_t guid)noexcept
	{
		return dCash.GetByGuid(guid);
	}
	JDirectory* JResourceManagerImpl::GetDirectory(const std::wstring& path)noexcept
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
	JResourceObject* JResourceManagerImpl::GetResourceByPath(const J_RESOURCE_TYPE type, const std::wstring& path)noexcept
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
	JResourceMangerAppInterface* JResourceManagerImpl::AppInterface()
	{
		return this;
	}
	JResourceStorageInterface* JResourceManagerImpl::ResourceStorageInterface()
	{
		return this;
	}
	JDirectoryStorageInterface* JResourceManagerImpl::DirectoryStorageInterface()
	{
		return this;
	}
	JResourceManagerImpl::JEventInterface* JResourceManagerImpl::EvInterface()
	{
		return this;
	}
	void JResourceManagerImpl::StoreProjectResource()
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
	void JResourceManagerImpl::LoadSelectorResource()
	{
		resourceData->Initialize();
		J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_DO_NOT_SAVE);
		engineRootDir = JDFI::CreateRoot(JApplicationVariable::GetEnginePath(), Core::MakeGuid(), rootFlag);
		resourceIO->LoadEngineDirectory(engineRootDir);
		resourceIO->LoadEngineResource(engineRootDir);
		CreateDefaultTexture(resourceData->selectorTextureType);
	}
	void JResourceManagerImpl::LoadProjectResource()
	{
		resourceData->Initialize();
		J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_DO_NOT_SAVE);
		engineRootDir = JDFI::CreateRoot(JApplicationVariable::GetEnginePath(), Core::MakeGuid(), rootFlag);
		resourceIO->LoadEngineDirectory(engineRootDir);
		resourceIO->LoadEngineResource(engineRootDir);

		rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_DO_NOT_SAVE);
		projectRootDir = resourceIO->LoadRootDirectory(JApplicationVariable::GetProjectPath(), rootFlag);
		projectRootDir->OCInterface()->OpenDirectory();
		resourceIO->LoadProjectDirectory(projectRootDir);
		resourceIO->LoadProjectResource(projectRootDir);

		CreateDefaultTexture(resourceData->projectTextureType);
		CreateDefaultShader();
		CreateDefaultMaterial();
		CreateDefaultMesh();

		if (JSceneManager::Instance().GetOpendSceneCount() == 0)
		{
			JDirectory* projectContentsDir = GetDirectory(JApplicationVariable::GetProjectContentPath());
			JDirectory* defaultSceneDir = projectContentsDir->GetChildDirctory(JApplicationVariable::GetProjectContentScenePath());
			JScene* newScene = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>(defaultSceneDir));
			JSceneManager::Instance().TryOpenScene(newScene, false);
			JSceneManager::Instance().SetMainScene(newScene);

			newScene->SpaceSpatialInterface()->ActivateSpaceSpatial(true);
			((JResourceObjectInterface*)newScene)->CallStoreResource();
		}
		DestroyUnusedResource(J_RESOURCE_TYPE::SHADER, false);
	}
	void JResourceManagerImpl::Initialize()
	{
		std::vector<JRI::RTypeHint> rinfo = JRI::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::NONE);
		const uint rinfoCount = (uint)rinfo.size();
		for (uint i = 0; i < rinfoCount; ++i)
			rCash.emplace(rinfo[i].thisType, ResourceStorage());
	}
	void JResourceManagerImpl::Terminate()
	{
		StoreProjectResource();
		DeleteUnuseMetafile();

		auto rHintVec = JResourceObjectInterface::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY_REVERSE);
		for (uint i = 0; i < rHintVec.size(); ++i)
		{
			std::vector<JResourceObject*> copyVec = rCash.find(rHintVec[i].thisType)->second.GetVector();
			for (uint j = 0; j < copyVec.size(); ++j)
				JObject::BegineForcedDestroy(copyVec[j]);
		}

		if (engineRootDir != nullptr)
		{
			JObject::BegineForcedDestroy(engineRootDir); 
			engineRootDir = nullptr;
		}
		if (projectRootDir != nullptr)
		{
			JObject::BegineForcedDestroy(projectRootDir);
			projectRootDir = nullptr;
		}
		JReflectionInfo::Instance().SearchIntance();
		dCash.Clear();
		for (auto& data : rCash)
			data.second.Clear();
		rCash.clear();
		resourceData->Clear();
	}
	void JResourceManagerImpl::DeleteUnuseMetafile()
	{
		//추가필요
	}
	bool JResourceManagerImpl::AddResource(JResourceObject& newResource)noexcept
	{
		return rCash.find(newResource.GetResourceType())->second.AddResource(&newResource);
	}
	bool JResourceManagerImpl::RemoveResource(JResourceObject& resource)noexcept
	{
		if (resource.IsActivated())
			NotifyEvent(resource.GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE, &resource);

		return rCash.find(resource.GetResourceType())->second.RemoveResource(resource);
	}
	bool JResourceManagerImpl::AddJDirectory(JDirectory& newDirectory)noexcept
	{
		return dCash.Add(&newDirectory);
	}
	bool JResourceManagerImpl::RemoveJDirectory(JDirectory& dir)noexcept
	{
		return dCash.Remove(&dir);
	}
	void JResourceManagerImpl::DestroyUnusedResource(const J_RESOURCE_TYPE rType, bool isIgnreUndestroyableFlag)
	{
		//추가필요
		//engine resource destory -> window file destory
		std::vector<JResourceObject*>& rvec = rCash.find(rType)->second.GetVector();
		std::vector<JResourceObject*> copied = rvec;
		for (uint i = 0; i < copied.size(); ++i)
		{
			bool canDestroy = (!copied[i]->HasFlag(OBJECT_FLAG_UNDESTROYABLE)) || isIgnreUndestroyableFlag;
			//ref count == 0 is deactivated state
			if (!copied[i]->IsActivated() && canDestroy)
			{
				JResourceObjectInterface* Ir = copied[i];
				Ir->DeleteRFile();
				copied[i] = nullptr;
			}
		}
	}
	void JResourceManagerImpl::CreateDefaultTexture(const std::vector<J_DEFAULT_TEXTURE>& textureType)
	{ 
		//수정필요
		uint handleIncrement = JGraphic::Instance().DeviceInterface()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		uint textureCount = (uint)textureType.size();
		JDirectory* textureDir = GetDirectory(JApplicationVariable::GetEngineDefaultResourcePath());
 
		//1 is imgui preserved 
		for (uint i = 0; i < textureCount; ++i)
		{
			J_OBJECT_FLAG objFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE);
			if (textureType[i] == J_DEFAULT_TEXTURE::MISSING)
				objFlag = (J_OBJECT_FLAG)(objFlag | OBJECT_FLAG_HIDDEN);

			const bool isUse = JDefaultTexture::IsDefaultUse(textureType[i]);
			std::wstring foldernam;
			std::wstring name;
			std::wstring format;
			JCUtil::DecomposeFileName(JDefaultTexture::GetName(textureType[i]), name, format);
			JFile* file = textureDir->GetFile(name);
	 
			if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::TEXTURE && file->GetFormat() == format)
				resourceData->RegisterDefaultResource(textureType[i], Core::GetUserPtr<JTexture>(file->GetResource()), isUse);
			else
			{
				const std::wstring oriPath = Core::JFileConstant::MakeFilePath(textureDir->GetPath(), JDefaultTexture::GetName(textureType[i]));
				JTexture* newTexture = JRFI<JTexture>::Create(Core::JPtrUtil::MakeOwnerPtr<JTexture::InitData>(name, Core::MakeGuid(), objFlag, textureDir, oriPath));
				if (textureType[i] == J_DEFAULT_TEXTURE::DEFAULT_SKY)
					newTexture->SetTextureType(Graphic::J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE);
				ThrowIfFailedN(newTexture != nullptr);
				resourceData->RegisterDefaultResource(textureType[i], Core::GetUserPtr(newTexture), isUse); 
			}
		}
	}
	void JResourceManagerImpl::CreateDefaultShader()
	{
		JDirectory* shaderDir = GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath());
		for (uint i = 0; i < (int)J_DEFAULT_GRAPHIC_SHADER::COUNTER; ++i)
		{
			const J_DEFAULT_GRAPHIC_SHADER type = (J_DEFAULT_GRAPHIC_SHADER)i;
			const J_GRAPHIC_SHADER_FUNCTION shaderF = JDefaultShader::GetShaderFunction(type);
			const J_OBJECT_FLAG objF = JDefaultShader::GetObjectFlag(type);
 
			std::wstring shaderName = JShaderType::ConvertToName(shaderF);
			JFile* file = shaderDir->GetFile(shaderName);

			if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::SHADER)
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr<JShader>(file->GetResource()), false);
			else
			{
				JShader* newShader = JRFI<JShader>::Create(Core::JPtrUtil::MakeOwnerPtr<JShader::InitData>(objF, shaderF));
				ThrowIfFailedN(newShader != nullptr); 
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr(newShader), false);
			}
		}
		  
		for (uint i = 0; i < (int)J_DEFAULT_COMPUTE_SHADER::COUNTER; ++i)
		{
			const J_DEFAULT_COMPUTE_SHADER type = (J_DEFAULT_COMPUTE_SHADER)i;
			const J_COMPUTE_SHADER_FUNCTION shaderF = JDefaultShader::GetComputeShaderFunction(type);
			const J_OBJECT_FLAG objF = JDefaultShader::GetObjectFlag(type);

			const bool isUse = JDefaultShader::IsDefaultUse(type);
			std::wstring shaderName = JShaderType::ConvertToName(shaderF);
			JFile* file = shaderDir->GetFile(shaderName); 
			if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::SHADER)
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr<JShader>(file->GetResource()), isUse);
			else
			{
				JShader* newShader = JRFI<JShader>::Create(Core::JPtrUtil::MakeOwnerPtr<JShader::InitData>(objF, SHADER_FUNCTION_NONE, shaderF));
				ThrowIfFailedN(newShader != nullptr);
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr(newShader), isUse);
			}
		}
	}
	void JResourceManagerImpl::CreateDefaultMaterial()
	{
		const J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);

		JDirectory* matDir = GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath());
		for (uint i = 0; i < (int)J_DEFAULT_MATERIAL::COUNTER; ++i)
		{
			//BasicMaterial format is all .mat( default format)
			J_DEFAULT_MATERIAL type = (J_DEFAULT_MATERIAL)i;
			const bool isUse = JDefaultMateiral::IsDefaultUse(type);

			const std::wstring name = JDefaultMateiral::ConvertToName(type);
			JFile* file = matDir->GetFile(name);
			if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::MATERIAL)
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr<JMaterial>(file->GetResource()), isUse);
			else
			{ 
				JMaterial* newMaterial = nullptr;
				size_t guid = Core::MakeGuid();
				switch (type)
				{
				case J_DEFAULT_MATERIAL::DEFAULT_STANDARD:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, flag, matDir));
					newMaterial->SetShadow(true);
					newMaterial->SetLight(true);
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEFAULT_SKY:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, flag, matDir));
					newMaterial->SetSkyMaterial(true);
					newMaterial->SetAlbedoMap(GetDefaultTexture(J_DEFAULT_TEXTURE::DEFAULT_SKY));
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), matDir));
					newMaterial->SetShadowMapWrite(true);
					newMaterial->SetAlphaClip(true);
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), matDir));
					newMaterial->SetDebugMaterial(true);
					newMaterial->SetAlbedoColor(XMFLOAT4(0.75f, 0.1f, 0.1f, 0.6f));
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), matDir));
					newMaterial->SetDebugMaterial(true);
					newMaterial->SetAlbedoColor(XMFLOAT4(0.1f, 0.75f, 0.1f, 0.6f));
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), matDir));
					newMaterial->SetDebugMaterial(true);
					newMaterial->SetAlbedoColor(XMFLOAT4(0.1f, 0.1f, 0.75f, 0.6f));
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), matDir));
					newMaterial->SetDebugMaterial(true);
					newMaterial->SetAlbedoColor(XMFLOAT4(0.75f, 0.75f, 0.05f, 0.6f));
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), matDir));
					newMaterial->SetBoundingObjectDepthTest(true); 
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				default:
					break;
				}

				ThrowIfFailedN(newMaterial != nullptr);
				resourceData->RegisterDefaultResource(type, GetUserPtr(newMaterial), isUse);
			}
		}
	}
	void JResourceManagerImpl::CreateDefaultMesh()
	{
		auto createCubeLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateCube(1, 1, 1, 3); };
		auto createGridLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateGrid(40.0f, 60.0f, 60, 40); };
		auto createSphereLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateSphere(0.5f, 20, 20); };
		auto createCylinderLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20); };
		auto createQuadLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f); };
		auto createLineBBoxLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateLineBoundingBox(); };
		auto createTriangleBBoxLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateTriangleBoundingBox(); };
		auto createBFrustumLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateBoundingFrustum(); };

		using CreateStaticMesh = Core::JStaticCallableType<JStaticMeshData, JDefaultGeometryGenerator&>;
		std::vector<CreateStaticMesh::Callable> callableVec
		{
			(CreateStaticMesh::Ptr)createCubeLam, (CreateStaticMesh::Ptr)createGridLam,
			(CreateStaticMesh::Ptr)createSphereLam, (CreateStaticMesh::Ptr)createCylinderLam,
			(CreateStaticMesh::Ptr)createQuadLam, (CreateStaticMesh::Ptr)createLineBBoxLam,
			 (CreateStaticMesh::Ptr)createTriangleBBoxLam, (CreateStaticMesh::Ptr)createBFrustumLam
		};

		JDefaultGeometryGenerator geoGen;
		JDirectory* meshDir = GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath());
		//0 = empty
		for (int i = 1; i < (int)J_DEFAULT_SHAPE::COUNT; ++i)
		{
			const J_DEFAULT_SHAPE shapeType = (J_DEFAULT_SHAPE)i;
			const J_MESHGEOMETRY_TYPE meshType = JDefaultShape::GetMeshType(shapeType);
			const bool isUse = JDefaultShape::IsDefaultUse(shapeType);

			const std::wstring meshName = JDefaultShape::ConvertToName(shapeType);
			JFile* file = meshDir->GetFile(meshName); 

			if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::MESH)
			{
				if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
					resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr<JStaticMeshGeometry>(file->GetResource()), isUse);
				else if (meshType == J_MESHGEOMETRY_TYPE::SKINNED)
					resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr<JSkinnedMeshGeometry>(file->GetResource()), isUse);
				else
					assert("CreateDefaultMesh MeshType Error");
			}
			else
			{
				if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
				{
					int callableIndex = i - 1;
					JStaticMeshData staticMeshData = callableVec[callableIndex](nullptr, geoGen);
					staticMeshData.CreateBoundingObject();

					Core::JOwnerPtr<JStaticMeshGroup> group = Core::JPtrUtil::MakeOwnerPtr< JStaticMeshGroup>();
					group->AddMeshData(std::move(staticMeshData));

					const size_t guid = Core::MakeGuid();
					J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);

					if (i >= JDefaultShape::debugTypeSt)
						flag = (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN);

					JStaticMeshGeometry* newMesh = JRFI<JStaticMeshGeometry>::Create(Core::JPtrUtil::MakeOwnerPtr<JMeshGeometry::JMeshInitData>
						(meshName, guid, flag, meshDir, JRI::CallFormatIndex(J_RESOURCE_TYPE::MESH, L".mesh"), std::move(group)));
					ThrowIfFailedN(newMesh != nullptr);
					resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr(newMesh), isUse);
				}
				else if (meshType == J_MESHGEOMETRY_TYPE::SKINNED)
				{
					 //추가필요
					assert("CreateDefaultMesh MeshType Error.... skinned");
				}
				else
					assert("CreateDefaultMesh MeshType Error");			 
			}
		}
	}
	JResourceManagerImpl::JResourceManagerImpl()
		:JEventManager([](const size_t& a, const size_t& b) {return a == b; })
	{
		resourceData= std::make_unique<JResourceData>();
		resourceIO = std::make_unique<JResourceIO>();
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

		return CreateMaterial(JResourceObject::GetDefaultName<JMaterial>(), Core::MakeGuid(), flag, selectedDirctory);
	}
	JMaterial* JResourceManagerImpl::CreateMaterial(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* selectedDirctory)noexcept
	{
		const size_t shaderGuid = resourceData->defaultGraphicShaderGuidMap[J_DEFAULT_GRAPHIC_SHADER::DEFAULT_STANDARD_SHADER];

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
	JShader* JResourceManagerImpl::CreateShader(const J_GRAPHIC_SHADER_FUNCTION newFunctionFlag, J_OBJECT_FLAG flag)noexcept
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
	JShader* JResourceManagerImpl::CreateShader(const size_t guid, J_OBJECT_FLAG flag, const J_GRAPHIC_SHADER_FUNCTION newFunctionFlag)noexcept
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