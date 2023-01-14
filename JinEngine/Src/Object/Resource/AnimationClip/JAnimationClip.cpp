#include"JAnimationClip.h"
#include"../JResourceManager.h"
#include"../JResourceImporter.h"
#include"../JResourceObjectFactory.h"
#include"../Skeleton/Avatar/JAvatar.h"
#include"../Skeleton/JSkeleton.h"
#include"../Skeleton/JSkeletonAsset.h"
#include"../Skeleton/JSkeletonFixedData.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/File/JFileIOHelper.h" 
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationShareData.h"
#include"../../../Core/Loader/FbxLoader/JFbxFileLoader.h"
#include"../../../Application/JApplicationVariable.h"

using namespace DirectX;
namespace JinEngine
{
	JAnimationClip::JAnimationClipInitData::JAnimationClipInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const std::wstring oridataPath,
		Core::JOwnerPtr<JAnimationData> anidata)
		:JResourceInitData(name, guid, flag, directory, JResourceObject::GetFormatIndex<JAnimationClip>(oridataPath)),
		anidata(std::move(anidata))
	{}
	JAnimationClip::JAnimationClipInitData::JAnimationClipInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex)
		: JResourceInitData(name, guid, flag, directory, formatIndex),
		anidata(std::move(anidata))
	{}
	JAnimationClip::JAnimationClipInitData::JAnimationClipInitData(const std::wstring& name,
		JDirectory* directory,
		const std::wstring oridataPath,
		Core::JOwnerPtr<JAnimationData> anidata)
		: JResourceInitData(name, directory, JResourceObject::GetFormatIndex<JAnimationClip>(oridataPath)),
		anidata(std::move(anidata))
	{}
	bool JAnimationClip::JAnimationClipInitData::IsValidCreateData()
	{
		if (JResourceInitData::IsValidCreateData() && anidata.IsValid())
			return true;
		else
			return false;
	}
	J_RESOURCE_TYPE JAnimationClip::JAnimationClipInitData::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::ANIMATION_CLIP;
	}

	JSkeletonAsset* JAnimationClip::GetClipSkeletonAsset()noexcept
	{
		return clipSkeletonAsset;
	}
	float JAnimationClip::GetFramePerSecond()const noexcept
	{
		return framePerSecond;
	}
	bool JAnimationClip::IsLoop()const noexcept
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
	std::wstring JAnimationClip::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::wstring> JAnimationClip::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".fbx" };
		return format;
	}
	void JAnimationClip::SetClipSkeletonAsset(JSkeletonAsset* clipSkeletonAsset)noexcept
	{
		if (IsActivated())
			CallOffResourceReference(JAnimationClip::clipSkeletonAsset);
		JAnimationClip::clipSkeletonAsset = clipSkeletonAsset;
		if (IsActivated())
			CallOnResourceReference(JAnimationClip::clipSkeletonAsset);

		matchClipSkeleton = IsMatchSkeleton();
	}
	void JAnimationClip::SetFramePerSpeed(float value)noexcept
	{
		framePerSecond = value;
	}
	void JAnimationClip::SetLoop(bool value)noexcept
	{
		isLooping = value;
	}
	bool JAnimationClip::IsSameSkeleton(JSkeletonAsset* srcSkeletonAsset)noexcept
	{
		return  clipSkeletonAsset->GetGuid() == srcSkeletonAsset->GetGuid();
	}
	void JAnimationClip::ClipEnter(Core::JAnimationTime& animationTime, Core::JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float nowTime, const float timeOffset)noexcept
	{
		animationTime.timePos = nowTime;
		animationTime.startTime = animationTime.timePos + timeOffset;
		animationTime.endTime = animationTime.timePos + ((1 / framePerSecond) * clipLength);
		animationTime.normalizedTime = 0;
	}
	void JAnimationClip::ClipClose()noexcept
	{
	}
	void JAnimationClip::Update(Core::JAnimationTime& animationTime, Core::JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& localTransform, float nowTime, float deltaTime)noexcept
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
				//MessageBox(0, L"Zero Pose", JCUtil::StrToWstr(tarSkeleton->GetJointName(i)).c_str(), 0);
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
	void JAnimationClip::UpdateUsingAvatar(Core::JAnimationTime& animationTime, Core::JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& localTransform)noexcept
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
	void JAnimationClip::DoCopy(JObject* ori)
	{
		JAnimationClip* oriClip = static_cast<JAnimationClip*>(ori);
		CopyRFile(*oriClip);
		ClearResource();
		StuffResource();

		SetClipSkeletonAsset(oriClip->GetClipSkeletonAsset());
		SetFramePerSpeed(oriClip->GetFramePerSecond());
		SetLoop(oriClip->IsLoop());
	}
	void JAnimationClip::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
		CallOnResourceReference(JAnimationClip::clipSkeletonAsset);
	}
	void JAnimationClip::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		ClearResource();
		CallOffResourceReference(JAnimationClip::clipSkeletonAsset);
	}
	void JAnimationClip::StuffResource()
	{
		if (!Core::JValidInterface::IsValid())
		{
			if (ReadClipData())
				SetValid(true);
		}
	}
	void JAnimationClip::ClearResource()
	{
		if (Core::JValidInterface::IsValid())
		{
			animationSample.clear(); 
			SetValid(false);
		}
	}
	bool JAnimationClip::IsValid()const noexcept
	{
		return Core::JValidInterface::IsValid() && (clipSkeletonAsset != nullptr);
	}
	bool JAnimationClip::WriteClipData()
	{
		std::wofstream stream;
		stream.open(GetPath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return false;

		JFileIOHelper::StoreAtomicData(stream, L"SampleCount:", animationSample.size());
		const uint sampleCount = (uint)animationSample.size();
		for (uint i = 0; i < sampleCount; ++i)
		{
			JFileIOHelper::StoreAtomicData(stream, L"JointPoseCount:", animationSample[i].jointPose.size());
			const uint jointCount = (uint)animationSample[i].jointPose.size();
			for (uint j = 0; j < jointCount; ++j)
			{
				JFileIOHelper::StoreXMFloat4(stream, L"Quaternion:", animationSample[i].jointPose[j].RotationQuat);
				JFileIOHelper::StoreXMFloat3(stream, L"Translation:", animationSample[i].jointPose[j].Translation);
				JFileIOHelper::StoreXMFloat3(stream, L"Scale:", animationSample[i].jointPose[j].Scale);
				JFileIOHelper::StoreAtomicData(stream, L"StTime:", animationSample[i].jointPose[j].stTime);
			}
		}

		JFileIOHelper::StoreAtomicData(stream, L"OriSkeletonHash:", oriSkeletoHash);
		JFileIOHelper::StoreAtomicData(stream, L"clipLength", clipLength);
		stream.close();
		return true;
	}
	bool JAnimationClip::ReadClipData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return false;

		animationSample.clear();

		uint sampleCount;
		JFileIOHelper::LoadAtomicData(stream, sampleCount);
		animationSample.resize(sampleCount);
		for (uint i = 0; i < sampleCount; ++i)
		{
			uint jointCount ;
			JFileIOHelper::LoadAtomicData(stream, jointCount);
			animationSample[i].jointPose.resize(jointCount);
			for (uint j = 0; j < jointCount; ++j)
			{
				JFileIOHelper::LoadXMFloat4(stream, animationSample[i].jointPose[j].RotationQuat);
				JFileIOHelper::LoadXMFloat3(stream, animationSample[i].jointPose[j].Translation);
				JFileIOHelper::LoadXMFloat3(stream, animationSample[i].jointPose[j].Scale);
				JFileIOHelper::LoadAtomicData(stream, animationSample[i].jointPose[j].stTime);
			}
		}
		JFileIOHelper::LoadAtomicData(stream, oriSkeletoHash);
		JFileIOHelper::LoadAtomicData(stream, clipLength);
		stream.close();
		return true;
	}
	void JAnimationClip::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			if (clipSkeletonAsset != nullptr && clipSkeletonAsset->GetGuid() == jRobj->GetGuid())
				SetClipSkeletonAsset(nullptr);
		}
	}
	bool JAnimationClip::ImportAnimationClip(const JAnimationData& jfbxAniData)
	{
		animationSample = jfbxAniData.animationSample;
		oriSkeletoHash = jfbxAniData.skeletonHash;
		clipLength = jfbxAniData.clipLength;
		framePerSecond = jfbxAniData.framePerSecond;

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
		StoreObject(this);

		//Resource 할당은 Activated상태에서 이루어진다
		//Import는 데이터 변환과 메타데이터 저장을 위함
		animationSample.clear();
		return true;
	}
	Core::J_FILE_IO_RESULT JAnimationClip::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JAnimationClip::StoreObject(JAnimationClip* clip)
	{
		if (clip == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)clip->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(clip->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT storeMetaRes = StoreMetadata(stream, clip);
		stream.close();

		if (storeMetaRes != Core::J_FILE_IO_RESULT::SUCCESS)
			return storeMetaRes;

		if (clip->WriteClipData())
			return Core::J_FILE_IO_RESULT::SUCCESS;
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT JAnimationClip::StoreMetadata(std::wofstream& stream, JAnimationClip* clip)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::StoreMetadata(stream, clip);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			JFileIOHelper::StoreHasObjectIden(stream, clip->clipSkeletonAsset);
			JFileIOHelper::StoreAtomicData(stream, L"FramePerSecond", clip->framePerSecond);
			JFileIOHelper::StoreAtomicData(stream, L"IsLooping", clip->isLooping);
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JAnimationClip* JAnimationClip::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JAnimationClipMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JAnimationClip* newClip = nullptr;
		if (directory->HasFile(pathData.name))
			newClip = JResourceManager::Instance().GetResourceByPath<JAnimationClip>(pathData.engineFileWPath);

		if (newClip == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			JAnimationClipInitData initdata{ pathData.name, metadata.guid,metadata.flag, directory, (uint8)metadata.formatIndex };
			if (initdata.IsValidLoadData())
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationClip>(initdata);
				newClip = ownerPtr.Get();
				if (!AddInstance(std::move(ownerPtr)))
					return nullptr;
			}
		}
		if (newClip != nullptr)
			newClip->SetClipSkeletonAsset(metadata.clipSkeletonAsset);
		return newClip;
	}
	Core::J_FILE_IO_RESULT JAnimationClip::LoadMetadata(std::wifstream& stream, JAnimationClipMetadata& metadata)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::LoadMetadata(stream, metadata);

			Core::JIdentifier* skeletonAsset = JFileIOHelper::LoadHasObjectIden(stream);
			JFileIOHelper::LoadAtomicData(stream, metadata.framePerSecond);
			JFileIOHelper::LoadAtomicData(stream, metadata.isLooping);		
			if (skeletonAsset != nullptr && skeletonAsset->GetTypeInfo().IsA(JSkeletonAsset::StaticTypeInfo()))
				metadata.clipSkeletonAsset = static_cast<JSkeletonAsset*>(skeletonAsset);

			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JAnimationClip::RegisterJFunc()
	{
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData>initdata) ->JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CLIP && initdata->IsValidCreateData())
			{
				const JAnimationClipInitData* aInitdata = static_cast<JAnimationClipInitData*>(initdata.Get());
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationClip>(*aInitdata);
				JAnimationClip* newClip = ownerPtr.Get();
				if (AddInstance(std::move(ownerPtr)))
				{
					newClip->ImportAnimationClip(*aInitdata->anidata.Get());
					return newClip;
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
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationClip>(InitData(ori->GetName(),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JAnimationClip>(ori->GetFormat())));

			JAnimationClip* newClip = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newClip->Copy(ori);
				return newClip;
			}
			else
				return nullptr;
		};
		JRFI<JAnimationClip>::Register(defaultC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JAnimationClip>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JAnimationClip::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JAnimationClip::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SKELETON}, true, false};
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});

		auto fbxMeshImportC = [](JDirectory* dir, const Core::JFileImportHelpData importPathData) -> std::vector<JResourceObject*>
		{
			std::vector<JResourceObject*> res;
			using FbxFileTypeInfo = Core::JFbxFileLoaderImpl::FbxFileTypeInfo;
			FbxFileTypeInfo info = Core::JFbxFileLoader::Instance().GetFileTypeInfo(importPathData.oriFilePath);
			if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_SKELETON))
			{
				Core::JFbxAnimationData jfbxAniData;
				Core::J_FBXRESULT loadRes = Core::JFbxFileLoader::Instance().LoadFbxAnimationFile(importPathData.oriFilePath, jfbxAniData);
				if (loadRes == Core::J_FBXRESULT::FAIL)
					return { nullptr };

				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_ANIMATION))
				{
					res.push_back(JRFI<JAnimationClip>::Create(Core::JPtrUtil::MakeOwnerPtr<InitData>(importPathData.name,
						Core::MakeGuid(),
						importPathData.flag,
						dir,
						importPathData.oriFileWPath,
						Core::JPtrUtil::MakeOwnerPtr<JAnimationData>(std::move(jfbxAniData)))));
				}
			}
			return res;
		};

		JResourceImporter::Instance().AddFormatInfo(L".fbx", J_RESOURCE_TYPE::ANIMATION_CLIP, fbxMeshImportC);
	}
	JAnimationClip::JAnimationClip(const JAnimationClipInitData& initdata)
		:JAnimationClipInterface(initdata)
	{
		AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
	}
	JAnimationClip::~JAnimationClip()
	{
		RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
	}
}