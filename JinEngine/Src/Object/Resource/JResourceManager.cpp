#include"JResourceManager.h" 
#include"JResourceIO.h"
#include"JResourceObjectFactory.h" 
#include"JResourcePathData.h"
#include"JResourceData.h"
#include"JResourceImporter.h"

#include"Mesh/JDefaultGeometryGenerator.h"
#include"Mesh/JDefaultShapeType.h"  
#include"Mesh/JMeshGeometry.h"  
#include"Mesh/JStaticMeshGeometry.h"
#include"Mesh/JSkinnedMeshGeometry.h"
#include"Mesh/JDefaultGeometryGenerator.h"
#include"Material/JMaterial.h"  
#include"Material/JDefaultMaterialSetting.h"
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
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Core/Guid/GuidCreator.h" 

#include"../../Utility/JCommonUtility.h"
#include"../../Application/JApplicationVariable.h"
#include"../../Core/Geometry/JDirectXCollisionEx.h"

#include"../../Graphic/JGraphic.h"
#include"../../Graphic/JGraphicDrawList.h" 

#include"../JModifiedObjectInfo.h"
#include"../../Editor/Interface/JEditorObjectInterface.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Private
	{
		class JModifiedObjectInfoReader : public Editor::JEditorModifedObjectStructureInterface
		{
			using ModVector = Editor::JModifiedObjectInfoVector::ObjectVector;
		public:
			ModVector& GetVector()
			{
				return GetModifiedObjectInfoVec();
			}
		};
	}
	//ResourceStorage
	JResourceObject* JResourceManagerImpl::ResourceStorage::Get(const size_t guid)const noexcept
	{
		return rMap.Get(guid);
	}
	JResourceObject* JResourceManagerImpl::ResourceStorage::GetByIndex(const uint index)const noexcept
	{
		return rVec.Get(index);
	}
	JResourceObject* JResourceManagerImpl::ResourceStorage::GetByPath(const std::wstring& path)const noexcept
	{
		const uint count = rVec.Count();
		for (uint i = 0; i < count; ++i)
		{
			if (rVec.Get(i)->GetPath() == path)
				return rVec.Get(i);
		}
		return nullptr;
	}
	std::vector<JResourceObject*> JResourceManagerImpl::ResourceStorage::GetVector()const noexcept
	{
		return rVec.GetVector();
	}
	std::vector<JResourceObject*>& JResourceManagerImpl::ResourceStorage::GetVectorAddress() noexcept
	{
		return rVec.GetVectorAddress();
	}
	std::vector<JResourceObject*>::const_iterator JResourceManagerImpl::ResourceStorage::GetVectorCIter(uint& count)const noexcept
	{
		count = rVec.Count();
		return rVec.GetCBegin();
	}
	bool JResourceManagerImpl::ResourceStorage::Has(const size_t guid)const noexcept
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
	JDirectory* JResourceManagerImpl::DirectoryStorage::Get(const uint index)const noexcept
	{
		return dVec.Get(index);
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::GetByGuid(const size_t guid)const noexcept
	{
		return dMap.Get(guid);
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::GetByPath(const std::wstring& path)const noexcept
	{
		const uint count = dVec.Count();
		for (uint i = 0; i < count; ++i)
		{
			if (dVec[i]->GetPath() == path)
				return dVec[i];
		}
		return nullptr;
	}
	JDirectory* JResourceManagerImpl::DirectoryStorage::GetOpenDirectory()const noexcept
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
		return true;
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
			const J_GRAPHIC_SHADER_FUNCTION shaderF = JDefaultShader::GetShaderFunction(type);
			const JShaderGraphicPsoCondition psoCondition = JDefaultShader::GetShaderGraphicPso(type);
			JFile* file = GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath())->GetFile(JShaderType::ConvertToName(shaderF, psoCondition.UniqueID()));
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
		return dStorage.GetByGuid(guid);
	}
	JDirectory* JResourceManagerImpl::GetDirectory(const std::wstring& path)noexcept
	{
		return dStorage.GetByPath(path);
	}
	JDirectory* JResourceManagerImpl::GetEditorResourceDirectory()noexcept
	{
		return dStorage.GetByPath(JApplicationVariable::GetProjectEditorResourcePath());
	}
	JDirectory* JResourceManagerImpl::GetActivatedDirectory()noexcept
	{
		return dStorage.GetOpenDirectory();
	}
	uint JResourceManagerImpl::GetResourceCount(const J_RESOURCE_TYPE type)noexcept
	{
		return rStorage.find(type)->second.Count();
	}
	JResourceObject* JResourceManagerImpl::GetResource(const J_RESOURCE_TYPE type, const size_t guid)noexcept
	{
		return rStorage.find(type)->second.Get(guid);
	}
	JResourceObject* JResourceManagerImpl::GetResourceByPath(const J_RESOURCE_TYPE type, const std::wstring& path)noexcept
	{
		return rStorage.find(type)->second.GetByPath(path);
	}
	std::vector<JResourceObject*>::const_iterator JResourceManagerImpl::GetResourceVectorHandle(const J_RESOURCE_TYPE type, uint& resouceCount)noexcept
	{
		return rStorage.find(type)->second.GetVectorCIter(resouceCount);
	}
	bool JResourceManagerImpl::HasResource(const J_RESOURCE_TYPE rType, const size_t guid)noexcept
	{
		return rStorage.find(rType)->second.Has(guid);
	}
	JResourceMangerAppInterface* JResourceManagerImpl::AppInterface()
	{
		return this;
	}
	JResourceManagerImpl::JEventInterface* JResourceManagerImpl::EvInterface()
	{
		return this;
	}
	void JResourceManagerImpl::Initialize()
	{
		std::vector<JRI::RTypeHint> rinfo = JRI::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::NONE);
		const uint rinfoCount = (uint)rinfo.size();
		for (uint i = 0; i < rinfoCount; ++i)
			rStorage.emplace(rinfo[i].thisType, ResourceStorage());
	}
	void JResourceManagerImpl::Terminate()
	{
		//StoreProjectResource();
		auto rHintVec = JResourceObjectInterface::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY_REVERSE);
		for (uint i = 0; i < rHintVec.size(); ++i)
		{
			std::vector<JResourceObject*> copyVec = rStorage.find(rHintVec[i].thisType)->second.GetVector();
			for (uint j = 0; j < copyVec.size(); ++j)
				JObject::BegineForcedDestroy(copyVec[j]);
		}
		resourceData->Clear();
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
		JReflectionInfo::Instance().SearchInstance();
		dStorage.Clear();
		for (auto& data : rStorage)
			data.second.Clear();
		rStorage.clear();
	}
	void JResourceManagerImpl::StoreProjectResource()
	{ 
		auto modInfo = Private::JModifiedObjectInfoReader{}.GetVector();
		for (const auto& data : modInfo)
		{
			Core::JIdentifier* obj = Core::GetRawPtr(data->typeName, data->guid);
			if (data->isModified && data->isStore && obj->GetTypeInfo().IsChildOf<JResourceObject>())
			{
				static_cast<JResourceObjectInterface*>(obj)->CallStoreResource();
				data->isModified = false;
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
			JScene* newScene = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>(defaultSceneDir, J_SCENE_USE_CASE_TYPE::MAIN));
			JSceneManager::Instance().TryOpenScene(newScene, false);
			JSceneManager::Instance().SetMainScene(newScene); 
			static_cast<JResourceObjectInterface*>(newScene)->CallStoreResource();
		}
		DestroyUnusedResource(J_RESOURCE_TYPE::SHADER, false);
	}
	bool JResourceManagerImpl::AddType(JResourceObject* newResource)noexcept
	{
		return rStorage.find(newResource->GetResourceType())->second.AddResource(newResource);
	}
	bool JResourceManagerImpl::RemoveType(JResourceObject* resource)noexcept
	{
		if (resource->IsActivated())
			NotifyEvent(resource->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE, resource);

		return rStorage.find(resource->GetResourceType())->second.RemoveResource(*resource);
	}
	bool JResourceManagerImpl::AddType(JDirectory* newDirectory)noexcept
	{ 
		return dStorage.Add(newDirectory);
	}
	bool JResourceManagerImpl::RemoveType(JDirectory* dir)noexcept
	{
		return dStorage.Remove(dir);
	}
	void JResourceManagerImpl::DestroyUnusedResource(const J_RESOURCE_TYPE rType, bool isIgnreUndestroyableFlag)
	{
		//추가필요
		//engine resource destory -> window file destory
		std::vector<JResourceObject*>& rvec = rStorage.find(rType)->second.GetVectorAddress();
		std::vector<JResourceObject*> copied = rvec;
		for (uint i = 0; i < copied.size(); ++i)
		{
			bool canDestroy = (!copied[i]->HasFlag(OBJECT_FLAG_UNDESTROYABLE)) || isIgnreUndestroyableFlag;
			//ref count == 0 is deactivated state
			if (!copied[i]->IsActivated() && canDestroy)
			{
				JResourceObjectInterface* Ir = copied[i];
				Ir->DeleteRFile();
				JObject::BegineForcedDestroy(copied[i]);
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
			const J_OBJECT_FLAG objFlag = JDefaultTexture::GetFlag(textureType[i]);	
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
					newTexture->SetTextureType(Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE);
				ThrowIfFailedN(newTexture != nullptr);
				resourceData->RegisterDefaultResource(textureType[i], Core::GetUserPtr(newTexture), isUse);
				static_cast<JResourceObjectInterface*>(newTexture)->CallStoreResource();
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
			const JShaderGraphicPsoCondition psoCondition = JDefaultShader::GetShaderGraphicPso(type);
			const J_OBJECT_FLAG objF = JDefaultShader::GetObjectFlag(type);

			const bool isUse = JDefaultShader::IsDefaultUse(type);
			std::wstring shaderName = JShaderType::ConvertToName(shaderF, psoCondition.UniqueID());
			JFile* file = shaderDir->GetFile(shaderName);

			if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::SHADER)
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr<JShader>(file->GetResource()), isUse);
			else
			{
				JShader* newShader = JRFI<JShader>::Create(Core::JPtrUtil::MakeOwnerPtr<JShader::InitData>(objF, shaderF, psoCondition));
				ThrowIfFailedN(newShader != nullptr);
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr(newShader), isUse);
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
				JShader* newShader = JRFI<JShader>::Create(Core::JPtrUtil::MakeOwnerPtr<JShader::InitData>(objF, SHADER_FUNCTION_NONE, JShaderGraphicPsoCondition(), shaderF));
				ThrowIfFailedN(newShader != nullptr);
				resourceData->RegisterDefaultResource(type, Core::GetUserPtr(newShader), isUse);
			}
		}
	}
	void JResourceManagerImpl::CreateDefaultMaterial()
	{
		auto debugLam = [](JDirectory* ownerDir,
			const std::wstring& name,
			const size_t guid,
			const J_OBJECT_FLAG flag,
			const XMFLOAT4& color,
			const bool isLine)
		{
			JMaterial* newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
				(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), ownerDir));
			JDefaultMaterialSetting::SetDebug(newMaterial, isLine, color);
			((JResourceObjectInterface*)newMaterial)->CallStoreResource();
			return newMaterial;
		};

		JDirectory* matDir = GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath());
		for (uint i = 0; i < (int)J_DEFAULT_MATERIAL::COUNTER; ++i)
		{
			//BasicMaterial format is all .mat( default format)
			const J_DEFAULT_MATERIAL type = (J_DEFAULT_MATERIAL)i;
			const J_OBJECT_FLAG flag = JDefaultMateiral::GetFlag(type);
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
					JDefaultMaterialSetting::SetStandard(newMaterial); 
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEFAULT_SKY:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, flag, matDir));
					JDefaultMaterialSetting::SetSky(newMaterial, GetDefaultTexture(J_DEFAULT_TEXTURE::DEFAULT_SKY));
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
				{
					newMaterial = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>
						(name, guid, Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), matDir));
					JDefaultMaterialSetting::SetShadowMap(newMaterial);
					((JResourceObjectInterface*)newMaterial)->CallStoreResource();
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_RED:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.75f, 0.1f, 0.1f, 0.8f), false);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_GREEN:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.1f, 0.75f, 0.1f, 0.8f), false);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_BLUE:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.1f, 0.1f, 0.75f, 0.8f), false);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_YELLOW:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.75f, 0.75f, 0.05f, 0.8f), false);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.75f, 0.1f, 0.1f, 0.8f), true);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.1f, 0.75f, 0.1f, 0.8f), true);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.1f, 0.1f, 0.75f, 0.8f), true);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.75f, 0.75f, 0.05f, 0.8f), true);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_GRAY:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.325f, 0.325f, 0.325f, 0.8f), true);
					break;
				}
				case J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK:
				{
					newMaterial = debugLam(matDir, name, guid, flag, XMFLOAT4(0.015f, 0.015f, 0.015f, 0.8f), true);
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
				static_cast<JResourceObjectInterface*>(newMaterial)->CallStoreResource();
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
		auto createCircleLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateCircle(1.2f, 1.1f); };
		auto createScaleArrowLam = [](JDefaultGeometryGenerator& geoGen)
		{
			JStaticMeshData cyilinderMesh = geoGen.CreateCylinder(0.125f, 0.125f, 2.04f, 10, 10);
			JStaticMeshData cubeMesh = geoGen.CreateCube(0.5f, 0.5f, 0.5f, 1);

			const DirectX::BoundingBox cyilinderBBox = cyilinderMesh.GetBBox();
			const DirectX::BoundingBox cubeBBox = cubeMesh.GetBBox();

			const float cyilinderYOffset = (-cyilinderBBox.Center.y) + cyilinderBBox.Extents.y;
			const float cubeYOffset = cyilinderYOffset + cyilinderBBox.Center.y + cyilinderBBox.Extents.y + cubeBBox.Center.y;
			DirectX::XMFLOAT3 cyilinderOffset = DirectX::XMFLOAT3(0, cyilinderYOffset, 0);
			DirectX::XMFLOAT3 cubeOffset = DirectX::XMFLOAT3(0, cubeYOffset, 0);

			cyilinderMesh.AddPositionOffset(cyilinderOffset);
			cubeMesh.AddPositionOffset(cubeOffset);
			cyilinderMesh.Merge(cubeMesh);
			cyilinderMesh.SetName(L"ScaleArrow");
			return cyilinderMesh;
		};
		auto createLineLam = [](JDefaultGeometryGenerator& geoGen) {return geoGen.CreateLine(3); };

		using CreateStaticMesh = Core::JStaticCallableType<JStaticMeshData, JDefaultGeometryGenerator&>;
		std::unordered_map<J_DEFAULT_SHAPE, CreateStaticMesh::Callable> callableVec
		{
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE, (CreateStaticMesh::Ptr)createCubeLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID, (CreateStaticMesh::Ptr)createGridLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE, (CreateStaticMesh::Ptr)createSphereLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER, (CreateStaticMesh::Ptr)createCylinderLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD, (CreateStaticMesh::Ptr)createQuadLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, (CreateStaticMesh::Ptr)createLineBBoxLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE, (CreateStaticMesh::Ptr)createTriangleBBoxLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM, (CreateStaticMesh::Ptr)createBFrustumLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_CIRCLE, (CreateStaticMesh::Ptr)createCircleLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_SCALE_ARROW, (CreateStaticMesh::Ptr)createScaleArrowLam},
			{J_DEFAULT_SHAPE::DEFAULT_SHAPE_LINE, (CreateStaticMesh::Ptr)createLineLam}
		};

		JDefaultGeometryGenerator geoGen;
		JDirectory* projectDefualDir = GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath());
		JDirectory* engineDefaultDir = GetDirectory(JApplicationVariable::GetEngineDefaultResourcePath());
		//0 = empty
		for (int i = 1; i < (int)J_DEFAULT_SHAPE::COUNT; ++i)
		{
			const J_DEFAULT_SHAPE shapeType = (J_DEFAULT_SHAPE)i;
			const J_MESHGEOMETRY_TYPE meshType = JDefaultShape::GetMeshType(shapeType);
			const bool isUse = JDefaultShape::IsDefaultUse(shapeType);
			const bool isExternalFile = JDefaultShape::IsExternalFile(shapeType);

			//Contain format
			//default inner shape hasn't format		ex) cube
			//default external shape has format		ex) arrow.fbx	
			const std::wstring meshName = JDefaultShape::ConvertToName(shapeType);

			if (isExternalFile)
			{
				//basically external file is stored Engine DefaultResource folder
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFileName(meshName, name, format);

				JFile* file = projectDefualDir->GetFile(name);
				if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::MESH)
				{
					if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
						resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr<JStaticMeshGeometry>(file->GetResource()), isUse);
					else if (meshType == J_MESHGEOMETRY_TYPE::SKINNED)
						resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr<JSkinnedMeshGeometry>(file->GetResource()), isUse);
					else
						assert("MeshType Error");
				}
				else
				{
					const std::wstring srcPath = engineDefaultDir->GetPath() + L"\\" + meshName;
					const std::wstring destPath = projectDefualDir->GetPath() + L"\\" + meshName;
					Core::J_FILE_IO_RESULT copyRes = JFileIOHelper::CopyFile(srcPath, destPath);
					if (copyRes != Core::J_FILE_IO_RESULT::SUCCESS)
						assert("Copy File Error");

					const J_OBJECT_FLAG objFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED |
						OBJECT_FLAG_UNEDITABLE |
						OBJECT_FLAG_UNDESTROYABLE |
						OBJECT_FLAG_UNCOPYABLE |
						OBJECT_FLAG_HIDDEN);
					Core::JFileImportHelpData pathData{ projectDefualDir->GetPath() + L"\\" + meshName, objFlag };
					std::vector<JResourceObject*> result = JResourceImporter::Instance().ImportResource(projectDefualDir, pathData);
					resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr(result[0]), isUse);
				}
			}
			else
			{
				JFile* file = projectDefualDir->GetFile(meshName);
				if (file != nullptr && file->GetResource()->GetResourceType() == J_RESOURCE_TYPE::MESH)
				{
					if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
						resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr<JStaticMeshGeometry>(file->GetResource()), isUse);
					else if (meshType == J_MESHGEOMETRY_TYPE::SKINNED)
						resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr<JSkinnedMeshGeometry>(file->GetResource()), isUse);
					else
						assert("MeshType Error");
				}
				else
				{
					if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
					{
						JStaticMeshData staticMeshData = callableVec.find(shapeType)->second(nullptr, geoGen);
						staticMeshData.CreateBoundingObject();

						Core::JOwnerPtr<JStaticMeshGroup> group = Core::JPtrUtil::MakeOwnerPtr< JStaticMeshGroup>();
						group->AddMeshData(std::move(staticMeshData));

						const size_t guid = Core::MakeGuid();
						J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);

						if (i >= JDefaultShape::debugTypeSt)
							flag = (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN);

						JStaticMeshGeometry* newMesh = JRFI<JStaticMeshGeometry>::Create(Core::JPtrUtil::MakeOwnerPtr<JMeshGeometry::JMeshInitData>
							(meshName, guid, flag, projectDefualDir, JRI::CallFormatIndex(J_RESOURCE_TYPE::MESH, L".mesh"), std::move(group)));
						ThrowIfFailedN(newMesh != nullptr);
						resourceData->RegisterDefaultResource(shapeType, Core::GetUserPtr(newMesh), isUse);
					}
					else if (meshType == J_MESHGEOMETRY_TYPE::SKINNED)
					{
						//추가필요
						assert("Load default skinned mesh is not updated");
					}
					else
						assert("MeshType Error");
				}
			}
		}
	}
	JResourceManagerImpl::JResourceManagerImpl()
		:JEventManager([](const size_t& a, const size_t& b) {return a == b; })
	{
		resourceData = std::make_unique<JResourceData>();
		resourceIO = std::make_unique<JResourceIO>();
	}
	JResourceManagerImpl::~JResourceManagerImpl()
	{

	}
}
 