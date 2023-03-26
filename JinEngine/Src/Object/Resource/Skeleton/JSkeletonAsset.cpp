#include"JSkeletonAsset.h" 
#include"Avatar/JAvatar.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Application/JApplicationVariable.h"
#include<fstream>
#include<DirectXMath.h>

namespace JinEngine
{
	using namespace DirectX;

	JSkeletonAsset::JSkeletonInitData::JSkeletonInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex,
		std::unique_ptr<JSkeleton> skeleton)
		:JResourceInitData(name, guid, flag, directory, formatIndex, J_RESOURCE_TYPE::SKELETON),
		skeleton(std::move(skeleton))
	{}
	JSkeletonAsset::JSkeletonInitData::JSkeletonInitData(const std::wstring& name,
		JDirectory* directory,
		const uint8 formatIndex,
		std::unique_ptr<JSkeleton> skeleton)
		: JResourceInitData(name, guid, flag, directory, formatIndex, J_RESOURCE_TYPE::SKELETON),
		skeleton(std::move(skeleton))
	{}
	JSkeletonAsset::JSkeletonInitData::JSkeletonInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const std::wstring& oriPath,
		std::unique_ptr<JSkeleton> skeleton)
		: JResourceInitData(name, guid, flag, directory, JResourceObject::GetFormatIndex<JSkeletonAsset>(JCUtil::DecomposeFileFormat(oriPath)), J_RESOURCE_TYPE::SKELETON),
		skeleton(std::move(skeleton))
	{}
	JSkeletonAsset::JSkeletonInitData::JSkeletonInitData(const std::wstring& name,
		JDirectory* directory,
		const std::wstring& oriPath,
		std::unique_ptr<JSkeleton> skeleton)
		: JResourceInitData(name, directory, JResourceObject::GetFormatIndex<JSkeletonAsset>(JCUtil::DecomposeFileFormat(oriPath)), J_RESOURCE_TYPE::SKELETON),
		skeleton(std::move(skeleton))
	{}
	bool JSkeletonAsset::JSkeletonInitData::IsValidCreateData()
	{
		if (JResourceInitData::IsValidCreateData() && skeleton != nullptr)
			return true;
		else
			return false;
	}

	J_RESOURCE_TYPE JSkeletonAsset::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JSkeletonAsset::GetFormat()const noexcept
	{
		return GetAvailableFormat()[GetFormatIndex()];
	}
	std::vector<std::wstring> JSkeletonAsset::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".skel", L".fbx" };
		return format;
	}
	JSkeleton* JSkeletonAsset::GetSkeleton()noexcept
	{
		return skeleton.get();
	}
	JAvatar* JSkeletonAsset::GetAvatar()noexcept
	{
		return avatar.get();
	}
	JSKELETON_TYPE JSkeletonAsset::GetSkeletonType()const noexcept
	{
		return skeletonType;
	}
	size_t JSkeletonAsset::GetSkeletonHash()const noexcept
	{
		return skeletonHash;
	}
	std::wstring JSkeletonAsset::GetJointName(int index)noexcept
	{
		return skeleton->GetJointName(index);
	}
	std::vector<std::vector<uint8>> JSkeletonAsset::GetSkeletonTreeIndexVec()noexcept
	{
		const uint jointCount = skeleton->GetJointCount();
		std::vector<std::vector<uint8>> treeIndex(skeleton->GetJointCount());

		//0 is root node
		//root node has invalid parentInedex
		for (uint i = 1; i < jointCount; ++i)
			treeIndex[skeleton->GetJoint(i).parentIndex].push_back(i);
		return treeIndex;
	}
	void JSkeletonAsset::SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept
	{
		JSkeletonAsset::skeletonType = skeletonType;
	}
	bool JSkeletonAsset::HasAvatar()noexcept
	{
		return avatar != nullptr;
	}
	bool JSkeletonAsset::IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)noexcept
	{ 
		if (childIndex == parentIndex || childIndex < parentIndex)
			return false;

		uint8 nowParentIndex = skeleton->GetJointParentIndex(childIndex);
		if (nowParentIndex == parentIndex)
			return true;

		bool isParent = false;
		while (nowParentIndex != JSkeletonFixedData::incorrectJointIndex)
		{
			if (nowParentIndex == parentIndex)
			{
				isParent = true;
				break;
			}
			nowParentIndex = skeleton->GetJointParentIndex(nowParentIndex);
			if (nowParentIndex < parentIndex)
				break;
		}

		return isParent;
	}
	JSkeletonAssetAvatarInterface* JSkeletonAsset::AvatarInterface()
	{
		return this;
	}
	void JSkeletonAsset::SetAvatar(JAvatar* inAvatar)noexcept
	{
		if (JSkeletonAsset::avatar == nullptr)
			avatar = std::make_unique<JAvatar>();

		const uint32 jointCount = skeleton->GetJointCount();
		if (avatar->jointInterpolation.size() == 0)
		{
			avatar->jointInterpolation.resize(jointCount);
			avatar->jointInterpolation.reserve(jointCount);
		}
		avatar->jointInterpolation[0].translation = XMFLOAT3(0, 0, 0);
		avatar->jointInterpolation[0].quaternion = XMFLOAT4(0, 0, 0, 0);

		for (uint32 index = 1; index < jointCount; ++index)
		{
			uint parentIndex = skeleton->GetJointParentIndex(index);
			XMFLOAT3 tF;
			XMFLOAT4 qF;
			XMFLOAT3 sF;
			XMVECTOR tV;
			XMVECTOR qV;
			XMVECTOR sV;

			XMMATRIX parent;
			XMMATRIX child;
			XMMATRIX parentInverse;
			XMMATRIX variance;
			parent = skeleton->GetBindPose(parentIndex);
			child = skeleton->GetBindPose(index);

			parentInverse = XMMatrixInverse(nullptr, parent);
			variance = XMMatrixMultiply(child, parentInverse);

			XMMatrixDecompose(&sV, &qV, &tV, variance);
			XMStoreFloat3(&sF, sV);
			XMStoreFloat4(&qF, qV);
			XMStoreFloat3(&tF, tV);

			avatar->jointInterpolation[index].translation = tF;
			avatar->jointInterpolation[index].quaternion = qF;
			avatar->jointInterpolation[index].isAvatarJoint = false;
		}
		for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
		{
			avatar->jointReference[i] = inAvatar->jointReference[i];
			if (avatar->jointReference[i] != JSkeletonFixedData::incorrectJointIndex)
				avatar->jointInterpolation[avatar->jointReference[i]].isAvatarJoint = true;
		}

		if (avatar->jointBackReferenceMap.size() == 0)
		{
			avatar->jointBackReferenceMap.resize(jointCount);
			avatar->jointBackReferenceMap.reserve(jointCount);
		}
		else
			avatar->jointBackReferenceMap.clear();

		for (uint i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
		{
			const uint8 jointIndex = avatar->jointReference[i];
			const uint8 parentRefIndex = avatar->jointReferenceParent[i];
			avatar->jointBackReferenceMap[jointIndex].refIndex = i;
			avatar->jointBackReferenceMap[jointIndex].allottedParentRefIndex = avatar->FindReferenceIndexEndToRoot(parentRefIndex, 0);
		}
		for (uint i = 0; i < jointCount; ++i)
		{
			if (avatar->jointBackReferenceMap[i].refIndex == JSkeletonFixedData::incorrectJointIndex &&
				avatar->jointBackReferenceMap[i].allottedParentRefIndex == JSkeletonFixedData::incorrectJointIndex)
			{
				uint8 nowIndex = skeleton->GetJointParentIndex(i);
				while (nowIndex != JSkeletonFixedData::incorrectJointIndex)
				{
					if (avatar->jointBackReferenceMap[nowIndex].refIndex != JSkeletonFixedData::incorrectJointIndex)
					{
						avatar->jointBackReferenceMap[i].allottedParentRefIndex = avatar->jointBackReferenceMap[nowIndex].refIndex;
						break;
					}
					nowIndex = skeleton->GetJointParentIndex(nowIndex);
				}
			}
		}
		/*for (uint i = 0; i < jointCount; ++i)
		{
			JDebugTextOut::PrintWstr(JCUtil::StrToWstr(GetJointName(i)));
			if (avatar->jointBackReferenceMap[i].refIndex == JSkeletonFixedData::incorrectJointIndex)
				JDebugTextOut::PrintWstr(L"refIndex: nullptr");
			else
				JDebugTextOut::PrintWstr(L"refIndex: " + JCUtil::StrToWstr(GetJointName(avatar->jointReference[avatar->jointBackReferenceMap[i].refIndex])));

			JDebugTextOut::PrintWstr(L"allParent: " + JCUtil::StrToWstr(GetJointName(avatar->jointReference[avatar->jointBackReferenceMap[i].allottedParentRefIndex])));
			JDebugTextOut::PrintEnter(1);
		}
		JDebugTextOut::CloseStream();*/
	}
	void JSkeletonAsset::CopyAvatarJointIndex(JAvatar* outAvatar)noexcept
	{
		if (outAvatar == nullptr)
			return;

		if ((uint32)outAvatar->jointReference.size() != JSkeletonFixedData::maxAvatarJointCount)
		{
			outAvatar->jointReference.clear();
			outAvatar->jointReference.resize(JSkeletonFixedData::maxAvatarJointCount);
		}

		for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
			outAvatar->jointReference[i] = avatar->jointReference[i];
	}
	void JSkeletonAsset::SetSkeleton(std::unique_ptr<JSkeleton> skeleton)
	{
		JSkeletonAsset::skeleton = std::move(skeleton);
		if(JSkeletonAsset::skeleton != nullptr)
			skeletonHash = JSkeletonAsset::skeleton->GetHash();
	}
	void JSkeletonAsset::DoCopy(JObject* ori)
	{
		JSkeletonAsset* oriS = static_cast<JSkeletonAsset*>(ori);
		CopyRFile(*oriS);
		ClearResource();
		StuffResource();
	}
	void JSkeletonAsset::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
	}
	void JSkeletonAsset::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		ClearResource();
	}
	void JSkeletonAsset::StuffResource()
	{
		if (!IsValid())
		{
			if (ReadSkeletonAssetData())
				SetValid(true);
		}
	}
	void JSkeletonAsset::ClearResource()
	{
		if (IsValid())
		{
			skeleton.reset();
			avatar.reset();
			SetValid(false);
		}
	}
	bool JSkeletonAsset::WriteSkeletonAssetData()
	{
		std::wofstream stream;
		stream.open(GetPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{		 
			JFileIOHelper::StoreAtomicData(stream, L"JointCount:", skeleton->GetJointCount());
			const uint jointCount = skeleton->GetJointCount();
			for (uint i = 0; i < skeleton->GetJointCount(); ++i)
			{
				const Joint joint = skeleton->GetJoint(i);
				JFileIOHelper::StoreJString(stream, L"Name:", joint.name);
				JFileIOHelper::StoreAtomicData(stream, L"ParentIndex:", joint.parentIndex);
				JFileIOHelper::StoreAtomicData(stream, L"Length:", joint.length);
				JFileIOHelper::StoreXMFloat4x4(stream, L"InBindPose:", joint.inbindPose);
			}
			stream.close();
			return true;
		}
		else
			return false;
	}
	bool JSkeletonAsset::ReadSkeletonAssetData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);
		if (stream.is_open())
		{ 
			uint jointCount;
			std::vector<Joint> joint;
			 
			JFileIOHelper::LoadAtomicData(stream, jointCount);
			joint.resize(jointCount);

			for (uint i = 0; i < jointCount; ++i)
			{ 
				int parentIndex;
				JFileIOHelper::LoadJString(stream, joint[i].name);
				JFileIOHelper::LoadAtomicData(stream, parentIndex);
				JFileIOHelper::LoadAtomicData(stream, joint[i].length);
				JFileIOHelper::LoadXMFloat4x4(stream, joint[i].inbindPose);
				joint[i].parentIndex = parentIndex;		  
			}			
			stream.close();
			SetSkeleton(std::make_unique<JSkeleton>(std::move(joint)));
			return true;
		}
		else
			return false;
	}
	Core::J_FILE_IO_RESULT JSkeletonAsset::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JSkeletonAsset::StoreObject(JSkeletonAsset* skeletonAsset)
	{
		if (skeletonAsset == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)skeletonAsset->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(skeletonAsset->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, skeletonAsset);
		stream.close();

		return res; 
	}
	Core::J_FILE_IO_RESULT JSkeletonAsset::StoreMetadata(std::wofstream& stream, JSkeletonAsset* skeletonAsset)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::StoreMetadata(stream, skeletonAsset);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			//아바타 정보 저장 필요		
			bool hasAvatar = skeletonAsset->GetAvatar() != nullptr;
			JFileIOHelper::StoreAtomicData(stream, L"HasAvatar:", hasAvatar);
			if (hasAvatar)
			{
				JAvatar* avatar = skeletonAsset->GetAvatar();
				for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
					JFileIOHelper::StoreAtomicData(stream, std::to_wstring(i) + L"Index:", avatar->jointReference[i]);
			}
			JFileIOHelper::StoreEnumData(stream, L"SkeletonType:", skeletonAsset->GetSkeletonType());
			JFileIOHelper::StoreAtomicData(stream, L"SkeletonHash:", skeletonAsset->GetSkeletonHash());
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JSkeletonAsset* JSkeletonAsset::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JSkeletonAssetMetaData metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JSkeletonAsset* newSkeletonAsset = nullptr;
		if (directory->HasFile(pathData.name))
			newSkeletonAsset = JResourceManager::Instance().GetResourceByPath<JSkeletonAsset>(pathData.engineFileWPath);

		if (newSkeletonAsset == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			JSkeletonInitData initdata{ pathData.name, metadata.guid, metadata.flag, directory, (uint8)metadata.formatIndex };
			if (initdata.IsValidLoadData())
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JSkeletonAsset>(initdata);
				newSkeletonAsset = ownerPtr.Get();
				if (!AddInstance(std::move(ownerPtr)))
					return nullptr;
			}
		}	
		if (newSkeletonAsset != nullptr)
		{
			if(metadata.avatar != nullptr)
				newSkeletonAsset->SetAvatar(metadata.avatar.get());
			newSkeletonAsset->SetSkeletonType((JSKELETON_TYPE)metadata.skeletonType);
			newSkeletonAsset->skeletonHash = metadata.skeletonHash;
		}
		return newSkeletonAsset;
	}
	Core::J_FILE_IO_RESULT JSkeletonAsset::LoadMetadata(std::wifstream& stream, JSkeletonAssetMetaData& metadata)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::LoadMetadata(stream, metadata);
			 
			bool hasAvatar; 
			JSKELETON_TYPE skeletonType;
			int index;
			 
			JFileIOHelper::LoadAtomicData(stream, hasAvatar);
			if (hasAvatar)
			{
				metadata.avatar = std::make_unique<JAvatar>();
				for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
				{
					JFileIOHelper::LoadAtomicData(stream, index);
					metadata.avatar->jointReference[i] = (uint8)index;
				}
			} 
			JFileIOHelper::LoadEnumData(stream, metadata.skeletonType);
			JFileIOHelper::LoadAtomicData(stream, metadata.skeletonHash);
			//SetSkeletonType((JSKELETON_TYPE)skeletonType);
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JSkeletonAsset::RegisterCallOnce()
	{
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData>initdata) ->JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::SKELETON && initdata->IsValidCreateData())
			{
				JSkeletonInitData* sInitdata = static_cast<JSkeletonInitData*>(initdata.Get());
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JSkeletonAsset>(*sInitdata);
				JSkeletonAsset* newSkeletonAsset = ownerPtr.Get();
				if (AddInstance(std::move(ownerPtr)))
				{
					newSkeletonAsset->WriteSkeletonAssetData();
					StoreObject(newSkeletonAsset);
					newSkeletonAsset->SetValid(true);
					newSkeletonAsset->ClearResource();
					return newSkeletonAsset;
				}
			}
			return nullptr;
		};
		auto loadC = [](JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			InitData initData{ ori->GetName(), Core::MakeGuid(), ori->GetFlag(), directory, GetFormatIndex<JSkeletonAsset>(ori->GetFormat())};
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JSkeletonAsset>(initData);

			JSkeletonAsset* newSkeletonAsset = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newSkeletonAsset->Copy(ori);
				return newSkeletonAsset;
			}
			else
				return nullptr;
		};

		JRFI<JSkeletonAsset>::Register(defaultC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JSkeletonAsset>(format); };

		static GetTypeInfoCallable getTypeInfoCallable{ &JSkeletonAsset::StaticTypeInfo };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JSkeletonAsset::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false};
		static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JSkeletonAsset::JSkeletonAsset(JSkeletonInitData& initdata)
		:JSkeletonAssetInterface(initdata)
	{
		SetSkeleton(std::move(initdata.skeleton)); 
	}
	JSkeletonAsset::~JSkeletonAsset() {}
}