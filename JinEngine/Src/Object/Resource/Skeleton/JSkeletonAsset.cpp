#include"JSkeletonAsset.h" 
#include"JSkeleton.h"
#include"JSkeletonAssetPrivate.h"
#include"Avatar/JAvatar.h"  
#include"../JResourceObjectHint.h"
#include"../JClearableInterface.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Application/JApplicationProject.h"
#include<fstream>
#include<DirectXMath.h>

namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		static JSkeletonAssetPrivate sPrivate;
	}
 
	class JSkeletonAsset::JSkeletonAssetImpl : public Core::JIdentifierImplBase, public JClearableInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSkeletonAssetImpl)
	public:
		JSkeletonAsset* thisSkel = nullptr;
	public: 
		std::unique_ptr<JSkeleton> skeleton = nullptr;
		std::unique_ptr<JAvatar> avatar = nullptr;
		JSKELETON_TYPE skeletonType;
		size_t skeletonHash = 0;
	public:
		JSkeletonAssetImpl(const InitData& initData, JSkeletonAsset* thisSkel)
			:thisSkel(thisSkel)
		{}
		~JSkeletonAssetImpl() {}
	public:
		void Initialize(InitData& initData)
		{
			ImportSkeleton(std::move(initData.skeleton));
		}
	public:
		std::vector<std::vector<uint8>> GetSkeletonTreeIndexVec()noexcept
		{
			const uint jointCount = skeleton->GetJointCount();
			std::vector<std::vector<uint8>> treeIndex(skeleton->GetJointCount());

			//0 is root node
			//root node has invalid parentInedex
			for (uint i = 1; i < jointCount; ++i)
				treeIndex[skeleton->GetJoint(i).parentIndex].push_back(i);
			return treeIndex;
		}
	public:
		void SetAvatar(JAvatar* inAvatar)noexcept
		{
			if (avatar == nullptr)
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
		void CopyAvatarJointIndex(_Out_ JAvatar* outAvatar)noexcept
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

	public:
		bool IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)noexcept
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
	public:
		void StuffResource()
		{
			if (!thisSkel->IsValid())
			{
				if (ImportSkeleton(ReadAssetData(thisSkel->GetPath())))
					thisSkel->SetValid(true);
			}
		}
		void ClearResource()
		{
			if (thisSkel->IsValid())
			{
				skeleton.reset();
				avatar.reset();
				thisSkel->SetValid(false);
			}
		}
	public:	 
		static std::unique_ptr<JSkeleton> ReadAssetData(const std::wstring& path)
		{
			std::wifstream stream;
			stream.open(path, std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return std::unique_ptr<JSkeleton>{};

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
			return std::make_unique<JSkeleton>(std::move(joint));
		}
		bool WriteAssetData()
		{
			std::wofstream stream;
			stream.open(thisSkel->GetPath(), std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

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
		bool ImportSkeleton(std::unique_ptr<JSkeleton>&& newSkeleton)
		{
			skeleton = std::move(newSkeleton);
			if (skeleton != nullptr)
				skeletonHash = skeleton->GetHash();
			return true;
		}
	public:
		static void RegisterCallOnce()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JSkeletonAsset::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JSkeletonAsset::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JSkeletonAsset::StaticTypeInfo(), sPrivate);
		}
	};

	JSkeletonAsset::InitData::InitData(const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JSkeleton>&& skeleton)
		:JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory), skeleton(std::move(skeleton))
	{}
	JSkeletonAsset::InitData::InitData(const size_t guid, 
		const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JSkeleton>&& skeleton)
		: JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory), skeleton(std::move(skeleton))
	{}
	JSkeletonAsset::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JSkeleton>&& skeleton)
		: JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory),
		skeleton(std::move(skeleton))
	{}
	bool JSkeletonAsset::InitData::IsValidData()const noexcept
	{
		return JResourceObject::InitData::IsValidData() && skeleton != nullptr;
	}

	JSkeletonAsset::LoadMetaData::LoadMetaData(JDirectory* directory)
		:JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JSkeletonAsset::GetPrivateInterface()const noexcept
	{
		return sPrivate;
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
		return impl->skeleton.get();
	}
	JAvatar* JSkeletonAsset::GetAvatar()noexcept
	{
		return impl->avatar.get();
	}
	JSKELETON_TYPE JSkeletonAsset::GetSkeletonType()const noexcept
	{
		return impl->skeletonType;
	}
	size_t JSkeletonAsset::GetSkeletonHash()const noexcept
	{
		return impl->skeletonHash;
	}
	std::wstring JSkeletonAsset::GetJointName(int index)noexcept
	{
		return impl->skeleton->GetJointName(index);
	}
	std::vector<std::vector<uint8>> JSkeletonAsset::GetSkeletonTreeIndexVec()noexcept
	{
		return impl->GetSkeletonTreeIndexVec();
	}
	void JSkeletonAsset::SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept
	{
		impl->skeletonType = skeletonType;
	}
	bool JSkeletonAsset::HasAvatar()noexcept
	{
		return impl->avatar != nullptr;
	}
	bool JSkeletonAsset::IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)noexcept
	{ 
		return impl->IsRegularChildJointIndex(childIndex, parentIndex);
	} 
	void JSkeletonAsset::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		impl->StuffResource();
	}
	void JSkeletonAsset::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		impl->ClearResource();
	}
	JSkeletonAsset::JSkeletonAsset(InitData& initData)
		:JResourceObject(initData), impl(std::make_unique<JSkeletonAssetImpl>(initData, this))
	{
		impl->Initialize(initData);
	}
	JSkeletonAsset::~JSkeletonAsset() {}
	 
	using CreateInstanceInterface = JSkeletonAssetPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JSkeletonAssetPrivate::AssetDataIOInterface;
	using AvatarInterface = JSkeletonAssetPrivate:: AvatarInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JSkeletonAsset>(*static_cast<JSkeletonAsset::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JSkeletonAsset::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkeletonAsset::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JSkeletonAsset::LoadData*>(data);
		auto pathData = loadData->pathData;
		JDirectory* directory = loadData->directory;
		JSkeletonAsset::LoadMetaData metadata(loadData->directory);

		if (LoadMetaData(pathData.engineMetaFileWPath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JSkeletonAsset* newSkel = nullptr;
		if (directory->HasFile(metadata.guid))
			newSkel = static_cast<JSkeletonAsset*>(Core::GetUserPtr(JSkeletonAsset::StaticTypeInfo().TypeGuid(), metadata.guid).Get());

		if (newSkel == nullptr)
		{
			std::unique_ptr<JSkeletonAsset::InitData> initData = std::make_unique<JSkeletonAsset::InitData>(pathData.name,
				metadata.guid,
				metadata.flag,
				(uint8)metadata.formatIndex,
				directory,
				JSkeletonAsset::JSkeletonAssetImpl::ReadAssetData(pathData.engineFileWPath));

			auto rawPtr = sPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &sPrivate);
			newSkel = static_cast<JSkeletonAsset*>(rawPtr);
		}
		if (newSkel != nullptr)
		{
			if (metadata.avatar != nullptr)
				newSkel->impl->SetAvatar(metadata.avatar.get());
			newSkel->SetSkeletonType((JSKELETON_TYPE)metadata.skeletonType); 
		} 
		return newSkel;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkeletonAsset::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JSkeletonAsset::StoreData*>(data);
		if (!storeData->HasCorrectType(JSkeletonAsset::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JSkeletonAsset* skel = static_cast<JSkeletonAsset*>(storeData->obj);
		return skel->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkeletonAsset::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JSkeletonAsset::LoadMetaData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		bool hasAvatar;
		JFileIOHelper::LoadAtomicData(stream, hasAvatar);
		if (hasAvatar)
		{
			loadMetaData->avatar = std::make_unique<JAvatar>();
			for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
			{
				int index;
				JFileIOHelper::LoadAtomicData(stream, index);
				loadMetaData->avatar->jointReference[i] = (uint8)index;
			}
		}
		JFileIOHelper::LoadEnumData(stream, loadMetaData->skeletonType); 

		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkeletonAsset::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JSkeletonAsset::StoreData*>(data);
		JSkeletonAsset* skel = static_cast<JSkeletonAsset*>(storeData->obj);

		std::wofstream stream;
		stream.open(skel->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		//아바타 정보 저장 필요		
		bool hasAvatar = skel->GetAvatar() != nullptr;
		JFileIOHelper::StoreAtomicData(stream, L"HasAvatar:", hasAvatar);
		if (hasAvatar)
		{
			JAvatar* avatar = skel->GetAvatar();
			for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
				JFileIOHelper::StoreAtomicData(stream, std::to_wstring(i) + L"Index:", avatar->jointReference[i]);
		}
		JFileIOHelper::StoreEnumData(stream, L"SkeletonType:", skel->GetSkeletonType()); 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void AvatarInterface::SetAvatar(JSkeletonAsset* skel, JAvatar* avatar)noexcept
	{
		skel->impl->SetAvatar(avatar);
	}
	void AvatarInterface::CopyAvatarJointIndex(JSkeletonAsset* skel, _Out_ JAvatar* target)noexcept
	{
		skel->impl->CopyAvatarJointIndex(target);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JSkeletonAssetPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JSkeletonAssetPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}