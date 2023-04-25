#include"JResourceManager.h" 
#include"JResourceManagerPrivate.h"
#include"JResourceObjectPrivate.h" 
#include"JResourceObjectIO.h" 
#include"JResourceObjectDefualtData.h"
#include"JResourceObjectImporter.h"
#include"JResourceObjectHint.h"

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
#include"../Directory/JDirectoryPrivate.h"  

#include"../../Core/Identity/JIdenCreator.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Core/Guid/GuidCreator.h" 
#include"../../Core/Func/Functor/JFunctor.h"

#include"../../Utility/JCommonUtility.h"
#include"../../Application/JApplicationEngine.h"
#include"../../Application/JApplicationProject.h"
//#include"../../Core/Geometry/JDirectXCollisionEx.h"

#include"../../Graphic/JGraphic.h"
//#include"../../Graphic/JGraphicDrawList.h" 

#include"../JModifiedObjectInfo.h"
#include"../../Editor/Interface/JEditorObjectHandleInterface.h"

//Debug
//#include"../../Core/Memory/JMemoryCapture.h"
//#include"../../Core/Unit/JByteUnit.h"
using namespace DirectX;
namespace JinEngine
{
	namespace
	{
		class JModifiedObjectInfoReader : public JEditorModifedObjectInterface
		{
			using ModVector = JModifiedObjectInfoVector::ObjectVector;
		public:
			ModVector& GetVector()
			{
				return GetModifiedObjectInfoVec();
			}
		}; 
	}
	class JResourceManager::JResourceManagerImpl final
	{
	public:
		JResourceManager* thisManager;
	public:
		Core::JUserPtr<JDirectory> engineRootDir = nullptr;
		Core::JUserPtr<JDirectory> projectRootDir = nullptr;
		std::unique_ptr<JResourceObjectDefualtData> defaultData = nullptr;
		std::unique_ptr<JResourceObjectIO> resourceIO = nullptr;
	public:
		JResourceManagerImpl(JResourceManager* thisManager)
			:thisManager(thisManager)
		{
			defaultData = std::make_unique<JResourceObjectDefualtData>();
			resourceIO = std::make_unique<JResourceObjectIO>();
		}
		~JResourceManagerImpl()
		{}
	public:
		void Initialize() {}
		void Terminate()
		{
			//StoreProjectResource();
			auto rHintVec = RTypeCommonCall::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY_REVERSE);
			for (uint i = 0; i < rHintVec.size(); ++i)
			{
				auto& typeInfo = RTypeCommonCall::CallGetTypeInfo(rHintVec[i].thisType);
				auto ptrVec = typeInfo.GetInstanceRawPtrVec();
				for (uint j = 0; j < ptrVec.size(); ++j)
					JResourceObjectPrivate::DestroyInstanceInterfaceEx::BeginForcedDestroy(static_cast<JResourceObject*>(ptrVec[j]));
			}
			defaultData->Clear();
			if (engineRootDir != nullptr)
			{
				JDirectoryPrivate::DestroyInstanceInterfaceEx::BeginForcedDestroy(engineRootDir.Get());
				engineRootDir = nullptr;
			}
			if (projectRootDir != nullptr)
			{
				JDirectoryPrivate::DestroyInstanceInterfaceEx::BeginForcedDestroy(projectRootDir.Get()); 
				projectRootDir = nullptr;
			}
			Core::JReflectionInfo::Instance().SearchInstance();
		}
		void StoreProjectResource()
		{
			auto modInfo = JModifiedObjectInfoReader{}.GetVector();
			for (const auto& data : modInfo)
			{
				if (data->isModified && data->isStore)
				{
					Core::JIdentifier* obj = Core::GetRawPtr(data->typeGuid, data->objectGuid);
					if (obj != nullptr && obj->GetTypeInfo().IsChildOf<JResourceObject>())
					{
						StoreResource(static_cast<JResourceObject*>(obj));
						data->isModified = false;
					}
					else
					{
						//cache file to asset file
						JFile* file = JDirectory::SearchFile(data->objectGuid);
						if (file != nullptr)
						{
							if (!RTypeCommonCall::GetRTypeHint(file->GetResourceType()).isFixedAssetFile)
								JFileIOHelper::DevideFile(file->GetCacheFilePath(), std::vector<std::wstring>{file->GetMetaFilePath(), file->GetPath()});
							else
								JFileIOHelper::CopyFile(file->GetCacheFilePath(), file->GetMetaFilePath());
							_wremove(file->GetCacheFilePath().c_str());
						}
					}
				}
				data->isModified = false;
			}
		}
		void LoadSelectorResource()
		{
			defaultData->Initialize();
			J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_DO_NOT_SAVE);
			engineRootDir = JICI::CreateRetUser<JDirectory>(JApplicationEngine::RootPath(), Core::MakeGuid(), rootFlag, nullptr);
			
