#include"JSkeletonAsset.h" 
#include"Avatar/JAvatar.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Application/JApplicationVariable.h"
#include<fstream>
#include<DirectXMath.h>

namespace JinEngine
{
	using namespace DirectX;
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
	size_t JSkeletonAsset::GetOwnerModelGuid()const noexcept
	{
		return ownerModelGuid;
	}
	std::string JSkeletonAsset::GetJointName(int index)noexcept
	{
		return skeleton->GetJointName(index);
	}
	J_RESOURCE_TYPE JSkeletonAsset::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	} 
	std::string JSkeletonAsset::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::string> JSkeletonAsset::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".skel" };
		return format;
	}
	void JSkeletonAsset::SetSkeletonType(JSKELETON_TYPE skeletonType)noexcept
	{
		JSkeletonAsset::skeletonType = skeletonType;
	}
	bool JSkeletonAsset::HasAvatar()noexcept
	{
		return avatar != nullptr;
	}
	bool JSkeletonAsset::HasValidModelGuid()noexcept
	{
		return isValidOwnerModelGuid;
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
	JSkeletonAssetModelInteface* JSkeletonAsset::ModelInteface()
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
			JDebugTextOut::PrintWstr(JCommonUtility::StringToWstring(GetJointName(i)));
			if (avatar->jointBackReferenceMap[i].refIndex == JSkeletonFixedData::incorrectJointIndex)
				JDebugTextOut::PrintWstr(L"refIndex: nullptr");
			else
				JDebugTextOut::PrintWstr(L"refIndex: " + JCommonUtility::StringToWstring(GetJointName(avatar->jointReference[avatar->jointBackReferenceMap[i].refIndex])));

			JDebugTextOut::PrintWstr(L"allParent: " + JCommonUtility::StringToWstring(GetJointName(avatar->jointReference[avatar->jointBackReferenceMap[i].allottedParentRefIndex])));
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
		}
	}
	bool JSkeletonAsset::ReadSkeletonAssetData()
	{
		const JResourcePathData pathData{ GetWPath() };
		std::wifstream stream;
		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);

		if (stream.is_open())
		{
			std::wstring guide;
			int jointCount; ;
			size_t skeletonHash;
			std::vector<Joint> joint;
			bool hasAvatar;
			JAvatar avatar;
			int skeletonType;
			int index;

			stream >> skeletonHash;
			stream >> jointCount;

			joint.resize(jointCount);

			for (int i = 0; i < jointCount; ++i)
			{
				Joint newJoint;
				std::wstring name;
				int parentIndex;

				stream >> name;
				stream >> parentIndex;
				stream >> newJoint.length;

				stream >> newJoint.inbindPose._11 >> newJoint.inbindPose._12 >> newJoint.inbindPose._13 >> newJoint.inbindPose._14;
				stream >> newJoint.inbindPose._21 >> newJoint.inbindPose._22 >> newJoint.inbindPose._23 >> newJoint.inbindPose._24;
				stream >> newJoint.inbindPose._31 >> newJoint.inbindPose._32 >> newJoint.inbindPose._33 >> newJoint.inbindPose._34;
				stream >> newJoint.inbindPose._41 >> newJoint.inbindPose._42 >> newJoint.inbindPose._43 >> newJoint.inbindPose._44;

				newJoint.name = JCommonUtility::WstringToU8String(name);
				newJoint.parentIndex = (uint8)parentIndex;
				joint[i] = std::move(newJoint); 
			}
			
			stream >> guide >> hasAvatar;
			if (hasAvatar)
			{
				for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
				{
					stream >> guide >> index;
					avatar.jointReference[i] = (uint8)index;
				}
			}
			stream >> guide >> skeletonType;
			stream.close();
			 
			SetSkeleton(JSkeleton{std::move(joint), skeletonHash });
			if (hasAvatar)
				SetAvatar(&avatar);
			SetSkeletonType((JSKELETON_TYPE)skeletonType);
			return true;
		}
		else
			return false;
	}
	void JSkeletonAsset::SetSkeleton(JSkeleton&& skeleon)
	{
		JSkeletonAsset::skeleton = std::make_unique<JSkeleton>(std::move(skeleon));
	}
	void JSkeletonAsset::SetOwnerModelGuid(const size_t modelGuid)
	{
		JSkeletonAsset::ownerModelGuid = modelGuid;
		isValidOwnerModelGuid = JResourceManager::Instance().HasResource(J_RESOURCE_TYPE::MODEL, ownerModelGuid);
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
		 
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		stream.open(skeletonAsset->GetPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			bool hasAvatar = skeletonAsset->GetAvatar() != nullptr;

			stream << "JSkeleton Hash: " << skeletonAsset->skeleton->GetHash() << '\n';
			stream << "JSkeleton JointCount: " << skeletonAsset->skeleton->GetJointCount() << '\n';
		 
			const uint jointCount = skeletonAsset->skeleton->GetJointCount();
			for (uint i = 0; i < skeletonAsset->skeleton->GetJointCount(); ++i)
			{
				const Joint joint = skeletonAsset->skeleton->GetJoint(i);
				stream << JCommonUtility::U8StringToWstring(joint.name) << '\n';
				stream << joint.parentIndex << '\n';
				stream << joint.length << '\n';

				stream << joint.inbindPose._11 << ' ' << joint.inbindPose._12 << ' ' <<
					joint.inbindPose._13 << ' ' << joint.inbindPose._14 << ' ';
				stream << joint.inbindPose._21 << ' ' << joint.inbindPose._22 << ' ' <<
					joint.inbindPose._23 << ' ' << joint.inbindPose._24 << ' ';
				stream << joint.inbindPose._31 << ' ' << joint.inbindPose._32 << ' ' <<
					joint.inbindPose._33 << ' ' << joint.inbindPose._34 << ' ';
				stream << joint.inbindPose._41 << ' ' << joint.inbindPose._42 << ' ' <<
					joint.inbindPose._43 << ' ' << joint.inbindPose._44 << ' ';
				stream << '\n';
			}

			//아바타 정보 저장 필요
			stream << L"HasAvatar: " << hasAvatar << '\n';
			if (hasAvatar)
			{
				JAvatar* avatar = skeletonAsset->GetAvatar();
				for (uint32 i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
					stream << std::to_wstring(i) << L"Index: " << avatar->jointReference[i] << '\n';
			}
			stream << L"SkeletonType: " << (int)skeletonAsset->GetSkeletonType() << '\n';
			stream.close();
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JSkeletonAsset* JSkeletonAsset::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;
		 
		if (!JResourceObject::IsResourceFormat<JSkeletonAsset>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JSkeletonAsset* newSkeletonAsset = nullptr;
		if (directory->HasFile(pathData.fullName))
			newSkeletonAsset = JResourceManager::Instance().GetResourceByPath<JSkeletonAsset>(pathData.strPath);

		if (newSkeletonAsset == nullptr)
		{
			if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
				newSkeletonAsset = new JSkeletonAsset(pathData.name, metadata.guid, metadata.flag, directory, GetFormatIndex<JSkeletonAsset>(pathData.format));
			else
				newSkeletonAsset = new JSkeletonAsset(pathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, directory, GetFormatIndex<JSkeletonAsset>(pathData.format));
		}

		if (newSkeletonAsset->IsValid())
			return newSkeletonAsset;
		else if (newSkeletonAsset->ReadSkeletonAssetData())
			return newSkeletonAsset;
		else
		{
			delete newSkeletonAsset;
			return nullptr;
		}
	}  
	void JSkeletonAsset::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JSkeletonAsset(owner->MakeUniqueFileName(GetDefaultName<JSkeletonAsset>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JSkeletonAsset(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JSkeletonAsset*>(ori)->CopyResource();
		}; 

		JRFI<JSkeletonAsset>::Register(defaultC, initC, loadC, copyC, &JSkeletonAsset::SetSkeleton);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JSkeletonAsset>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JSkeletonAsset::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JSkeletonAsset::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };
		 
		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JSkeletonAsset::JSkeletonAsset(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JSkeletonAssetInterface(name, guid, flag, directory, formatIndex)
	{ }
	JSkeletonAsset::~JSkeletonAsset(){}
}