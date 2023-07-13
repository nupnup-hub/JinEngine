#include"JSkeletonAsset.h" 
#include"JSkeleton.h"
#include"JSkeletonAssetPrivate.h" 
#include"../JResourceObjectHint.h"
#include"../JClearableInterface.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
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
 
	class JSkeletonAsset::JSkeletonAssetImpl : public Core::JTypeImplBase, public JClearableInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JSkeletonAssetImpl)
	public:
		JWeakPtr<JSkeletonAsset> thisPointer = nullptr;
	public: 
		JOwnerPtr<JSkeleton> skeleton = nullptr;
		JOwnerPtr<JAvatar> avatar = nullptr;
		JSKELETON_TYPE skeletonType; 
	public:
		JSkeletonAssetImpl(const InitData& initData, JSkeletonAsset* thisSkelRaw){}
		~JSkeletonAssetImpl() {}
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
			{
				avatar = Core::JPtrUtil::MakeOwnerPtr<JAvatar>();
				avatar->ownerSkeleton = thisPointer;
			}

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
							//아바타에 할당되지 않는 조인트는 할당된 부모 조인트 refIndex을 allottedParentRefIndex으로 받는다
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
	public:
		bool IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)const noexcept
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
		void StuffResource()
		{
			if (!thisPointer->IsValid())
			{
				const std::wstring path = thisPointer->GetPath();
				if (ImportSkeleton(ReadAssetData(path)))
				{ 
					JSkeletonAsset::LoadMetaData metadata(thisPointer->GetDirectory());
					static_cast<JSkeletonAssetPrivate::AssetDataIOInterface&>(sPrivate.GetAssetDataIOInterface()).LoadMetaData(path, &metadata);
					if (metadata.isValidAvatar)
						SetAvatar(&metadata.avatar);
					thisPointer->SetValid(true);
				}
			}
		}
		void ClearResource()
		{
			if (thisPointer->IsValid())
			{
				skeleton.Clear(); 
				avatar.Clear();
				thisPointer->SetValid(false);
			}
		}
	public:	 
		static std::vector<Joint> ReadAssetData(const std::wstring& path)
		{
			std::wifstream stream;
			stream.open(path, std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return std::vector<Joint>{};

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
			return std::move(joint);
		}
		bool WriteAssetData()
		{
			std::wofstream stream;
			stream.open(thisPointer->GetPath(), std::ios::out | std::ios::binary);
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
		bool ImportSkeleton(std::vector<Joint>&& joint)
		{
			skeleton = Core::JPtrUtil::MakeOwnerPtr<JSkeleton>(std::move(joint));
			if (thisPointer.IsValid())
				skeleton->ownerSkeleton = thisPointer;
			return true;
		}
	public: 
		void Initialize(InitData* initData)
		{
			ImportSkeleton(std::move(initData->joint));
		}
		void RegisterThisPointer(JSkeletonAsset* skel)
		{
			thisPointer = Core::GetWeakPtr(skel);
		}
		static void RegisterTypeData()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JSkeletonAsset::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JSkeletonAsset::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JSkeletonAsset::StaticTypeInfo(), sPrivate);

			IMPL_REALLOC_BIND(JSkeletonAsset::JSkeletonAssetImpl, thisPointer)

			NotifyReAllocPtr notifySkeltonReAllocPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
			{ 
				auto movedSkel = static_cast<JSkeleton*>(movedPtr);
				movedSkel->ownerSkeleton->impl->skeleton.Release();
				movedSkel->ownerSkeleton->impl->skeleton.Reset(movedSkel);
			};
			NotifyReAllocPtr notifyAvatarReAllocPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
			{				
				auto movedAvatar = static_cast<JAvatar*>(movedPtr);
				movedAvatar->ownerSkeleton->impl->avatar.Release();
				movedAvatar->ownerSkeleton->impl->avatar.Reset(movedAvatar);
			};
			auto skeletonReAllocF = std::make_unique<NotifyReAllocF>(notifySkeltonReAllocPtr);
			auto avatarReAllocF = std::make_unique<NotifyReAllocF>(notifyAvatarReAllocPtr);
			std::unique_ptr<JAllocationDesc> skeletonAllocDesc = std::make_unique<JAllocationDesc>();
			std::unique_ptr<JAllocationDesc> avatarAllocDesc = std::make_unique<JAllocationDesc>();
			skeletonAllocDesc->notifyReAllocB = UniqueBind(std::move(skeletonReAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
			avatarAllocDesc->notifyReAllocB = UniqueBind(std::move(avatarReAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
			JSkeleton::StaticTypeInfo().SetAllocationOption(std::move(skeletonAllocDesc));
			JAvatar::StaticTypeInfo().SetAllocationOption(std::move(avatarAllocDesc));
		}
	};

	JSkeletonAsset::InitData::InitData(const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::vector<Joint>&& joint)
		:JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory), joint(std::move(joint))
	{}
	JSkeletonAsset::InitData::InitData(const size_t guid, 
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::vector<Joint>&& joint)
		: JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory), joint(std::move(joint))
	{}
	JSkeletonAsset::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::vector<Joint>&& joint)
		: JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory),
		joint(std::move(joint))
	{}
	bool JSkeletonAsset::InitData::IsValidData()const noexcept
	{
		return JResourceObject::InitData::IsValidData() && joint.size()> 0;
	}

	JSkeletonAsset::LoadMetaData::LoadMetaData(const JUserPtr<JDirectory>& directory)
		:JResourceObject::InitData(JSkeletonAsset::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JSkeletonAsset::PrivateInterface()const noexcept
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
	JUserPtr<JSkeleton> JSkeletonAsset::GetSkeleton()const noexcept
	{ 
		return impl->skeleton;
	}
	JUserPtr<JAvatar> JSkeletonAsset::GetAvatar()const noexcept
	{
		return impl->avatar;
	}
	JSKELETON_TYPE JSkeletonAsset::GetSkeletonType()const noexcept
	{
		return impl->skeletonType;
	}
	size_t JSkeletonAsset::GetSkeletonHash()const noexcept
	{
		return impl->skeleton->GetHash();
	}
	std::wstring JSkeletonAsset::GetJointName(int index)const noexcept
	{
		return impl->skeleton->GetJointName(index);
	}
	std::vector<std::vector<uint8>> JSkeletonAsset::GetSkeletonTreeIndexVec()const noexcept
	{
		return impl->GetSkeletonTreeIndexVec();
	}
	void JSkeletonAsset::SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept
	{
		impl->skeletonType = skeletonType;
	}
	bool JSkeletonAsset::HasAvatar()const noexcept
	{ 
		return impl->avatar != nullptr;
	}
	bool JSkeletonAsset::IsRegularChildJointIndex(uint8 childIndex, uint8 parentIndex)const noexcept
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
	{}
	JSkeletonAsset::~JSkeletonAsset() {}
	 
	using CreateInstanceInterface = JSkeletonAssetPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JSkeletonAssetPrivate::AssetDataIOInterface;
	using AvatarInterface = JSkeletonAssetPrivate:: AvatarInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JSkeletonAsset>(*static_cast<JSkeletonAsset::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JSkeletonAsset* skel = static_cast<JSkeletonAsset*>(createdPtr);
		skel->impl->RegisterThisPointer(skel);
		skel->impl->Initialize(static_cast<JSkeletonAsset::InitData*>(initData));
	} 
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JSkeletonAsset::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JSkeletonAsset::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JSkeletonAsset::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;
		JSkeletonAsset::LoadMetaData metadata(loadData->directory);

		if (LoadMetaData(pathData.engineMetaFileWPath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JSkeletonAsset> newSkel = nullptr;
		if (directory->HasFile(metadata.guid))
			newSkel = Core::GetUserPtr<JSkeletonAsset>(JSkeletonAsset::StaticTypeInfo().TypeGuid(), metadata.guid);

		if (newSkel == nullptr)
		{
			std::unique_ptr<JSkeletonAsset::InitData> initData = std::make_unique<JSkeletonAsset::InitData>(pathData.name,
				metadata.guid,
				metadata.flag,
				(uint8)metadata.formatIndex,
				directory,
				JSkeletonAsset::JSkeletonAssetImpl::ReadAssetData(pathData.engineFileWPath));

			auto idenUser = sPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &sPrivate);
			newSkel.ConnnectChild(idenUser);
		}
		if (newSkel != nullptr)
		{
			if (metadata.isValidAvatar)
				newSkel->impl->SetAvatar(&metadata.avatar);
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

		JUserPtr<JSkeletonAsset> skel;
		skel.ConnnectChild(storeData->obj);
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
		 
		JFileIOHelper::LoadAtomicData(stream, loadMetaData->isValidAvatar);
		if (loadMetaData->isValidAvatar)
		{  
			for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
			{
				int index;
				JFileIOHelper::LoadAtomicData(stream, index);
				loadMetaData->avatar.jointReference[i] = (uint8)index;
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
		JUserPtr<JSkeletonAsset> skel;
		skel.ConnnectChild(storeData->obj);

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
			JUserPtr<JAvatar> avatar = skel->GetAvatar();
			for (uint i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
				JFileIOHelper::StoreAtomicData(stream, std::to_wstring(i) + L"Index:", avatar->jointReference[i]);
		}
		JFileIOHelper::StoreEnumData(stream, L"SkeletonType:", skel->GetSkeletonType()); 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void AvatarInterface::SetAvatar(JSkeletonAsset* skel, JAvatar* avatar)noexcept
	{
		skel->impl->SetAvatar(avatar);
	}
	void AvatarInterface::CopyAvatarJointIndex(JSkeletonAsset* skel, _Inout_ JAvatar* target)noexcept
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