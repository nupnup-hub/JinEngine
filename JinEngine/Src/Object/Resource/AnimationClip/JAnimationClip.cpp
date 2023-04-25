#include"JAnimationClip.h"
#include"JAnimationClipPrivate.h"
#include"JAnimationData.h"
#include"../JClearableInterface.h"
#include"../JResourceObjectHint.h"
#include"../JResourceObjectUserInterface.h"
#include"../JResourceObjectPrivate.h"
#include"../JResourceManager.h"
#include "../JResourceObjectImporter.h" 
#include"../Skeleton/Avatar/JAvatar.h"
#include"../Skeleton/JSkeleton.h"
#include"../Skeleton/JSkeletonAsset.h"
#include"../Skeleton/JSkeletonFixedData.h"
#include"../../Directory/JDirectory.h"  
#include"../../Directory/JFile.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Core/File/JFileIOHelper.h" 
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/Time/JGameTimer.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationUpdateData.h"
#include"../../../Core/Loader/FbxLoader/JFbxFileLoader.h"
#include"../../../Application/JApplicationProject.h"
#include"../../../Utility/JCommonUtility.h"

 
namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		static JAnimationClipPrivate aPrivate; 
	}
 
	class JAnimationClip::JAnimationClipImpl : public Core::JIdentifierImplBase,
		public JClearableInterface, 
		public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationClipImpl)
	public:
		JAnimationClip* thisClip;
	public:
		std::vector<JAnimationSample>animationSample;			//fixed
		size_t skeletonHash;			//fixed
	public:
		REGISTER_PROPERTY_EX(clipSkeletonAsset, GetClipSkeletonAsset, SetClipSkeletonAsset, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false))
		Core::JUserPtr<JSkeletonAsset> clipSkeletonAsset;
		uint32 length;
		float framePerSecond;
		REGISTER_PROPERTY_EX(updateFramePerSecond, GetUpdateFPS, SetUpdateFPS, GUI_SLIDER(30, 120))
		float updateFramePerSecond;
		REGISTER_PROPERTY_EX(isLooping, IsLoop, SetLoop, GUI_CHECKBOX())
		bool isLooping = false;
		bool isMatchClipSkeleton = false;
	public:
		JAnimationClipImpl(const InitData& initData, JAnimationClip* thisClip)
			:JResourceObjectUserInterface(thisClip->GetGuid()),thisClip(thisClip)
		{ }
		~JAnimationClipImpl()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisClip->GetGuid());
		}
	public:
		void Initialize(InitData& initData)
		{
			ImportAnimationClip(std::move(initData.anidata));
			thisClip->SetValid(true);
			AddEventListener(*JResourceObject::EvInterface(), thisClip->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
	public:
		Core::JUserPtr<JSkeletonAsset> GetClipSkeletonAsset()const noexcept
		{
			return clipSkeletonAsset;
		}
		float GetUpdateFPS()const noexcept
		{
			return updateFramePerSecond;
		}
		uint GetSampleKeyCount(const uint sampleIndex)const noexcept
		{
			if (sampleIndex >= (uint)animationSample.size())
				return 0;
			else
				return (uint)animationSample[sampleIndex].jointPose.size();
		}
		uint GetAnimationSampleJointIndex(const uint sampleIndex, const float localTime)const noexcept
		{
			const uint jointCount = (uint)animationSample[sampleIndex].jointPose.size();
			for (uint i = jointCount - 1; i >= 0; --i)
			{
				if (animationSample[sampleIndex].jointPose[i].stTime <= localTime)
					return i;
			}
			return 0;
		}
	public:
		void SetClipSkeletonAsset(Core::JUserPtr<JSkeletonAsset> newClipSkeletonAsset)noexcept
		{
			if (thisClip->IsActivated())
				CallOffResourceReference(clipSkeletonAsset.Get());
			clipSkeletonAsset = newClipSkeletonAsset;
			if (thisClip->IsActivated())
				CallOnResourceReference(clipSkeletonAsset.Get());
			if (thisClip->IsActivated())
				SetMatchSkeletonTrigger();
		}
		void SetUpdateFPS(float value)noexcept
		{
			updateFramePerSecond = value;
		}
		void SetLoop(bool value)noexcept
		{
			isLooping = value;
		}
		void SetMatchSkeletonTrigger()
		{
			isMatchClipSkeleton = clipSkeletonAsset.IsValid() && clipSkeletonAsset->GetSkeleton()->GetJointCount() == animationSample.size();
		}
	public:
		bool IsLoop()const noexcept
		{
			return isLooping;
		}
		bool IsMatchSkeleton()const noexcept
		{
			return isMatchClipSkeleton;
		}
	public:
		void ClipEnter(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber, const float timeOffset)noexcept
		{
			Core::JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
			const float nowTime = updateData->timer->TotalTime();

			animationTime.timePos = nowTime;
			animationTime.startTime = animationTime.timePos + timeOffset;
			animationTime.endTime = animationTime.timePos + ((1 / framePerSecond) * length);
			animationTime.normalizedTime = 0;
			animationTime.nextUpdateTime = animationTime.startTime;
		}
		void ClipClose()noexcept
		{
		}
		void Update(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			//Core::JAnimationTime& animationTime, Core::JAnimationUpdateData* updateData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& worldTransform, float nowTime, float deltaTime
			//animationTime.timePos = nowTime;

			Core::JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
			const float deltaTime = updateData->timer->DeltaTime();

			animationTime.timePos += deltaTime;
			animationTime.normalizedTime = (animationTime.timePos - animationTime.startTime) / (animationTime.endTime - animationTime.startTime);
			if (animationTime.timePos >= animationTime.nextUpdateTime)
				animationTime.nextUpdateTime = animationTime.timePos + (1 / updateFramePerSecond);
			else
				return;

			if (!isMatchClipSkeleton)
				return;

			if (!thisClip->IsSameSkeleton(updateData->modelSkeleton))
			{
				if (updateData->modelSkeleton->HasAvatar() && clipSkeletonAsset->HasAvatar())
				{
					UpdateUsingAvatar(updateData, layerNumber, updateNumber);
					return;
				}
			}
			float localTime = animationTime.timePos - animationTime.startTime;
			JSkeleton* tarSkeleton = clipSkeletonAsset->GetSkeleton();
			const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);;
			const XMFLOAT4X4 identity = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4* worldTransform = updateData->diagramData[layerNumber].worldTransform[updateNumber];
			worldTransform[0] = identity;

			uint sampleSize = (uint)animationSample.size();
			for (uint i = 0; i < sampleSize; ++i)
			{
				if (animationSample[i].jointPose.size() == 0)
				{
					worldTransform[i] = worldTransform[tarSkeleton->GetJointParentIndex(i)];
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

					XMStoreFloat4x4(&worldTransform[i], XMMatrixAffineTransformation(S, zero, Q, P));
				}
				else if (localTime >= animationSample[i].jointPose.back().stTime)
				{
					const XMVECTOR S = XMLoadFloat3(&animationSample[i].jointPose.back().Scale);
					const XMVECTOR P = XMLoadFloat3(&animationSample[i].jointPose.back().Translation);
					const XMVECTOR Q = XMLoadFloat4(&animationSample[i].jointPose.back().RotationQuat);

					XMStoreFloat4x4(&worldTransform[i], XMMatrixAffineTransformation(S, zero, Q, P));
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

					XMStoreFloat4x4(&worldTransform[i], XMMatrixAffineTransformation(S, zero, Q, P));
				}
				//Debug
				//XMMATRIX bind = srcSkeletonAsset->GetSkeleton()->GetBindPose(i);
				//XMStoreFloat4x4(&worldTransform[i], bind);
			}
		}
		void UpdateUsingAvatar(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			JAvatar* tarAvatar = clipSkeletonAsset->GetAvatar();
			JAvatar* srcAvatar = updateData->modelSkeleton->GetAvatar();
			JSkeleton* tarSkeleton = clipSkeletonAsset->GetSkeleton();
			JSkeleton* srcSkeleton = updateData->modelSkeleton->GetSkeleton();

			const XMFLOAT4X4 identity = JMathHelper::Identity4x4();
			const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			DirectX::XMFLOAT4X4* worldTransform = updateData->diagramData[layerNumber].worldTransform[updateNumber];
			worldTransform[0] = identity;

			const size_t clipGuid = clipSkeletonAsset->GetGuid();

			uint jointCount = (uint)srcSkeleton->GetJointCount();
			for (uint i = 0; i < jointCount; ++i)
				srcAvatar->jointInterpolation[i].isUpdated = false;

			Core::JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
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
				const XMMATRIX additionalM = XMLoadFloat4x4(&updateData->additionalBind[clipGuid][i].transform);
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
				XMStoreFloat4x4(&worldTransform[srcIndex], worldModM);
				//XMStoreFloat4x4(&worldTransform[srcIndex], srcWorldBindModM);	
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
					const XMMATRIX parentAniWorld = XMLoadFloat4x4(&worldTransform[parentIndex]);
					const XMMATRIX finalM = XMMatrixMultiply(local, parentAniWorld);

					XMStoreFloat4x4(&worldTransform[i], finalM);
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
				XMStoreFloat4x4(&worldTransform[i], finalM);
			}*/
		} 
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == thisClip->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (clipSkeletonAsset.IsValid() && clipSkeletonAsset->GetGuid() == jRobj->GetGuid())
					SetClipSkeletonAsset(Core::JUserPtr<JSkeletonAsset>{});
			}
		}
		void OnResourceRef()
		{
			CallOnResourceReference(clipSkeletonAsset.Get());
		}
		void OffResourceRef()
		{
			CallOffResourceReference(clipSkeletonAsset.Get());
		}
	public:
		void StuffResource()
		{
			if (!thisClip->IsValidTriggerOn())
				ImportAnimationClip(ReadAssetData(thisClip->GetPath()));
		}
		void ClearResource()
		{
			if (thisClip->IsValidTriggerOn())
			{
				animationSample.clear();
				thisClip->SetValid(false);
			}
		}
	public:
		static std::unique_ptr<JAnimationData> ReadAssetData(const std::wstring& path)
		{
			std::wifstream stream;
			stream.open(path, std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return std::unique_ptr<JAnimationData>{};

			std::unique_ptr<JAnimationData> res = std::make_unique<JAnimationData>();

			uint sampleCount;
			JFileIOHelper::LoadAtomicData(stream, sampleCount);
			res->animationSample.resize(sampleCount);
			for (uint i = 0; i < sampleCount; ++i)
			{
				uint jointCount;
				JFileIOHelper::LoadAtomicData(stream, jointCount);
				res->animationSample[i].jointPose.resize(jointCount);
				for (uint j = 0; j < jointCount; ++j)
				{
					JFileIOHelper::LoadXMFloat4(stream, res->animationSample[i].jointPose[j].RotationQuat);
					JFileIOHelper::LoadXMFloat3(stream, res->animationSample[i].jointPose[j].Translation);
					JFileIOHelper::LoadXMFloat3(stream, res->animationSample[i].jointPose[j].Scale);
					JFileIOHelper::LoadAtomicData(stream, res->animationSample[i].jointPose[j].stTime);
				}
			}
			JFileIOHelper::LoadAtomicData(stream, res->skeletonHash);
			JFileIOHelper::LoadAtomicData(stream, res->length);
			JFileIOHelper::LoadAtomicData(stream, res->framePerSecond);
			stream.close();
			return std::move(res);
		}
		bool WriteAssetData()
		{
			std::wofstream stream;
			stream.open(thisClip->GetPath(), std::ios::out | std::ios::binary);
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

			JFileIOHelper::StoreAtomicData(stream, L"OriSkeletonHash:", skeletonHash);
			JFileIOHelper::StoreAtomicData(stream, L"ClipLength", length);
			JFileIOHelper::StoreAtomicData(stream, L"ClipFramePerSecond", framePerSecond);
			stream.close();
			return true;
		}
		bool ImportAnimationClip(std::unique_ptr<JAnimationData>&& aniData)
		{
			animationSample = std::move(aniData->animationSample);
			skeletonHash = aniData->skeletonHash;
			length = aniData->length;
			framePerSecond = aniData->framePerSecond;
			updateFramePerSecond = framePerSecond;

			bool(*ptr)(JSkeletonAsset*, size_t) = [](JSkeletonAsset* skel, size_t hash){return skel->GetSkeletonHash() == hash;};			 
		    auto rawPtr = _JResourceManager::Instance().GetResourceByCondition<JSkeletonAsset>(ptr, std::move(skeletonHash));
			if (rawPtr != nullptr)
				SetClipSkeletonAsset(Core::GetUserPtr(rawPtr));
			
			return true;
		}
	public:
		static void RegisterCallOnce()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JAnimationClip::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JAnimationClip::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SKELETON}, true, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});

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

					JDirectory* fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, dir);
					
					JDirectory* animationDir = JICI::Create<JDirectory>(L"Animation", Core::MakeGuid(), OBJECT_FLAG_NONE, fileDir);
					//Animation
					if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_ANIMATION))
					{
						auto aInitData = std::make_unique<JAnimationClip::InitData>(importPathData.name,
							Core::MakeGuid(),
							importPathData.flag,
							RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.format),
							animationDir,
							std::make_unique<JAnimationData>(std::move(jfbxAniData)));
				  
						res.push_back(static_cast<JResourceObject*>(JICI::Create(std::move(aInitData)).Get()));
					}
				}
				return res;
			};

			JResourceObjectImporter::Instance().AddFormatInfo(L".fbx", J_RESOURCE_TYPE::ANIMATION_CLIP, fbxMeshImportC);
			Core::JIdentifier::RegisterPrivateInterface(JAnimationClip::StaticTypeInfo(), aPrivate);
		}
	};

	JAnimationClip::InitData::InitData(const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JAnimationData>&& anidata)
		:JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory), anidata(std::move(anidata))
	{}
	JAnimationClip::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		JDirectory* directory,
		std::unique_ptr<JAnimationData>&& anidata)
		: JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory),
		anidata(std::move(anidata))
	{}
	JAnimationClip::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex, 
		JDirectory* directory,
		std::unique_ptr<JAnimationData>&& anidata)
		: JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory),
		anidata(std::move(anidata))
	{}

	bool JAnimationClip::InitData::IsValidData()const noexcept
	{
		return anidata != nullptr;
	}

	JAnimationClip::LoadMetaData::LoadMetaData(JDirectory* directory)
		:JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JAnimationClip::GetPrivateInterface()const noexcept
	{
		return aPrivate;
	}
	Core::JUserPtr<JSkeletonAsset> JAnimationClip::GetClipSkeletonAsset()const noexcept
	{
		return impl->GetClipSkeletonAsset();
	}
	float JAnimationClip::GetUpdateFPS()const noexcept
	{
		return impl->GetUpdateFPS();
	}
	uint JAnimationClip::GetSampleCount()const noexcept
	{
		return (uint)impl->animationSample.size();
	}
	uint JAnimationClip::GetSampleKeyCount(const uint sampleIndex)const noexcept
	{
		return impl->GetSampleKeyCount(sampleIndex);
	}
	J_RESOURCE_TYPE JAnimationClip::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JAnimationClip::GetFormat()const noexcept
	{ 
		return GetAvailableFormat()[GetFormatIndex()];
	}
	std::vector<std::wstring> JAnimationClip::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".fbx" };
		return format;
	}
	void JAnimationClip::SetClipSkeletonAsset(Core::JUserPtr<JSkeletonAsset> newClipSkeletonAsset)noexcept
	{
		impl->SetClipSkeletonAsset(newClipSkeletonAsset);
	}
	void JAnimationClip::SetUpdateFPS(float value)noexcept
	{
		impl->SetUpdateFPS(value);
	}
	void JAnimationClip::SetLoop(bool value)noexcept
	{
		impl->SetLoop(value);
	}
	bool JAnimationClip::IsLoop()const noexcept
	{
		return impl->IsLoop();
	}
	bool JAnimationClip::IsSameSkeleton(JSkeletonAsset* srcSkeletonAsset)noexcept
	{
		return impl->clipSkeletonAsset->GetGuid() == srcSkeletonAsset->GetGuid();
	}
	bool JAnimationClip::IsValid()const noexcept
	{
		return Core::JValidInterface::IsValid() && (impl->clipSkeletonAsset.IsValid());
	}
	void JAnimationClip::ClipEnter(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber, const float timeOffset)noexcept
	{	
		impl->ClipEnter(updateData, layerNumber, updateNumber, timeOffset);
	}
	void JAnimationClip::ClipClose()noexcept
	{
		impl->ClipClose();
	}
	void JAnimationClip::Update(Core::JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
	{ 
		impl->Update(updateData, layerNumber, updateNumber);
	}
	void JAnimationClip::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		impl->StuffResource();
		impl->OnResourceRef();
		impl->SetMatchSkeletonTrigger();
	}
	void JAnimationClip::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		impl->ClearResource();
		impl->OffResourceRef();
	}
	JAnimationClip::JAnimationClip(InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JAnimationClipImpl>(initData, this))
	{ 
		impl->Initialize(initData);
	}
	JAnimationClip::~JAnimationClip()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimationClipPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JAnimationClipPrivate::AssetDataIOInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JAnimationClip>(*static_cast<JAnimationClip::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationClip::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationClip::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JAnimationClip::LoadData*>(data);
		auto pathData = loadData->pathData;
		JDirectory* directory = loadData->directory;
		JAnimationClip::LoadMetaData metadata(loadData->directory);
		 
		if (LoadMetaData(pathData.engineMetaFileWPath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JAnimationClip* newClip = nullptr;
		if (directory->HasFile(metadata.guid))
			newClip = static_cast<JAnimationClip*>(Core::GetUserPtr(JAnimationClip::StaticTypeInfo().TypeGuid(), metadata.guid).Get());

		if (newClip == nullptr)
		{ 
			std::unique_ptr<JAnimationClip::InitData> initData = std::make_unique<JAnimationClip::InitData>( pathData.name,
				metadata.guid, 
				metadata.flag, 
				(uint8)metadata.formatIndex, 
				directory,
				JAnimationClip::JAnimationClipImpl::ReadAssetData(pathData.engineFileWPath));

			auto rawPtr = aPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &aPrivate);
			newClip = static_cast<JAnimationClip*>(rawPtr);
		}
		if (newClip != nullptr)
		{
			newClip->SetLoop(metadata.isLooping);
			newClip->SetUpdateFPS(metadata.updateFramePerSecond);
			newClip->SetClipSkeletonAsset(metadata.clipSkeletonAsset);
		}
		return newClip;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationClip::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JAnimationClip::StoreData*>(data);
		if(!storeData->HasCorrectType(JAnimationClip::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JAnimationClip* clip = static_cast<JAnimationClip*>(storeData->obj);
		return clip->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationClip::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JAnimationClip::LoadMetaData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		loadMetaData->clipSkeletonAsset = JFileIOHelper::LoadHasObjectIden<JSkeletonAsset>(stream);
		JFileIOHelper::LoadAtomicData(stream, loadMetaData->updateFramePerSecond);
		JFileIOHelper::LoadAtomicData(stream, loadMetaData->isLooping);

		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationClip::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JAnimationClip::StoreData*>(data);
		JAnimationClip* clip = static_cast<JAnimationClip*>(storeData->obj);

		std::wofstream stream;
		stream.open(clip->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		JFileIOHelper::StoreHasObjectIden(stream, clip->impl->clipSkeletonAsset.Get());
		JFileIOHelper::StoreAtomicData(stream, L"updateFramePerSecond", clip->impl->updateFramePerSecond);
		JFileIOHelper::StoreAtomicData(stream, L"IsLooping", clip->impl->isLooping);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationClipPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JAnimationClipPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
 
}