			resourceIO->LoadEngineDirectory(engineRootDir.Get());
			resourceIO->LoadEngineResource(engineRootDir.Get());
			CreateDefaultTexture(defaultData->selectorTextureType);
		}
		void LoadProjectResource()
		{
			defaultData->Initialize();
			J_OBJECT_FLAG rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_DO_NOT_SAVE);
			engineRootDir = JICI::CreateRetUser<JDirectory>(JApplicationEngine::RootPath(), Core::MakeGuid(), rootFlag, nullptr);
			resourceIO->LoadEngineDirectory(engineRootDir.Get());
			resourceIO->LoadEngineResource(engineRootDir.Get());

			rootFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_DO_NOT_SAVE);
			projectRootDir = Core::GetUserPtr(resourceIO->LoadRootDirectory(JApplicationProject::RootPath(), rootFlag));
			
			JDirectoryPrivate::ActivationInterface::OpenDirectory(projectRootDir.Get());		 
			resourceIO->LoadProjectDirectory(projectRootDir.Get());
			resourceIO->LoadProjectResource(projectRootDir.Get());

			CreateDefaultShader();
			CreateDefaultTexture(defaultData->projectTextureType);
			CreateDefaultMaterial();
			CreateDefaultMesh();

			if (_JSceneManager::Instance().GetActivatedSceneCount() == 0)
			{
				JDirectory* projectContentsDir = thisManager->GetDirectory(JApplicationProject::ContentsPath());
				JDirectory* defaultSceneDir = projectContentsDir->GetChildDirctoryByPath(JApplicationProject::ContentScenePath());
				JICI::Create<JScene>(JScene::GetDefaultFormatIndex(), defaultSceneDir, J_SCENE_USE_CASE_TYPE::MAIN);
			} 
		}
	public:
		void CreateDefaultTexture(const std::vector<J_DEFAULT_TEXTURE>& textureType)
		{
			//수정필요
			uint handleIncrement = 0;// JGraphic::Instance(DeviceInterface()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			uint textureCount = (uint)textureType.size();
			JDirectory* textureDir = thisManager->GetDirectory(JApplicationEngine::DefaultResourcePath());

			//1 is imgui preserved 
			for (uint i = 0; i < textureCount; ++i)
			{
				const J_OBJECT_FLAG objFlag = JDefaultTexture::GetFlag(textureType[i]);
				const bool isUse = JDefaultTexture::IsDefaultUse(textureType[i]);
				std::wstring foldernam;
				std::wstring name;
				std::wstring format;
				JCUtil::DecomposeFileName(JDefaultTexture::GetName(textureType[i]), name, format);
				JFile* file = textureDir->SearchFile(name + format);

				if (file != nullptr)
					defaultData->RegisterDefaultResource(textureType[i], file, isUse);
				else
				{
					const std::wstring oriPath = Core::JFileConstant::MakeFilePath(textureDir->GetPath(), JDefaultTexture::GetName(textureType[i]));
					const size_t guid = Core::MakeGuid();
					Graphic::J_GRAPHIC_RESOURCE_TYPE gTextureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
					if (textureType[i] == J_DEFAULT_TEXTURE::DEFAULT_SKY)
						gTextureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
					
					JTexture* newTexture = JICI::Create<JTexture>(name, guid, objFlag, JResourceObject::GetFormatIndex<JTexture>(format),textureDir, oriPath, gTextureType);
					ThrowIfFailedN(newTexture != nullptr);
					defaultData->RegisterDefaultResource(textureType[i], textureDir->SearchFile(guid), isUse);
					StoreResource(newTexture);
				}
			}
		}
		void CreateDefaultShader()
		{
			JDirectory* shaderDir = thisManager->GetDirectory(JApplicationProject::ShaderMetafilePath());
			const std::wstring format = JResourceObject::GetDefaultFormat<JShader>();
			for (uint i = 0; i < (int)J_DEFAULT_GRAPHIC_SHADER::COUNTER; ++i)
			{
				const J_DEFAULT_GRAPHIC_SHADER type = (J_DEFAULT_GRAPHIC_SHADER)i;
				const J_GRAPHIC_SHADER_FUNCTION shaderF = JDefaultShader::GetShaderFunction(type);
				const JShaderGraphicPsoCondition psoCondition = JDefaultShader::GetShaderGraphicPso(type);
				const J_OBJECT_FLAG objF = JDefaultShader::GetObjectFlag(type);
				//use all default shader
				const bool isUse = JDefaultShader::IsDefaultUse(type);
				std::wstring shaderName = JShaderType::ConvertToName(shaderF, psoCondition.UniqueID());
				JFile* file = shaderDir->GetDirectoryFile(shaderName + format);

				if (file != nullptr)
					defaultData->RegisterDefaultResource(type, file, isUse);
				else
				{
					JShader* newShader = JICI::Create<JShader>(objF, shaderF, psoCondition);
					ThrowIfFailedN(newShader != nullptr);
					defaultData->RegisterDefaultResource(type, shaderDir->GetDirectoryFile(newShader->GetName() + format), isUse);
				}
			}

			for (uint i = 0; i < (int)J_DEFAULT_COMPUTE_SHADER::COUNTER; ++i)
			{
				const J_DEFAULT_COMPUTE_SHADER type = (J_DEFAULT_COMPUTE_SHADER)i;
				const J_COMPUTE_SHADER_FUNCTION shaderF = JDefaultShader::GetComputeShaderFunction(type);
				const J_OBJECT_FLAG objF = JDefaultShader::GetObjectFlag(type);

				const bool isUse = JDefaultShader::IsDefaultUse(type);
				std::wstring shaderName = JShaderType::ConvertToName(shaderF);
				JFile* file = shaderDir->GetDirectoryFile(shaderName + format);
				if (file != nullptr)
					defaultData->RegisterDefaultResource(type, file, isUse);
				else
				{
					JShader* newShader = JICI::Create<JShader>(objF, SHADER_FUNCTION_NONE, JShaderGraphicPsoCondition(), shaderF);
					ThrowIfFailedN(newShader != nullptr);
					defaultData->RegisterDefaultResource(type, shaderDir->GetDirectoryFile(newShader->GetName() + format), isUse);
				}
			}
		}
		void CreateDefaultMaterial()
		{
			auto debugLam = [](JDirectory* ownerDir,
				const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const XMFLOAT4& color,
				const bool isLine)
			{	 
				JMaterial* newMaterial = JICI::Create<JMaterial>(name, guid,
					Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), JMaterial::GetDefaultFormatIndex(), ownerDir);
				JDefaultMaterialSetting::SetDebug(newMaterial, isLine, color); 
				return newMaterial;
			};

			JDirectory* matDir = thisManager->GetDirectory(JApplicationProject::DefaultResourcePath());
			const std::wstring format = JResourceObject::GetDefaultFormat<JMaterial>();
			for (uint i = 0; i < (int)J_DEFAULT_MATERIAL::COUNTER; ++i)
			{
				//BasicMaterial format is all .mat( default format)
				const J_DEFAULT_MATERIAL type = (J_DEFAULT_MATERIAL)i;
				const J_OBJECT_FLAG flag = JDefaultMateiral::GetFlag(type);
				const bool isUse = JDefaultMateiral::IsDefaultUse(type);

				const std::wstring name = JDefaultMateiral::ConvertToName(type);
				JFile* file = matDir->SearchFile(name + format);
				if (file != nullptr)
					defaultData->RegisterDefaultResource(type, file, isUse);
				else
				{
					JMaterial* newMaterial = nullptr;
					size_t guid = Core::MakeGuid();
					switch (type)
					{
					case J_DEFAULT_MATERIAL::DEFAULT_STANDARD:
					{
						newMaterial = JICI::Create<JMaterial>(name, guid, flag, JMaterial::GetDefaultFormatIndex(), matDir);
						JDefaultMaterialSetting::SetStandard(newMaterial); 
						break;
					}
					case J_DEFAULT_MATERIAL::DEFAULT_SKY:
					{
						newMaterial = JICI::Create<JMaterial>(name, guid, flag, JMaterial::GetDefaultFormatIndex(), matDir);
						JDefaultMaterialSetting::SetSky(newMaterial, thisManager->GetDefaultTexture(J_DEFAULT_TEXTURE::DEFAULT_SKY));
						break;
					}
					case J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
					{
						newMaterial = JICI::Create<JMaterial>(name, guid, 
							Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), JMaterial::GetDefaultFormatIndex(), matDir);
						JDefaultMaterialSetting::SetShadowMap(newMaterial); 
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
						newMaterial = JICI::Create<JMaterial>(name, guid,
							Core::AddSQValueEnum(flag, OBJECT_FLAG_HIDDEN), JMaterial::GetDefaultFormatIndex(), matDir);
						newMaterial->SetBoundingObjectDepthTest(true); 
						break;
					}
					default:
						break;
					}

					ThrowIfFailedN(newMaterial != nullptr);
					defaultData->RegisterDefaultResource(type, matDir->SearchFile(guid), isUse);
					StoreResource(newMaterial); 
				}
			}
		}
		void CreateDefaultMesh()
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
			JDirectory* projectDefualDir = thisManager->GetDirectory(JApplicationProject::DefaultResourcePath());
			JDirectory* engineDefaultDir = thisManager->GetDirectory(JApplicationEngine::DefaultResourcePath());
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

					JFile* file = projectDefualDir->SearchFile(name + format);
					if (file != nullptr)
					{
						if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
							defaultData->RegisterDefaultResource(shapeType, file, isUse);
						else if (meshType == J_MESHGEOMETRY_TYPE::SKINNED)
							defaultData->RegisterDefaultResource(shapeType, file, isUse);
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
						std::vector<JResourceObject*> result = JResourceObjectImporter::Instance().ImportResource(projectDefualDir, pathData);
						for (const auto& data : result)
						{
							if (data->GetResourceType() == J_RESOURCE_TYPE::MESH)
							{
								defaultData->RegisterDefaultResource(shapeType, JDirectory::SearchFile(data->GetGuid()), isUse);
								break;
							}
						}
					}
				}
				else
				{
					JFile* file = projectDefualDir->SearchFile(meshName + JResourceObject::GetDefaultFormat<JMeshGeometry>());
					if (file != nullptr)
					{
						if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
							defaultData->RegisterDefaultResource(shapeType, file, isUse);
						else if (meshType == J_MESHGEOMETRY_TYPE::SKINNED)
							defaultData->RegisterDefaultResource(shapeType, file, isUse);
						else
							assert("MeshType Error");
					}
					else
					{
						if (meshType == J_MESHGEOMETRY_TYPE::STATIC)
						{
							JStaticMeshData staticMeshData = callableVec.find(shapeType)->second(nullptr, geoGen);
							staticMeshData.CreateBoundingObject();

							std::unique_ptr<JStaticMeshGroup> group = std::make_unique<JStaticMeshGroup>();
							group->AddMeshData(std::move(staticMeshData));

							const size_t guid = Core::MakeGuid();
							J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);

							if (i >= JDefaultShape::debugTypeSt)
								flag = (J_OBJECT_FLAG)(flag | OBJECT_FLAG_HIDDEN);

							JStaticMeshGeometry* newMesh = JICI::Create<JStaticMeshGeometry>(meshName, guid, flag, 
								JMeshGeometry::GetDefaultFormatIndex(), projectDefualDir, std::move(group));
									 
							ThrowIfFailedN(newMesh != nullptr);
							defaultData->RegisterDefaultResource(shapeType, projectDefualDir->SearchFile(guid), isUse);
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
	public:
		void StoreResource(JResourceObject* rObj)
		{
			auto storeData = JResourceObjectPrivate::AssetDataIOInterface::CreateStoreAssetDIDate(rObj);
			auto& rPrivate = static_cast<JResourceObjectPrivate&>(rObj->GetPrivateInterface());
			rPrivate.GetAssetDataIOInterface().StoreAssetData(storeData.get());
		}
	};
 
	//convert raw to user
	Core::JUserPtr<JMeshGeometry> JResourceManager::GetDefaultMeshGeometry(const J_DEFAULT_SHAPE type)const noexcept
	{ 
		return impl->defaultData->GetDefaultResource<JMeshGeometry>(type);
	}
	Core::JUserPtr<JMaterial> JResourceManager::GetDefaultMaterial(const J_DEFAULT_MATERIAL type)const noexcept
	{
		return impl->defaultData->GetDefaultResource<JMaterial>(type);
	}
	Core::JUserPtr<JTexture> JResourceManager::GetDefaultTexture(const J_DEFAULT_TEXTURE type)const noexcept
	{
		return impl->defaultData->GetDefaultResource<JTexture>(type);
	}
	Core::JUserPtr<JShader> JResourceManager::GetDefaultShader(const J_DEFAULT_GRAPHIC_SHADER type)const noexcept
	{
		return impl->defaultData->GetDefaultResource<JShader>(type);
	}
	Core::JUserPtr<JShader> JResourceManager::GetDefaultShader(const J_DEFAULT_COMPUTE_SHADER type)const noexcept
	{ 
		return impl->defaultData->GetDefaultResource<JShader>(type);
	}
	JDirectory* JResourceManager::GetDirectory(const size_t guid)const noexcept
	{
		return JDirectory::StaticTypeInfo().GetInstanceRawPtr<JDirectory>(guid);
	}
	JDirectory* JResourceManager::GetDirectory(const std::wstring& path)const noexcept
	{
		bool(*ptr)(JDirectory*, const std::wstring&) = [](JDirectory* dir, const std::wstring& path){return dir->GetPath() == path;};
		return GetDirectoryByCondition<JDirectory, const std::wstring&>(ptr, path);	 
	}
	JDirectory* JResourceManager::GetEditorResourceDirectory()const noexcept
	{
		return GetDirectory(JApplicationProject::EditorSettingPath());
	}
	JDirectory* JResourceManager::GetActivatedDirectory()const noexcept
	{
		bool(*ptr)(JDirectory*) = [](JDirectory* dir) {return dir->IsActivated(); };
		return GetDirectoryByCondition(ptr);
	}
	uint JResourceManager::GetResourceCount(const Core::JTypeInfo& info)const noexcept
	{
		return info.GetInstanceCount();
	}
	JResourceObject* JResourceManager::GetResource(const Core::JTypeInfo& info, const size_t guid)const noexcept
	{
		return static_cast<JResourceObject*>(info.GetInstanceRawPtr(guid));
	}
	JResourceObject* JResourceManager::GetResourceByPath(const Core::JTypeInfo& info, const std::wstring& path)const  noexcept
	{
		bool(*ptr)(JResourceObject*, const std::wstring&) = [](JResourceObject* rObj, const std::wstring& path) {return rObj->GetPath() == path; };
		return FineResource<JResourceObject>(info, ptr, path);
	}
	Core::JUserPtr<JResourceObject> JResourceManager::TryGetResourceUser(const Core::JTypeInfo& info, const size_t guid)noexcept
	{ 
		auto userPtr = Core::GetUserPtr<JResourceObject>(info.TypeGuid(), guid);
		if (userPtr != nullptr)
			return userPtr;
		else
			return JDirectory::SearchFile(guid)->TryGetResourceUser();
	} 

	JResourceManager::JResourceManager()
		:impl(std::make_unique<JResourceManagerImpl>(this))
	{}
	JResourceManager::~JResourceManager()
	{}

	using ApplicationAccess = JResourceManagerPrivate::ApplicationAccess;

	void ApplicationAccess::Initialize()
	{
		_JResourceManager::Instance().impl->Initialize();
	}
	void ApplicationAccess::Terminate()
	{
		_JResourceManager::Instance().impl->Terminate();
	}
	void ApplicationAccess::StoreProjectResource()
	{
		_JResourceManager::Instance().impl->StoreProjectResource();
	}
	void ApplicationAccess::LoadSelectorResource()
	{
		_JResourceManager::Instance().impl->LoadSelectorResource();
	}
	void ApplicationAccess::LoadProjectResource()
	{
		_JResourceManager::Instance().impl->LoadProjectResource();
	}

}
