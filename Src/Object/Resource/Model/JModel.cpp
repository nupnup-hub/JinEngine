#include"JModel.h"
#include"JModelAttribute.h" 
#include"JModelPart.h"
#include"../JResourceManager.h" 
#include"../Mesh/JMeshGeometry.h" 
#include"../Material/JMaterial.h" 
#include"../Skeleton/JSkeleton.h"
#include"../Skeleton/JSkeletonAsset.h"
#include"../Scene/JScene.h"  
#include"../Texture/JTexture.h"
#include"../JResourceObjectFactory.h" 
#include"../../GameObject/JGameObject.h"
#include"../../GameObject/JGameObjectFactory.h"
#include"../../Component/RenderItem/JRenderItem.h"  
#include"../../Component/JComponentFactory.h"
#include"../../Component/JComponentFactoryUtility.h"
#include"../../Directory/JDirectory.h"
#include"../../Directory/JDirectoryFactory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Utility/JCommonUtility.h" 
#include"../../../Core/Loader/FbxLoader/JFbxFileLoader.h"
#include"../../../Core/Loader/ObjLoader/JObjFileLoader.h"
#include"../../../Application/JApplicationVariable.h"

namespace JinEngine
{
	JSkeletonAsset* JModel::GetSkeletonAsset()noexcept
	{
		return skeletonAsset != nullptr ? skeletonAsset : nullptr;
	}
	const uint JModel::GetTotalMeshCount()const noexcept
	{
		return (uint)meshPartCash.size();
	}
	const uint JModel::GetTotalVertexCount()const noexcept
	{
		return modelAttribute->totalVertex;
	}
	const uint JModel::GetTotalIndexCount()const noexcept
	{
		return modelAttribute->totalIndex;
	}
	DirectX::XMFLOAT3 JModel::GetModelCenter()const noexcept
	{
		return modelAttribute->modelBSphere.Center;
	}
	DirectX::XMFLOAT3 JModel::GetSkeletonCenter()const noexcept
	{
		return modelAttribute->skletonBSphere.Center;
	}
	float JModel::GetModelRadius()const noexcept
	{
		return modelAttribute->modelBSphere.Radius;
	}
	float JModel::GetSkeletonRadius()const noexcept
	{
		return modelAttribute->skletonBSphere.Radius;
	}
	J_RESOURCE_TYPE JModel::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::string JModel::GetFormat()const noexcept
	{
		return GetAvailableFormat()[formatIndex];
	}
	std::vector<std::string> JModel::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".model", ".obj", ".fbx", };
		return format;
	}
	JModelSceneInterface* JModel::ModelSceneInterface()
	{
		return this;
	}
	JScene* JModel::GetModelScene()noexcept
	{
		return modelScene;
	}
	JGameObject* JModel::GetModelRoot()noexcept
	{
		return modelRoot;
	}
	JGameObject* JModel::GetSkeletonRoot()noexcept
	{
		return skeletonRoot;
	}
	void JModel::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
	}
	void JModel::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate(); 
		ClearResource();
	}
	void JModel::StuffResource()
	{
		if (!JValidInterface::IsValid())
		{
			switch (formatIndex)
			{
			case 0:
			{
				//.mesh
			}
			case 1:
			{
				//.obj
				if (ReadObjModelData())
					SetValid(true);
				break;
			}
			case 2:
			{
				//.fbx
				if (ReadFbxModelData())
					SetValid(true);
				break;
			}
			default:
				break;
			}
		}
	}
	void JModel::ClearResource()
	{
		if (JValidInterface::IsValid())
		{
			const bool preIgnore = IsIgnoreUndestroyableFlag();
			if (!preIgnore)
				SetIgnoreUndestroyableFlag(true);
			modelScene->BeginDestroy();
			if (!preIgnore)
				SetIgnoreUndestroyableFlag(false);

			modelScene = nullptr;
			modelRoot = nullptr;
			skeletonRoot = nullptr;
			meshPartCash.clear();
			OffResourceReference(*skeletonAsset);
			skeletonAsset = nullptr;
			SetValid(false);
		}
	}
	bool JModel::IsValidResource()const noexcept
	{
		return JValidInterface::IsValid() && (skeletonAsset != nullptr);
	}
	bool JModel::ReadObjModelData()
	{
		const JResourcePathData pathData{ GetWPath() };
		std::wifstream stream;
		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		ModelMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, pathData.folderPath, metadata);
		stream.close();

		std::vector<Core::JObjMeshPartData>objMeshData;
		std::vector<Core::JObjMatData> objMatData;
		JModelAttribute modelAttribute;

		if (JObjFileLoader::Instance().LoadObjFile(pathData.wstrPath, objMeshData, objMatData, modelAttribute))
		{	 
			const J_OBJECT_FLAG objRFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);

			using SetTexture = void(JMaterial::*)(JTexture*);
			SetTexture setTexturePtr;

			const uint objMatCount = (uint)objMatData.size();
			std::vector<JMaterial*> newMatV(objMatCount);
			for (uint i = 0; i < objMatCount; ++i)
			{
				const size_t matGuid = loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS && metadata.partGuidVector.size() > i ?
					metadata.partGuidVector[i].matGuid : Core::MakeGuid();

				JMaterial* newMat = JRFI<JMaterial>::Create(objMatData[i].name, matGuid, objRFlag, *GetDirectory(),
					JResourceObject::GetInvalidFormatIndex());

				if (newMat == nullptr)
					continue;

				std::string texturePath;
				if (((int)objMatData[i].flag & (int)Core::JOBJ_MATERIAL_FLAG::HAS_ALBEDO_T) > 0)
				{
					texturePath = pathData.folderPath + "\\" + objMatData[i].albedoTName;
					setTexturePtr = &JMaterial::SetAlbedoMap;
				}
				else if (((int)objMatData[i].flag & (int)Core::JOBJ_MATERIAL_FLAG::HAS_NORMAL_T) > 0)
				{
					texturePath = pathData.folderPath + "\\" + objMatData[i].normalTName;
					setTexturePtr = &JMaterial::SetNormalMap;
				}
				else if (((int)objMatData[i].flag & (int)Core::JOBJ_MATERIAL_FLAG::HAS_HEIGHT_T) > 0)
				{
					texturePath = pathData.folderPath + "\\" + objMatData[i].heightTName;
					setTexturePtr = &JMaterial::SetHeightMap;
				}
				else if (((int)objMatData[i].flag & (int)Core::JOBJ_MATERIAL_FLAG::HAS_AMBIENT_T) > 0)
				{
					texturePath = pathData.folderPath + "\\" + objMatData[i].ambientTName;
					setTexturePtr = &JMaterial::SetAmbientOcclusionMap;
				}
				else
					continue;

				JTexture* matTexture = JResourceManager::Instance().GetResourceByPath<JTexture>(texturePath);
				if (matTexture == nullptr)
					continue;

				(newMat->*setTexturePtr)(matTexture);
				newMat->SetAlbedoColor(objMatData[i].albedo);
				newMatV.push_back(newMat);
			}

			const uint newMatVCount = (uint)newMatV.size();
			const uint objMeshCount = (uint)objMeshData.size();
			std::vector<JMeshGeometry*> newMeshVec;
			std::vector<int> matIndex;
			for (uint i = 0; i < objMeshCount; ++i)
			{
				const size_t meshGuid = loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS && metadata.partGuidVector.size() > i ?
					metadata.partGuidVector[i].meshGuid : Core::MakeGuid();

				JMeshGeometry* newMesh = JRFI<JMeshGeometry>::Create(objMeshData[i].meshName, meshGuid, objRFlag, *GetDirectory(), 0);
				if (newMesh == nullptr)
					continue;

				JMeshInterface* iMesh = newMesh;
				iMesh->StuffStaticMesh(objMeshData[i].staticMeshData, objMeshData[i].boundingBox, objMeshData[i].boundingSphere);
				newMeshVec.push_back(newMesh);
				matIndex.push_back(-1);
				for (uint j = 0; j < newMatVCount; ++j)
				{
					if (objMeshData[i].meshName == newMatV[i]->GetName())
						matIndex[i] = j;
				}
			}

			JModel::modelAttribute = std::make_unique<JModelAttribute>(modelAttribute);
			modelScene = JRFI<JScene>::Create(GetDirectory()->MakeUniqueFileName(GetDefaultName<JScene>()),
				Core::MakeGuid(),
				(J_OBJECT_FLAG)(OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_UNEDITABLE),
				*GetDirectory(),
				GetInvalidFormatIndex());

			OnResourceReference(*modelScene);

			const uint modelPartCount = (uint)newMeshVec.size();
			JGameObject* modelPartRoot = JGFI::Create(GetName(), Core::MakeGuid(), OBJECT_FLAG_NONE, *modelRoot);

			std::vector<JGameObject*> meshGameObj;
			meshGameObj.reserve(modelPartCount + 1);
			meshGameObj.push_back(modelPartRoot);

			for (uint i = 0; i < modelPartCount; ++i)
			{
				const std::string meshName = newMeshVec[i]->GetName();
				JGameObject* child = JGFI::Create(meshName, Core::MakeGuid(), OBJECT_FLAG_NONE, *modelRoot);
				if (matIndex[i] == -1)
					JCFU::CreateRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, *child, newMeshVec[i], nullptr);
				else
					JCFU::CreateRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, *child, newMeshVec[i], newMatV[matIndex[i]]);

				meshPartCash.push_back(child);
				meshGameObj.push_back(child);
			}

			modelRoot = modelPartRoot;
			OffResourceReference(*modelScene);
			return true;
		}
		else
			return false;
	}
	bool JModel::ReadFbxModelData()
	{
		std::wifstream stream;
		stream.open(ConvertMetafilePath(GetWPath()), std::ios::in | std::ios::binary);
		ModelMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, GetFolderPath(), metadata);
		stream.close();

		std::vector<Core::JFbxPartMeshData> jFbxPartMeshData;
		std::vector<JModelPart> modelPart;
		JModelAttribute modelAttribute;
		std::vector<Joint> joint;

		Core::J_FBXRESULT fbxLoadRes = Core::JFbxFileLoader::Instance().LoadFbxModelFile(GetPath(), jFbxPartMeshData, modelAttribute, joint);
		if (((int)fbxLoadRes & (int)Core::J_FBXRESULT::HAS_MESH) > 0)
		{
			const uint meshCount = (uint)jFbxPartMeshData.size();
			modelPart.reserve(jFbxPartMeshData.size());

			const J_OBJECT_FLAG meshflag = (J_OBJECT_FLAG)(OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_UNEDITABLE);
			const J_OBJECT_FLAG matflag = (J_OBJECT_FLAG)(OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_UNEDITABLE);

			for (uint i = 0; i < meshCount; ++i)
			{
				if (!jFbxPartMeshData[i].hasMesh)
					continue;

				size_t meshGuid = loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS ? metadata.partGuidVector[i].meshGuid : Core::MakeGuid();
				size_t matGuid = loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS ? metadata.partGuidVector[i].matGuid : Core::MakeGuid();

				JMeshGeometry* mesh = JRFI<JMeshGeometry>::Create(jFbxPartMeshData[i].name, meshGuid, meshflag, *GetDirectory(), JResourceObject::GetInvalidFormatIndex());
				JMeshInterface* iMesh = mesh;
				if (modelAttribute.hasSkeleton)
					iMesh->StuffSkinnedMesh(jFbxPartMeshData[i].skinnedMeshData, jFbxPartMeshData[i].boundingBox, jFbxPartMeshData[i].boundingSphere);
				else
					iMesh->StuffStaticMesh(jFbxPartMeshData[i].staticMeshData, jFbxPartMeshData[i].boundingBox, jFbxPartMeshData[i].boundingSphere);

				JMaterial* mat = JRFI<JMaterial>::Create(jFbxPartMeshData[i].name + "_Material", matGuid, matflag, *GetDirectory(), JResourceObject::GetInvalidFormatIndex());
				modelPart.emplace_back(JModelPart(jFbxPartMeshData[i].name, jFbxPartMeshData[i].parentIndex, mesh, mat));
			}
			if (((int)fbxLoadRes & (int)Core::J_FBXRESULT::HAS_SKELETON) > 0)
			{
				const uint jointCount = (uint)joint.size();
				std::string totalName;
				for (uint i = 0; i < jointCount; ++i)
					totalName += joint[i].name;

				JSkeleton newSkeleton{std::move(joint), JCommonUtility::CalculateGuid(totalName) };

				uint count;
				std::vector<JResourceObject*>::const_iterator st = JResourceManager::Instance().GetResourceVectorHandle<JSkeletonAsset>(count);
				for (uint i = 0; i < count; ++i)
				{
					JSkeletonAsset* oldSkeletonAsset = static_cast<JSkeletonAsset*>(*(st + i));
					if (oldSkeletonAsset->GetSkeleton()->IsSame(newSkeleton))
					{
						skeletonAsset = oldSkeletonAsset;
						break;
					}
				}

				if (skeletonAsset == nullptr)
					skeletonAsset = JRFI<JSkeletonAsset>::Create(*GetDirectory(), std::move(newSkeleton));
				OnResourceReference(*skeletonAsset);
			}

			JModel::modelAttribute = std::make_unique<JModelAttribute>(modelAttribute);
			modelScene = JRFI<JScene>::Create(GetDirectory()->MakeUniqueFileName(GetDefaultName<JScene>()),
				Core::MakeGuid(),
				(J_OBJECT_FLAG)(OBJECT_FLAG_HIDDEN | OBJECT_FLAG_DO_NOT_SAVE | OBJECT_FLAG_UNEDITABLE),
				*GetDirectory(),
				GetInvalidFormatIndex());

			OnResourceReference(*modelScene);
			const uint modelPartCount = (uint)modelPart.size();

			JGameObject* modelPartRoot = JGFI::Create(GetName(), Core::MakeGuid(), OBJECT_FLAG_NONE, *modelRoot);

			std::vector<JGameObject*> meshGameObj;
			meshGameObj.reserve(modelPartCount + 1);
			meshGameObj.push_back(modelPartRoot);

			for (uint i = 0; i < modelPartCount; ++i)
			{
				const int parentIndex = modelPart[i].parentIndex;
				JGameObject* child = JGFI::Create(modelPart[i].name, Core::MakeGuid(), OBJECT_FLAG_NONE, *meshGameObj[parentIndex]);

				if (modelPart[i].mesh != nullptr)
				{
					JCFU::CreateRenderItem(Core::MakeGuid(), OBJECT_FLAG_NONE, *child, modelPart[i].mesh, modelPart[i].mat);
					meshPartCash.push_back(child);
				}
				meshGameObj.push_back(child);
			}

			JSkeleton* skeleton = GetSkeletonAsset()->GetSkeleton();

			if (skeleton != nullptr)
			{
				const std::string skeletonRootName = skeleton->GetJointName(0);
				JGameObject* jointRoot = JGFI::Create(skeletonRootName, Core::MakeGuid(), OBJECT_FLAG_NONE, *modelPartRoot);

				const uint8 jointCount = skeleton->GetJointCount();
				std::vector<JGameObject*> skeletonVector(jointCount);
				skeletonVector[0] = jointRoot;

				for (uint i = 1; i < jointCount; ++i)
				{
					const std::string name = skeleton->GetJointName(i);
					const uint8 parentIndex = skeleton->GetJointParentIndex(i);
					JGameObject* joint = JGFI::Create(name, Core::MakeGuid(), OBJECT_FLAG_NONE, *skeletonVector[parentIndex]);
					skeletonVector[i] = joint;
				}
				skeletonRoot = skeletonVector[0];
			}
			modelRoot = modelPartRoot;
			OffResourceReference(*modelScene);
			return true;
		}
		else
			return false;
	}
	void JModel::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			if (skeletonAsset != nullptr && skeletonAsset->GetGuid() == jRobj->GetGuid())
			{
				OffResourceReference(*skeletonAsset);
				skeletonAsset = nullptr;
			}
		}
	}
	std::vector<JGameObject*>::const_iterator JModel::GetMeshPartVectorHandle(uint& count)noexcept
	{
		count = (uint)meshPartCash.size();
		return meshPartCash.cbegin();
	}
	Core::J_FILE_IO_RESULT JModel::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JModel::StoreObject(JModel* model)
	{
		if (model == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_SEARCH_DATA;

		if (((int)model->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(model->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT storeMetaRes = StoreMetadata(stream, model);
		stream.close();

		return storeMetaRes;
	}
	Core::J_FILE_IO_RESULT JModel::StoreMetadata(std::wofstream& stream, JModel* model)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::StoreMetadata(stream, model);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			const uint meshPartCount = (uint)model->meshPartCash.size();
			stream << "MeshCount: " << meshPartCount << '\n';
			for (uint i = 0; i < meshPartCount; ++i)
			{
				stream << "MeshGuid: " << model->meshPartCash[i]->GetRenderItem()->GetMesh()->GetGuid() << '\n';
				stream << "MaterialGuid: " << model->meshPartCash[i]->GetRenderItem()->GetMaterial()->GetGuid() << '\n';
			}
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JModel* JModel::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (pathData.format == ".fbx")
		{
			using FbxFileTypeInfo = Core::JFbxFileLoaderImpl::FbxFileTypeInfo;
			FbxFileTypeInfo fileTypeInfo = Core::JFbxFileLoader::Instance().GetFileTypeInfo(pathData.strPath);
			if (((int)fileTypeInfo.typeInfo & (int)Core::J_FBXRESULT::HAS_MESH) == 0)
				return nullptr;
		}

		if (directory == nullptr)
			return nullptr;

		if (!JResourceObject::IsResourceFormat<JModel>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		ModelMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, pathData.folderPath, metadata);
		stream.close();

		JModel* newModel = nullptr;
		if (directory->HasFile(pathData.fullName))
			newModel = JResourceManager::Instance().GetResourceByPath<JModel>(pathData.strPath);

		if (newModel == nullptr)
		{
			if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
				newModel = new JModel(pathData.name, metadata.guid, metadata.flag, directory, GetFormatIndex<JModel>(pathData.format));
			else
				newModel = new JModel(pathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, directory, GetFormatIndex<JModel>(pathData.format));
		}

		if (newModel->IsValid())
			return newModel;
		else if (pathData.format == ".fbx")
		{
			if (newModel->ReadFbxModelData())
			{
				newModel->SetValid(true);
				return newModel;
			}
		}
		else if (pathData.format == ".obj")
		{
			if (newModel->ReadObjModelData())
			{
				newModel->SetValid(true);
				return newModel;
			}
		}
		 
		delete newModel;
		return nullptr;
	}
	Core::J_FILE_IO_RESULT JModel::LoadMetadata(std::wifstream& stream, const std::string& folderPath, ModelMetadata& metadata)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::LoadMetadata(stream, metadata);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			std::wstring guide;
			stream >> guide >> metadata.meshPartCount;
			metadata.partGuidVector.resize(metadata.meshPartCount);
			for (uint i = 0; i < metadata.meshPartCount; ++i)
			{
				stream >> guide >> metadata.partGuidVector[i].meshGuid;
				stream >> guide >> metadata.partGuidVector[i].matGuid;
			}
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JModel::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JModel(owner->MakeUniqueFileName(GetDefaultName<JModel>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JModel(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};

		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			JModel* oriModel = static_cast<JModel*>(ori);
			JDirectory* oriDir = oriModel->GetDirectory();
			JModel* newModel = new JModel(oriDir->MakeUniqueFileName(oriModel->GetName()),
				Core::MakeGuid(),
				oriModel->GetFlag(),
				oriDir,
				oriModel->formatIndex);

			newModel->modelScene = JRFI<JScene>::Copy(*oriModel->modelScene);
			const size_t modelRootGuid = oriModel->modelRoot->GetGuid();
			const size_t skeletonRootGuid = oriModel->skeletonRoot->GetGuid();

			const uint gameObjCount = (uint)oriModel->modelScene->GetGameObjectCount();
			for (uint i = 0; i < gameObjCount; ++i)
			{
				JGameObject* oriObj = oriModel->modelScene->GetGameObject(i);
				const size_t guid = oriObj->GetGuid();
				if (guid == modelRootGuid)
					newModel->modelRoot = newModel->modelScene->GetGameObject(i);
				else if (guid == skeletonRootGuid)
					newModel->skeletonRoot = newModel->modelScene->GetGameObject(i);

				JRenderItem* oriR = oriObj->GetRenderItem();
				if (oriR != nullptr)
				{
					JGameObject* newObj = newModel->modelScene->GetGameObject(i);
					JRenderItem* newR = JCFI<JRenderItem>::Copy(*oriR, *newObj);
					if (newR->GetMesh() != nullptr)
					{
						JMeshGeometry* oriMesh = oriR->GetMesh();
						newR->SetMeshGeometry(JRFI<JMeshGeometry>::Copy(*oriMesh));
					}
					if (newR->GetMaterial() != nullptr)
					{
						JMaterial* oriMat = oriR->GetMaterial();
						newR->SetMaterial(JRFI<JMaterial>::Copy(*oriMat));
					}
					newModel->meshPartCash.push_back(newObj);
				}
			}
			newModel->modelAttribute = std::make_unique<JModelAttribute>();
			newModel->modelAttribute->modelBBox = oriModel->modelAttribute->modelBBox;
			newModel->modelAttribute->modelBSphere = oriModel->modelAttribute->modelBSphere;
			newModel->modelAttribute->skletonBSphere = oriModel->modelAttribute->skletonBSphere;
			newModel->modelAttribute->hasSkeleton = oriModel->modelAttribute->hasSkeleton;
			newModel->modelAttribute->totalVertex = oriModel->modelAttribute->totalVertex;
			newModel->modelAttribute->totalIndex = oriModel->modelAttribute->totalIndex;
			if (oriModel->skeletonAsset != nullptr)
				newModel->skeletonAsset = JRFI<JSkeletonAsset>::Copy(*oriModel->skeletonAsset);
			return newModel;
		};

		JRFI<JModel>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JModel>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JModel::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JModel::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SKELETON}, false, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JModel::JModel(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JModelInterface(name, guid, flag, directory, formatIndex)
	{}
	JModel::~JModel() {}
}