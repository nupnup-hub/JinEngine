#include"JAnimationClip.h"
#include"../JResourceManager.h"
#include"../JResourceObjectFactory.h"
#include"../Skeleton/Avatar/JAvatar.h"
#include"../Skeleton/JSkeleton.h"
#include"../Skeleton/JSkeletonAsset.h"
#include"../Skeleton/JSkeletonFixedData.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationShareData.h"
#include"../../../Core/Loader/FbxLoader/JFbxFileLoader.h"
#include"../../../Application/JApplicationVariable.h"

using namespace DirectX;
namespace JinEngine
{
	using namespace Core;
	JSkeletonAsset* JAnimationClip::GetClipSkeletonAsset()noexcept
	{
		return clipSkeletonAsset;
	}
	bool JAnimationClip::GetIsLoop()const noexcept
	{
		return isLooping;
	}
	uint JAnimationClip::GetSampleCount()const noexcept
	{
		return (uint)animationSample.size();
	}
	uint JAnimationClip::GetSampleKeyCount(const uint sampleIndex)const noexcept
	{
		if (sampleIndex >= (uint)animationSample.size())
			return 0;
		else
			return (uint)animationSample[sampleIndex].jointPose.size();
	}
	J_RESOURCE_TYPE JAnimationClip::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::string JAnimationClip::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::string> JAnimationClip::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".fbx" };
		return format;
	}
	void JAnimationClip::SetClipSkeletonAsset(JSkeletonAsset* clipSkeletonAsset)noexcept
	{
		if (JAnimationClip::clipSkeletonAsset != nullptr)
			OffResourceReference(*JAnimationClip::clipSkeletonAsset);

		JAnimationClip::clipSkeletonAsset = clipSkeletonAsset;

		if (JAnimationClip::clipSkeletonAsset != nullptr)
			OnResourceReference(*JAnimationClip::clipSkeletonAsset);

		JAnimationClip::clipSkeletonAsset = clipSkeletonAsset;
		matchClipSkeleton = IsMatchSkeleton();
	}
	bool JAnimationClip::IsSameSkeleton(JSkeletonAsset* srcSkeletonAsset)noexcept
	{
		return  clipSkeletonAsset->GetGuid() == srcSkeletonAsset->GetGuid();
	}
	void JAnimationClip::ClipEnter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float nowTime, const float timeOffset)noexcept
	{
		animationTime.timePos = nowTime;
		animationTime.startTime = animationTime.timePos + timeOffset;
		animationTime.endTime = animationTime.timePos + ((1 / framePerSecond) * clipLength);
		animationTime.normalizedTime = 0;
	}
	void JAnimationClip::ClipClose()noexcept
	{
	}
	void JAnimationClip::Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& localTransform, float nowTime, float deltaTime)noexcept
	{
		//animationTime.timePos = nowTime;
		animationTime.timePos += deltaTime;
		animationTime.normalizedTime = (animationTime.timePos - animationTime.startTime) / (animationTime.endTime - animationTime.startTime);

		if (!matchClipSkeleton)
			return;

		if (!IsSameSkeleton(srcSkeletonAsset))
		{
			if (srcSkeletonAsset->HasAvatar() && clipSkeletonAsset->HasAvatar())
			{
				UpdateUsingAvatar(animationTime, animationShareData, srcSkeletonAsset, localTransform);
				return;
			}
		}
		float localTime = animationTime.timePos - animationTime.startTime;
		JSkeleton* tarSkeleton = clipSkeletonAsset->GetSkeleton();
		const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);;
		const XMFLOAT4X4 identity = JMathHelper::Identity4x4();
		localTransform[0] = identity;

		uint sampleSize = (uint)animationSample.size();
		for (uint i = 0; i < sampleSize; ++i)
		{
			if (animationSample[i].jointPose.size() == 0)
			{
				//MessageBox(0, L"Zero Pose", JCommonUtility::StringToWstring(tarSkeleton->GetJointName(i)).c_str(), 0);
				const uint parentIndex = tarSkeleton->GetJointParentIndex(i);
				const XMMATRIX parentWorldBindM = tarSkeleton->GetBindPose(parentIndex);
				const XMMATRIX childWorldBindM = tarSkeleton->GetBindPose(i);
				const XMMATRIX parentRes = XMLoadFloat4x4(&localTransform[parentIndex]);

				const XMMATRIX childLocalBindM = XMMatrixMultiply(childWorldBindM, XMMatrixInverse(nullptr, parentWorldBindM));
				const XMMATRIX childWorldAniM = XMMatrixMultiply(childLocalBindM, parentRes);
				//XMStoreFloat4x4(&localTransform[i], childWorldAniM);
				continue;
			}
			if (localTime < animationSample[i].jointPose.front().stTime)
			{
				float lerpPercent = localTime / animationSample[i].jointPose[0].stTime;
				const XMFLOAT3 initScale(1, 1, 1);
				const XMFLOAT3 initTrans(0, 0, 0);
				const XMFLOAT4 initRot(0, 0, 0, 0);

				const XMVECTOR s0 = XMLoadFloat3(&initScale);
				const XMVECTOR s1 = XMLoadFloat3(&animationSample[i].jointPose.front().Scale);

				const XMVECTOR p0 = XMLoadFloat3(&initTrans);
				const XMVECTOR p1 = XMLoadFloat3(&animationSample[i].jointPose.front().Translation);

				const XMVECTOR q0 = XMLoadFloat4(&initRot);
				const XMVECTOR q1 = XMLoadFloat4(&animationSample[i].jointPose.front().RotationQuat);

				const XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				const XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				const XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMStoreFloat4x4(&localTransform[i], XMMatrixAffineTransformation(S, zero, Q, P));
			}
			else if (localTime >= animationSample[i].jointPose.back().stTime)
			{
				const XMVECTOR S = XMLoadFloat3(&animationSample[i].jointPose.back().Scale);
				const XMVECTOR P = XMLoadFloat3(&animationSample[i].jointPose.back().Translation);
				const XMVECTOR Q = XMLoadFloat4(&animationSample[i].jointPose.back().RotationQuat);

				XMStoreFloat4x4(&localTransform[i], XMMatrixAffineTransformation(S, zero, Q, P));
			}
			else
			{
				const uint sampleJointIndex = GetAnimationSampleJointIndex(i, localTime);
				const float lerpPercent = (localTime - animationSample[i].jointPose[sampleJointIndex].stTime)
					/ (animationSample[i].jointPose[sampleJointIndex + 1].stTime - animationSample[i].jointPose[sampleJointIndex].stTime);

				const XMVECTOR s0 = XMLoadFloat3(&animationSample[i].jointPose[sampleJointIndex].Scale);
				const XMVECTOR s1 = XMLoadFloat3(&animationSample[i].jointPose[sampleJointIndex + 1].Scale);

				const XMVECTOR p0 = XMLoadFloat3(&animationSample[i].jointPose[sampleJointIndex].Translation);
				const XMVECTOR p1 = XMLoadFloat3(&animationSample[i].jointPose[sampleJointIndex + 1].Translation);

				const XMVECTOR q0 = XMLoadFloat4(&animationSample[i].jointPose[sampleJointIndex].RotationQuat);
				const XMVECTOR q1 = XMLoadFloat4(&animationSample[i].jointPose[sampleJointIndex + 1].RotationQuat);

				const XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				const XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				const XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMStoreFloat4x4(&localTransform[i], XMMatrixAffineTransformation(S, zero, Q, P));
			}
			//Debug
			//XMMATRIX bind = srcSkeletonAsset->GetSkeleton()->GetBindPose(i);
			//XMStoreFloat4x4(&localTransform[i], bind);
		}
	}
	uint JAnimationClip::GetAnimationSampleJointIndex(const uint sampleIndex, const float localTime)noexcept
	{
		const uint jointCount = (uint)animationSample[sampleIndex].jointPose.size();
		for (uint i = jointCount - 1; i >= 0; --i)
		{
			if (animationSample[sampleIndex].jointPose[i].stTime <= localTime)
				return i;
		}
		return 0;
	}
	void JAnimationClip::UpdateUsingAvatar(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& localTransform)noexcept
	{
		JAvatar* tarAvatar = clipSkeletonAsset->GetAvatar();
		JAvatar* srcAvatar = srcSkeletonAsset->GetAvatar();
		JSkeleton* tarSkeleton = clipSkeletonAsset->GetSkeleton();
		JSkeleton* srcSkeleton = srcSkeletonAsset->GetSkeleton();

		const XMFLOAT4X4 identity = JMathHelper::Identity4x4();
		const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		localTransform[0] = identity;

		const size_t clipGuid = clipSkeletonAsset->GetGuid();

		uint jointCount = (uint)srcSkeleton->GetJointCount();
		for (uint i = 0; i < jointCount; ++i)
			srcAvatar->jointInterpolation[i].isUpdated = false;

		float localTime = animationTime.timePos - animationTime.startTime;

		for (uint i = 0; i < JSkeletonFixedData::avatarBoneIndexCount; ++i)
		{
			const uint8 tarIndex = tarAvatar->jointReference[i];
			const uint8 srcIndex = srcAvatar->jointReference[i];

			if (tarIndex == JSkeletonFixedData::incorrectJointIndex || srcIndex == JSkeletonFixedData::incorrectJointIndex)
				continue;
			if (animationSample[tarIndex].jointPose.size() == 0)
				continue;

			const uint8 parentRefIndex = JAvatar::jointReferenceParent[i];
			const uint8 tarParentIndex = tarAvatar->jointReference[parentRefIndex];
			const uint8 srcParentIndex = srcAvatar->jointReference[parentRefIndex];

			const XMMATRIX tarWorldBindM = tarSkeleton->GetBindPose(tarIndex);
			const XMMATRIX srcWorldBindM = srcSkeleton->GetBindPose(srcIndex);
			const XMMATRIX additionalM = XMLoadFloat4x4(&animationShareData.additionalBind[clipGuid][i].transform);
			const XMMATRIX srcWorldBindModM = XMMatrixMultiply(additionalM, srcWorldBindM);
			srcAvatar->jointInterpolation[srcIndex].isUpdated = true;

			XMVECTOR S;
			XMVECTOR Q;
			XMVECTOR T;
			if (localTime < animationSample[tarIndex].jointPose.front().stTime)
			{
				float lerpPercent = localTime / animationSample[i].jointPose[0].stTime;
				const XMFLOAT3 initScale(1, 1, 1);
				const XMFLOAT3 initTrans(0, 0, 0);
				const XMFLOAT4 initRot(0, 0, 0, 0);

				const XMVECTOR s0 = XMLoadFloat3(&initScale);
				const XMVECTOR s1 = XMLoadFloat3(&animationSample[tarIndex].jointPose.front().Scale);

				const XMVECTOR t0 = XMLoadFloat3(&initTrans);
				const XMVECTOR t1 = XMLoadFloat3(&animationSample[tarIndex].jointPose.front().Translation);

				const XMVECTOR q0 = XMLoadFloat4(&initRot);
				const XMVECTOR q1 = XMLoadFloat4(&animationSample[tarIndex].jointPose.front().RotationQuat);

				S = XMVectorLerp(s0, s1, lerpPercent);
				T = XMVectorLerp(t0, t1, lerpPercent);
				Q = XMQuaternionSlerp(q0, q1, lerpPercent);
			}
			else if (localTime >= animationSample[tarIndex].jointPose.back().stTime)
			{
				S = XMLoadFloat3(&animationSample[tarIndex].jointPose.back().Scale);
				T = XMLoadFloat3(&animationSample[tarIndex].jointPose.back().Translation);
				Q = XMLoadFloat4(&animationSample[tarIndex].jointPose.back().RotationQuat);
			}
			else
			{
				const uint sampleJointIndex = GetAnimationSampleJointIndex(tarIndex, localTime);
				const float lerpPercent = (localTime - animationSample[tarIndex].jointPose[sampleJointIndex].stTime)
					/ (animationSample[tarIndex].jointPose[sampleJointIndex + 1].stTime - animationSample[tarIndex].jointPose[sampleJointIndex].stTime);

				const XMVECTOR s0 = XMLoadFloat3(&animationSample[tarIndex].jointPose[sampleJointIndex].Scale);
				const XMVECTOR s1 = XMLoadFloat3(&animationSample[tarIndex].jointPose[sampleJointIndex + 1].Scale);

				const XMVECTOR t0 = XMLoadFloat3(&animationSample[tarIndex].jointPose[sampleJointIndex].Translation);
				const XMVECTOR t1 = XMLoadFloat3(&animationSample[tarIndex].jointPose[sampleJointIndex + 1].Translation);

				const XMVECTOR q0 = XMLoadFloat4(&animationSample[tarIndex].jointPose[sampleJointIndex].RotationQuat);
				const XMVECTOR q1 = XMLoadFloat4(&animationSample[tarIndex].jointPose[sampleJointIndex + 1].RotationQuat);

				S = XMVectorLerp(s0, s1, lerpPercent);
				T = XMVectorLerp(t0, t1, lerpPercent);
				Q = XMQuaternionSlerp(q0, q1, lerpPercent);
			}

			XMVECTOR tarS;
			XMVECTOR tarQ;
			XMVECTOR tarT;
			XMVECTOR srcS;
			XMVECTOR srcQ;
			XMVECTOR srcT;

			XMMatrixDecompose(&tarS, &tarQ, &tarT, tarWorldBindM);
			XMMatrixDecompose(&srcS, &srcQ, &srcT, srcWorldBindModM);

			const XMMATRIX tarToSrcQM = XMMatrixRotationQuaternion(XMQuaternionMultiply(srcQ, XMQuaternionInverse(tarQ)));
			const XMMATRIX tarToSrcWorldBindModM = XMMatrixMultiply(tarToSrcQM, tarWorldBindM);

			const XMVECTOR tarLengthV = XMVector3Length(tarT);
			const XMVECTOR srcLengthV = XMVector3Length(srcT);
			XMFLOAT3 tarLengthF;
			XMFLOAT3 srcLengthF;
			XMStoreFloat3(&tarLengthF, tarLengthV);
			XMStoreFloat3(&srcLengthF, srcLengthV);

			const float lengthRate = (srcLengthF.x + 0.001f) / (tarLengthF.x + 0.001f);

			const XMMATRIX worldM = XMMatrixAffineTransformation(S, zero, Q, T);
			const XMMATRIX tarToSrcAnimationM = XMMatrixMultiply(tarToSrcQM, worldM);
			const XMMATRIX onlyAnimationM = XMMatrixMultiply(tarToSrcAnimationM, XMMatrixInverse(nullptr, tarToSrcWorldBindModM));

			XMVECTOR animationModS;
			XMVECTOR animationModQ;
			XMVECTOR animationModT;
			XMMatrixDecompose(&animationModS, &animationModQ, &animationModT, onlyAnimationM);
			animationModT = XMVectorScale(animationModT, lengthRate);

			const XMMATRIX onlyAnimationModM = XMMatrixAffineTransformation(animationModS, zero, animationModQ, animationModT);
			const XMMATRIX worldModM = XMMatrixMultiply(onlyAnimationModM, srcWorldBindModM);
			XMStoreFloat4x4(&localTransform[srcIndex], worldModM);
			//XMStoreFloat4x4(&localTransform[srcIndex], srcWorldBindModM);	
		}

		for (uint i = 0; i < jointCount; ++i)
		{
			if (!srcAvatar->jointInterpolation[i].isUpdated)
			{
				const uint8 parentIndex = srcSkeleton->GetJointParentIndex(i);

				const XMVECTOR tV = XMLoadFloat3(&srcAvatar->jointInterpolation[i].translation);
				const XMVECTOR qV = XMLoadFloat4(&srcAvatar->jointInterpolation[i].quaternion);
				const XMVECTOR sV = XMVectorSet(1, 1, 1, 0);
				const XMMATRIX local = XMMatrixAffineTransformation(sV, zero, qV, tV);
				const XMMATRIX parentAniWorld = XMLoadFloat4x4(&localTransform[parentIndex]);
				const XMMATRIX finalM = XMMatrixMultiply(local, parentAniWorld);

				XMStoreFloat4x4(&localTransform[i], finalM);
			}
		}
		/*JDebugTimer::StopGameTimer();
		float eTime = JDebugTimer::GetElapsedTime();

		JDebugTextOut::OpenStream(L"D:\\NewProjTextFolder\\ETime_Retarget00.txt", std::ios::app, 25);
		JDebugTextOut::PrintFloatData(eTime, L"eTime");
		JDebugTextOut::CloseStream();*/
		/*for (uint i = 0; i < jointCount; ++i)
		{
			const XMMATRIX aniWorld = XMLoadFloat4x4(&animationShareData.localAnimationTransform[i]);
			const XMMATRIX additionalWorld = XMLoadFloat4x4(&animationShareData.additionalBind[clipGuid][i]);
			const XMMATRIX finalM = XMMatrixMultiply(additionalWorld, aniWorld);
			XMStoreFloat4x4(&localTransform[i], finalM);
		}*/
	}
	bool JAnimationClip::IsMatchSkeleton()const noexcept
	{
		uint jointMax = 0;
		uint sampleSize = (uint)animationSample.size();
		for (uint i = 0; i < sampleSize; ++i)
		{
			uint sampleJointSize = (uint)animationSample[i].jointPose.size();
			if (jointMax < sampleJointSize)
				jointMax = sampleJointSize;
		}

		if (JAnimationClip::clipSkeletonAsset->GetSkeleton()->GetJointCount() == sampleSize)
			return true;
		else
			return false;
	}
	void JAnimationClip::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
	}
	void JAnimationClip::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		ClearResource();
	}
	void JAnimationClip::StuffResource()
	{
		if (!JValidInterface::IsValid())
		{
			if (ReadFbxData())
				SetValid(true);
		}
	}
	void JAnimationClip::ClearResource()
	{
		if (JValidInterface::IsValid())
		{
			animationSample.clear();
			SetClipSkeletonAsset(nullptr);
			SetValid(false);
		}
	}
	bool JAnimationClip::IsValid()const noexcept
	{
		return JValidInterface::IsValid() && (clipSkeletonAsset != nullptr);
	}
	bool JAnimationClip::ReadFbxData()
	{
		const JResourcePathData pathData{ GetWPath() };

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		AnimationClipMetadata metadata;
		J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, pathData.folderPath, metadata);
		stream.close();

		JFbxAnimationData jfbxAniData;
		J_FBXRESULT res = JFbxFileLoader::Instance().LoadFbxAnimationFile(pathData.strPath, jfbxAniData);
		if (res == J_FBXRESULT::HAS_ANIMATION)
		{
			if (animationSample.size() != jfbxAniData.animationSample.size())
				animationSample.resize(jfbxAniData.animationSample.size());

			animationSample = jfbxAniData.animationSample;
			oriSkeletoHash = jfbxAniData.skeletonHash;
			clipLength = jfbxAniData.clipLength;
			framePerSecond = jfbxAniData.framePerSecond;

			if (loadMetaRes == J_FILE_IO_RESULT::SUCCESS)
			{
				isLooping = metadata.isLooping;
				if (metadata.hasSkeleton)
				{
					JSkeletonAsset* skeletonAsset = JResourceManager::Instance().GetResource<JSkeletonAsset>(metadata.skeletonGuid);
					if (skeletonAsset != nullptr)
						SetClipSkeletonAsset(skeletonAsset);
				}
			}

			if (clipSkeletonAsset == nullptr)
			{
				uint count;
				std::vector<JResourceObject*>::const_iterator st = JResourceManager::Instance().GetResourceVectorHandle<JSkeletonAsset>(count);
				for (uint i = 0; i < count; ++i)
				{
					JSkeletonAsset* oldSkeletonAsset = static_cast<JSkeletonAsset*>(*(st + i));
					if (oldSkeletonAsset->GetSkeleton()->IsSame(oriSkeletoHash))
					{
						SetClipSkeletonAsset(oldSkeletonAsset);
						break;
					}
				}
			}
			return true;
		}
		else
			return false;
	}
	void JAnimationClip::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			if (clipSkeletonAsset->GetGuid() == jRobj->GetGuid())
				SetClipSkeletonAsset(nullptr);
		}
	}
	J_FILE_IO_RESULT JAnimationClip::CallStoreResource()
	{
		return StoreObject(this);
	}
	J_FILE_IO_RESULT JAnimationClip::StoreObject(JAnimationClip* clip)
	{
		if (clip == nullptr)
			return J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)clip->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(clip->GetMetafilePath(), std::ios::out | std::ios::binary);
		J_FILE_IO_RESULT storeMetaRes = StoreMetadata(stream, clip);
		stream.close();

		return storeMetaRes;
	}
	J_FILE_IO_RESULT JAnimationClip::StoreMetadata(std::wofstream& stream, JAnimationClip* clip)
	{
		if (stream.is_open())
		{
			JResourceObject::StoreMetadata(stream, clip);
			if (clip->clipSkeletonAsset != nullptr)
			{
				stream << "HasSkeleton: " << true << '\n';
				stream << "skeletonGuid: " << clip->clipSkeletonAsset->GetGuid() << '\n';
			}
			else
			{
				stream << "HasSkeleton: " << false << '\n';
				stream << "skeletonGuid: " << "0" << '\n';
			}
			stream << "IsLoop" << clip->isLooping << '\n';
			return J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JAnimationClip* JAnimationClip::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		if (JResourceObject::IsResourceFormat<JAnimationClip>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		AnimationClipMetadata metadata;
		J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, pathData.folderPath, metadata);
		stream.close();

		JAnimationClip* newClip = nullptr;
		if (directory->HasFile(pathData.fullName))
			newClip = JResourceManager::Instance().GetResourceByPath<JAnimationClip>(pathData.strPath);
		
		if (newClip == nullptr)
		{
			if (loadMetaRes == J_FILE_IO_RESULT::SUCCESS)
				newClip = new JAnimationClip(pathData.name, metadata.guid, metadata.flag, directory, GetFormatIndex<JAnimationClip>(pathData.format));
			else
				newClip = new JAnimationClip(pathData.name, MakeGuid(), OBJECT_FLAG_NONE, directory, GetFormatIndex<JAnimationClip>(pathData.format));
		}	

		if (newClip->IsValid())
			return newClip;
		else if (newClip->ReadFbxData())
		{
			newClip->SetValid(true);
			return newClip;
		}
		else
		{
			delete newClip;
			return nullptr;
		}
	}
	J_FILE_IO_RESULT JAnimationClip::LoadMetadata(std::wifstream& stream, const std::string& folderPath, AnimationClipMetadata& metadata)
	{
		if (stream.is_open())
		{
			AnimationClipMetadata metadata;
			J_FILE_IO_RESULT metaLoadRes = JResourceObject::LoadMetadata(stream, metadata);
			if (metaLoadRes != J_FILE_IO_RESULT::SUCCESS)
				return metaLoadRes;

			std::wstring guide;

			stream >> guide >> metadata.hasSkeleton;
			stream >> guide >> guide;
			stream >> guide >> metadata.skeletonGuid;
			stream >> guide >> metadata.isLooping;
			stream.close();
			return J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return J_FILE_IO_RESULT::FAIL_SEARCH_DATA;
	}
	void JAnimationClip::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JAnimationClip(owner->MakeUniqueFileName(GetDefaultName<JAnimationClip>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return new JAnimationClip(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JAnimationClip*>(ori)->CopyResource();
		};
		JRFI<JAnimationClip>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JAnimationClip>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JAnimationClip::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JAnimationClip::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SKELETON}, true, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JAnimationClip::JAnimationClip(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JAnimationClipInterface(name, guid, flag, directory, formatIndex)
	{}
	JAnimationClip::~JAnimationClip()
	{
		clipSkeletonAsset = nullptr;
	}
}