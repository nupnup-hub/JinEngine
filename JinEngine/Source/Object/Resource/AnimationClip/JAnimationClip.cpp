#include"JAnimationClip.h"
#include"JAnimationClipPrivate.h" 
#include"../AnimationController/FSM/JAnimationTime.h"
#include"../AnimationController/FSM/JAnimationUpdateData.h"
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
#include"../../JObjectFileIOHelper.h"
#include"../../Directory/JDirectory.h"  
#include"../../Directory/JFile.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h" 
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/Time/JGameTimer.h"
#include"../../../Core/Animation/JAnimationData.h"
#include"../../../Core/Geometry/Mesh/Loader/FbxLoader/JFbxFileLoader.h"
#include"../../../Core/Utility/JCommonUtility.h"
#include"../../../Application/JApplicationProject.h"

 
namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		static JAnimationClipPrivate aPrivate; 
	}
 
	class JAnimationClip::JAnimationClipImpl : public Core::JTypeImplBase,
		public JClearableInterface, 
		public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationClipImpl)
	public:
		JWeakPtr<JAnimationClip> thisPointer;
	public:
		std::vector<Core::JAnimationSample>animationSample;			//fixed
		size_t skeletonHash;			//fixed
	public:
		REGISTER_PROPERTY_EX(clipSkeletonAsset, GetClipSkeletonAsset, SetClipSkeletonAsset, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false))
		JUserPtr<JSkeletonAsset> clipSkeletonAsset;
		uint32 length;
		float framePerSecond;
		REGISTER_PROPERTY_EX(updateFramePerSecond, GetUpdateFPS, SetUpdateFPS, GUI_SLIDER(30, 120))
		float updateFramePerSecond;
		REGISTER_PROPERTY_EX(isLooping, IsLoop, SetLoop, GUI_CHECKBOX())
		bool isLooping = false;
		bool isMatchClipSkeleton = false;
	public:
		JAnimationClipImpl(const InitData& initData, JAnimationClip* thisClipRaw)
		{ }
		~JAnimationClipImpl(){}
	public:
		JUserPtr<JSkeletonAsset> GetClipSkeletonAsset()const noexcept
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
		//animation clip은 import시 animation과 같은 skeleton hash를 보유한 skeletonAsset(Skinned mesh import시 생성되는)을 검색해서 참조한다.
		void SetClipSkeletonAsset(JUserPtr<JSkeletonAsset> newClipSkeletonAsset)noexcept
		{
			if (thisPointer->IsActivated())
				CallOffResourceReference(clipSkeletonAsset.Get());
			clipSkeletonAsset = newClipSkeletonAsset;
			if (thisPointer->IsActivated())
				CallOnResourceReference(clipSkeletonAsset.Get());
			if (thisPointer->IsActivated())
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
		void ClipEnter(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber, const float timeOffset)noexcept
		{
			JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
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
		void Update(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			//JAnimationTime& animationTime, JAnimationUpdateData* updateData, JSkeletonAsset* srcSkeletonAsset, std::vector<DirectX::XMFLOAT4X4>& worldTransform, float nowTime, float deltaTime
			//animationTime.timePos = nowTime;

			JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
			const float deltaTime = updateData->timer->DeltaTime();

			animationTime.timePos += deltaTime;
			animationTime.normalizedTime = (animationTime.timePos - animationTime.startTime) / (animationTime.endTime - animationTime.startTime);
			if (animationTime.timePos >= animationTime.nextUpdateTime)
				animationTime.nextUpdateTime = animationTime.timePos + (1 / updateFramePerSecond);
			else
				return;

			if (!isMatchClipSkeleton)
				return;

			if (!thisPointer->IsSameSkeleton(updateData->modelSkeleton.Get()))
			{
				if (updateData->modelSkeleton->HasAvatar() && clipSkeletonAsset->HasAvatar())
					UpdateUsingAvatar(updateData, layerNumber, updateNumber);
				return;
			}
			float localTime = animationTime.timePos - animationTime.startTime;
			JSkeleton* tarSkeleton = clipSkeletonAsset->GetSkeleton().Get();
			const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			JMatrix4x4* worldTransform = updateData->diagramData[layerNumber].worldTransform[updateNumber];
			worldTransform[0] = JMatrix4x4::Identity();

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

					const XMVECTOR s0 = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
					const XMVECTOR s1 = animationSample[i].jointPose.front().scale.ToXmV();

					const XMVECTOR p0 = XMVectorSet(0, 0, 0, 1.0f);
					const XMVECTOR p1 = animationSample[i].jointPose.front().translation.ToXmV();

					const XMVECTOR q0 = XMVectorSet(0, 0, 0, 0);
					const XMVECTOR q1 = animationSample[i].jointPose.front().rotationQuat.ToXmV();

					const XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
					const XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
					const XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

					worldTransform[i].StoreXM(XMMatrixAffineTransformation(S, zero, Q, P));
				}
				else if (localTime >= animationSample[i].jointPose.back().stTime)
				{
					const XMVECTOR S = animationSample[i].jointPose.back().scale.ToXmV();
					const XMVECTOR P = animationSample[i].jointPose.back().translation.ToXmV();
					const XMVECTOR Q = animationSample[i].jointPose.back().rotationQuat.ToXmV();
					worldTransform[i].StoreXM(XMMatrixAffineTransformation(S, zero, Q, P));
				}
				else
				{
					const uint sampleJointIndex = GetAnimationSampleJointIndex(i, localTime);
					const float lerpPercent = (localTime - animationSample[i].jointPose[sampleJointIndex].stTime)
						/ (animationSample[i].jointPose[sampleJointIndex + 1].stTime - animationSample[i].jointPose[sampleJointIndex].stTime);

					const XMVECTOR s0 = animationSample[i].jointPose[sampleJointIndex].scale.ToXmV();
					const XMVECTOR s1 = animationSample[i].jointPose[sampleJointIndex + 1].scale.ToXmV();

					const XMVECTOR p0 = animationSample[i].jointPose[sampleJointIndex].translation.ToXmV();
					const XMVECTOR p1 = animationSample[i].jointPose[sampleJointIndex + 1].translation.ToXmV();

					const XMVECTOR q0 = animationSample[i].jointPose[sampleJointIndex].rotationQuat.ToXmV();
					const XMVECTOR q1 = animationSample[i].jointPose[sampleJointIndex + 1].rotationQuat.ToXmV();

					const XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
					const XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
					const XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

					worldTransform[i].StoreXM(XMMatrixAffineTransformation(S, zero, Q, P));
				}
				//Debug
				//XMMATRIX bind = srcSkeletonAsset->GetSkeleton()->GetBindPose(i);
				//XMStoreFloat4x4(&worldTransform[i], bind);
			}
		}
		void UpdateUsingAvatar(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			JAvatar* tarAvatar = clipSkeletonAsset->GetAvatar().Get();
			JAvatar* srcAvatar = updateData->modelSkeleton->GetAvatar().Get();
			JSkeleton* tarSkeleton = clipSkeletonAsset->GetSkeleton().Get();
			JSkeleton* srcSkeleton = updateData->modelSkeleton->GetSkeleton().Get();
			 
			const XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			JMatrix4x4* worldTransform = updateData->diagramData[layerNumber].worldTransform[updateNumber];
			worldTransform[0] = JMatrix4x4::Identity();

			const size_t clipGuid = clipSkeletonAsset->GetGuid();

			uint jointCount = (uint)srcSkeleton->GetJointCount();
			for (uint i = 0; i < jointCount; ++i)
				srcAvatar->jointInterpolation[i].isUpdated = false;

			JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
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
				const XMMATRIX additionalM = updateData->additionalBind[clipGuid][i].transform.LoadXM();
				const XMMATRIX srcWorldBindModM = XMMatrixMultiply(additionalM, srcWorldBindM);
				srcAvatar->jointInterpolation[srcIndex].isUpdated = true;

				XMVECTOR S;
				XMVECTOR Q;
				XMVECTOR T;
				if (localTime < animationSample[tarIndex].jointPose.front().stTime)
				{
					float lerpPercent = localTime / animationSample[i].jointPose[0].stTime;

					const XMVECTOR s0 = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
					const XMVECTOR s1 = animationSample[tarIndex].jointPose.front().scale.ToXmV();

					const XMVECTOR t0 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
					const XMVECTOR t1 = animationSample[tarIndex].jointPose.front().translation.ToXmV();

					const XMVECTOR q0 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
					const XMVECTOR q1 = animationSample[tarIndex].jointPose.front().rotationQuat.ToXmV();

					S = XMVectorLerp(s0, s1, lerpPercent);
					T = XMVectorLerp(t0, t1, lerpPercent);
					Q = XMQuaternionSlerp(q0, q1, lerpPercent);
				}
				else if (localTime >= animationSample[tarIndex].jointPose.back().stTime)
				{
					S = animationSample[tarIndex].jointPose.back().scale.ToXmV();
					T = animationSample[tarIndex].jointPose.back().translation.ToXmV();
					Q = animationSample[tarIndex].jointPose.back().rotationQuat.ToXmV();
				}
				else
				{
					const uint sampleJointIndex = GetAnimationSampleJointIndex(tarIndex, localTime);
					const float lerpPercent = (localTime - animationSample[tarIndex].jointPose[sampleJointIndex].stTime)
						/ (animationSample[tarIndex].jointPose[sampleJointIndex + 1].stTime - animationSample[tarIndex].jointPose[sampleJointIndex].stTime);

					const XMVECTOR s0 = animationSample[tarIndex].jointPose[sampleJointIndex].scale.ToXmV();
					const XMVECTOR s1 = animationSample[tarIndex].jointPose[sampleJointIndex + 1].scale.ToXmV();

					const XMVECTOR t0 = animationSample[tarIndex].jointPose[sampleJointIndex].translation.ToXmV();
					const XMVECTOR t1 = animationSample[tarIndex].jointPose[sampleJointIndex + 1].translation.ToXmV();

					const XMVECTOR q0 = animationSample[tarIndex].jointPose[sampleJointIndex].rotationQuat.ToXmV();
					const XMVECTOR q1 = animationSample[tarIndex].jointPose[sampleJointIndex + 1].rotationQuat.ToXmV();

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
				worldTransform[srcIndex].StoreXM(worldModM);
				//XMStoreFloat4x4(&worldTransform[srcIndex], srcWorldBindModM);	
			}

			for (uint i = 0; i < jointCount; ++i)
			{
				if (!srcAvatar->jointInterpolation[i].isUpdated)
				{
					const uint8 parentIndex = srcSkeleton->GetJointParentIndex(i);

					const XMVECTOR tV = srcAvatar->jointInterpolation[i].translation.ToXmV();
					const XMVECTOR qV = srcAvatar->jointInterpolation[i].quaternion.ToXmV();
					const XMVECTOR sV = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
					const XMMATRIX local = XMMatrixAffineTransformation(sV, zero, qV, tV);
					const XMMATRIX parentAniWorld = worldTransform[parentIndex].LoadXM();
					const XMMATRIX finalM = XMMatrixMultiply(local, parentAniWorld);

					worldTransform[i].StoreXM(finalM);
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
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (clipSkeletonAsset.IsValid() && clipSkeletonAsset->GetGuid() == jRobj->GetGuid())
					SetClipSkeletonAsset(JUserPtr<JSkeletonAsset>{});
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
			if (!thisPointer->IsValidTriggerOn())
				ImportAnimationClip(ReadAssetData(thisPointer->GetPath()));
		}
		void ClearResource()
		{
			if (thisPointer->IsValidTriggerOn())
			{
				animationSample.clear();
				thisPointer->SetValid(false);
			}
		}
	public:
		static std::unique_ptr<Core::JAnimationData> ReadAssetData(const std::wstring& path)
		{
			JFileIOTool tool;
			if (!tool.Begin(path, JFileIOTool::TYPE::INPUT_STREAM))
				return nullptr;
 
			std::unique_ptr<Core::JAnimationData> res = std::make_unique<Core::JAnimationData>();
			uint sampleCount;
			JObjectFileIOHelper::LoadAtomicData(tool, sampleCount, "SampleCount:"); 
			res->animationSample.resize(sampleCount);
			for (uint i = 0; i < sampleCount; ++i)
			{
				uint jointCount = 0;
				JObjectFileIOHelper::LoadAtomicData(tool, jointCount, "JointPoseCount:");
				res->animationSample[i].jointPose.resize(jointCount);
				for (uint j = 0; j < jointCount; ++j)
				{
					JObjectFileIOHelper::LoadVector4(tool, res->animationSample[i].jointPose[j].rotationQuat, "Quaternion:");
					JObjectFileIOHelper::LoadVector3(tool, res->animationSample[i].jointPose[j].translation, "translation:");
					JObjectFileIOHelper::LoadVector3(tool, res->animationSample[i].jointPose[j].scale, "scale:");
					JObjectFileIOHelper::LoadAtomicData(tool, res->animationSample[i].jointPose[j].stTime, "StTime:");
				}
			}
			JObjectFileIOHelper::LoadAtomicData(tool, res->skeletonHash, "OriSkeletonHash:");
			JObjectFileIOHelper::LoadAtomicData(tool, res->length, "ClipLength");
			JObjectFileIOHelper::LoadAtomicData(tool, res->framePerSecond, "ClipFramePerSecond");
			tool.Close();
			return std::move(res);
		}
		bool WriteAssetData()
		{
			JFileIOTool tool;
			if (!tool.Begin(thisPointer->GetPath(), JFileIOTool::TYPE::OUTPUT_STREAM))
				return false;
 
			JObjectFileIOHelper::StoreAtomicData(tool, animationSample.size(), "SampleCount:");
			const uint sampleCount = (uint)animationSample.size();
			for (uint i = 0; i < sampleCount; ++i)
			{
				JObjectFileIOHelper::StoreAtomicData(tool, animationSample[i].jointPose.size(), "JointPoseCount:");
				const uint jointCount = (uint)animationSample[i].jointPose.size();
				for (uint j = 0; j < jointCount; ++j)
				{
					JObjectFileIOHelper::StoreVector4(tool, animationSample[i].jointPose[j].rotationQuat, "Quaternion:");
					JObjectFileIOHelper::StoreVector3(tool, animationSample[i].jointPose[j].translation, "translation:");
					JObjectFileIOHelper::StoreVector3(tool, animationSample[i].jointPose[j].scale, "scale:");
					JObjectFileIOHelper::StoreAtomicData(tool, animationSample[i].jointPose[j].stTime, "StTime:");
				}
			}
			JObjectFileIOHelper::StoreAtomicData(tool, skeletonHash, "OriSkeletonHash:");
			JObjectFileIOHelper::StoreAtomicData(tool, length, "ClipLength");
			JObjectFileIOHelper::StoreAtomicData(tool, framePerSecond, "ClipFramePerSecond");
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
			return true;
		}
		bool ImportAnimationClip(std::unique_ptr<Core::JAnimationData>&& aniData)
		{
			animationSample = std::move(aniData->animationSample);
			skeletonHash = aniData->skeletonHash;
			length = aniData->length;
			framePerSecond = aniData->framePerSecond;
			updateFramePerSecond = framePerSecond;

			bool(*ptr)(JSkeletonAsset*, size_t) = [](JSkeletonAsset* skel, size_t hash){return skel->GetSkeletonHash() == hash;};			 
		    auto userPtr = _JResourceManager::Instance().GetByCondition(ptr, false, std::move(skeletonHash));
			if (userPtr != nullptr)
				SetClipSkeletonAsset(userPtr);		
			return true;
		}
	public:
		void NotifyReAlloc()
		{
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void Initialize(InitData* initData)
		{
			ImportAnimationClip(std::move(initData->anidata));
			thisPointer->SetValid(true);
		}
		void RegisterThisPointer(JAnimationClip* aniClip)
		{
			thisPointer = Core::GetWeakPtr(aniClip);
		}
		void RegisterPostCreation()
		{
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
		static void RegisterTypeData()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JAnimationClip::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JAnimationClip::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SKELETON}, true, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});

			auto fbxMeshImportC = [](JUserPtr<JDirectory> dir, const Core::JFileImportHelpData importPathData) -> std::vector<JUserPtr<JResourceObject>>
			{
				std::vector<JUserPtr<JResourceObject>> res;
				using FbxFileTypeInfo = Core::JFbxFileLoader::FbxFileTypeInfo;
				FbxFileTypeInfo info = JFbxFileLoader::Instance().GetFileTypeInfo(importPathData.oriFilePath);
				if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_SKELETON))
				{
					Core::JFbxAnimationData jfbxAniData;
					Core::J_FBXRESULT loadRes = JFbxFileLoader::Instance().LoadFbxAnimationFile(importPathData.oriFilePath, jfbxAniData);
					if (loadRes == Core::J_FBXRESULT::FAIL)
						return { nullptr };

					JUserPtr<JDirectory> fileDir = dir->GetChildDirctoryByName(importPathData.name);
					if (fileDir == nullptr)
						fileDir = JICI::Create<JDirectory>(importPathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, dir);
					
					JUserPtr<JDirectory> animationDir = JICI::Create<JDirectory>(L"Animation", Core::MakeGuid(), OBJECT_FLAG_NONE, fileDir);
					//Animation
					if (HasSQValueEnum(info.typeInfo, Core::J_FBXRESULT::HAS_ANIMATION))
					{
						res.push_back(JICI::Create<JAnimationClip>(importPathData.name,
							Core::MakeGuid(),
							(J_OBJECT_FLAG)importPathData.flag,
							RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.format),
							animationDir,
							std::make_unique<Core::JAnimationData>(std::move(jfbxAniData))));
					}
				}
				return res;
			};

			JResourceObjectImporter::Instance().AddFormatInfo(L".fbx", J_RESOURCE_TYPE::ANIMATION_CLIP, fbxMeshImportC);
			Core::JIdentifier::RegisterPrivateInterface(JAnimationClip::StaticTypeInfo(), aPrivate);

			IMPL_REALLOC_BIND(JAnimationClip::JAnimationClipImpl, thisPointer)
		}
	};

	JAnimationClip::InitData::InitData(const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JAnimationData>&& anidata)
		:JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory), anidata(std::move(anidata))
	{}
	JAnimationClip::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JAnimationData>&& anidata)
		: JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory),
		anidata(std::move(anidata))
	{}
	JAnimationClip::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex, 
		const JUserPtr<JDirectory>& directory,
		std::unique_ptr<Core::JAnimationData>&& anidata)
		: JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory),
		anidata(std::move(anidata))
	{}

	bool JAnimationClip::InitData::IsValidData()const noexcept
	{
		return anidata != nullptr;
	}

	JAnimationClip::LoadMetaData::LoadMetaData(const JUserPtr<JDirectory>& directory)
		:JResourceObject::InitData(JAnimationClip::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JAnimationClip::PrivateInterface()const noexcept
	{
		return aPrivate;
	}
	JUserPtr<JSkeletonAsset> JAnimationClip::GetClipSkeletonAsset()const noexcept
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
	void JAnimationClip::SetClipSkeletonAsset(JUserPtr<JSkeletonAsset> newClipSkeletonAsset)noexcept
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
	void JAnimationClip::ClipEnter(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber, const float timeOffset)noexcept
	{	
		impl->ClipEnter(updateData, layerNumber, updateNumber, timeOffset);
	}
	void JAnimationClip::ClipClose()noexcept
	{
		impl->ClipClose();
	}
	void JAnimationClip::Update(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
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
		impl->ClearResource();
		impl->OffResourceRef();
		JResourceObject::DoDeActivate();
	}
	JAnimationClip::JAnimationClip(InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JAnimationClipImpl>(initData, this))
	{  }
	JAnimationClip::~JAnimationClip()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimationClipPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JAnimationClipPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JAnimationClipPrivate::AssetDataIOInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JAnimationClip>(*static_cast<JAnimationClip::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JAnimationClip* clip = static_cast<JAnimationClip*>(createdPtr);
		clip->impl->RegisterThisPointer(clip);
		clip->impl->RegisterPostCreation();
		clip->impl->Initialize(static_cast<JAnimationClip::InitData*>(initData));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationClip::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JAnimationClip*>(ptr)->impl->DeRegisterPreDestruction();
		JResourceObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationClip::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JAnimationClip::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;
		JAnimationClip::LoadMetaData metadata(loadData->directory);
		 
		if (LoadMetaData(pathData.engineMetaFileWPath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JAnimationClip> newClip = nullptr;
		if (directory->HasFile(metadata.guid))
			newClip = Core::GetUserPtr<JAnimationClip>(JAnimationClip::StaticTypeInfo().TypeGuid(), metadata.guid);

		if (newClip == nullptr)
		{ 
			std::unique_ptr<JAnimationClip::InitData> initData = std::make_unique<JAnimationClip::InitData>( pathData.name,
				metadata.guid, 
				metadata.flag, 
				(uint8)metadata.formatIndex, 
				directory,
				JAnimationClip::JAnimationClipImpl::ReadAssetData(pathData.engineFileWPath));

			auto idenUser = aPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &aPrivate);
			newClip.ConnnectChild(idenUser);
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

		JUserPtr<JAnimationClip> clip;
		clip.ConnnectChild(storeData->obj);
		return clip->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationClip::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		 
		JFileIOTool tool;
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
		auto loadMetaData = static_cast<JAnimationClip::LoadMetaData*>(data);
		if (LoadCommonMetaData(tool, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		loadMetaData->clipSkeletonAsset = JObjectFileIOHelper::_LoadHasIden<JSkeletonAsset>(tool, "SkeletonAsset");
		JObjectFileIOHelper::LoadAtomicData(tool, loadMetaData->updateFramePerSecond, "updateFramePerSecond");
		JObjectFileIOHelper::LoadAtomicData(tool, loadMetaData->isLooping, "IsLooping");
		tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationClip::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JAnimationClip::StoreData*>(data);
		JUserPtr<JAnimationClip> clip;
		clip.ConnnectChild(storeData->obj);	 

		JFileIOTool tool;
		if (!tool.Begin(clip->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(tool, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		JObjectFileIOHelper::_StoreHasIden(tool, clip->impl->clipSkeletonAsset.Get(), "SkeletonAsset");
		JObjectFileIOHelper::StoreAtomicData(tool, clip->impl->updateFramePerSecond, "updateFramePerSecond");
		JObjectFileIOHelper::StoreAtomicData(tool, clip->impl->isLooping, "IsLooping");
		
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationClipPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JAnimationClipPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JAnimationClipPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
 
}