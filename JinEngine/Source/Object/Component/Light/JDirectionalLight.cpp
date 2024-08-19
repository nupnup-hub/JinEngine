/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JDirectionalLight.h"   
#include"JDirectionalLightPrivate.h"
#include"JLightConstants.h"
#include"../Camera/JCameraConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"  
#include"../../Resource/JResourceManager.h"
#include"../../GraphicRule/JGraphicModuleInterfaceHolder.h"
#include"../../GraphicRule/JGraphicModuleUtility.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Func/JFuncList.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Geometry/JDirectXCollisionEx.h"
#include"../../../Core/Math/JMathHelper.h"   
#include<Windows.h>
#include<fstream> 

using namespace DirectX;
namespace JinEngine
{
	namespace Private
	{
		//static auto isAvailableoverlapLam = []() {return false; };
		static JDirectionalLightPrivate instance;

		static constexpr float minPower = 0.1f;
		static constexpr float maxPower = 4.0f;

		static XMVECTOR GetInitDir()
		{
			return XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		}
		static XMVECTOR CalLightWorldDir(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMVector3Normalize(XMVector3Rotate(GetInitDir(), transform->GetWorldQuaternion().ToXmV()));
		}
		static XMVECTOR CalLightWorldPos(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMVectorScale(CalLightWorldDir(transform), -(Constants::globarLightMaxDistance * 0.5f));
		}
		static XMMATRIX CalView(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMMatrixLookAtLH(CalLightWorldPos(transform),
				XMVectorSet(0, 0, 0, 1),
				XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
		}
		static XMMATRIX CalProj(const BoundingBox& sceneBBox,
			const JMatrix4x4& view,
			const size_t mapSize,
			_Out_ JVector3<float>& outMin,
			_Out_ JVector3<float>& outMax)noexcept
		{
			static constexpr bool isFitToScene = true;
			static constexpr bool canMoveLightTexelSize = true;

			const XMMATRIX viewM = view.LoadXM();

			XMFLOAT3 corner[8];
			sceneBBox.GetCorners(corner);

			XMVECTOR minV = JVector3<float>::PositiveInfV().ToXmV();
			XMVECTOR maxV = JVector3<float>::NegativeInfV().ToXmV();

			for (int i = 0; i < 8; ++i)
			{
				const XMVECTOR v = XMVector3Transform(XMLoadFloat3(&corner[i]), viewM);
				minV = XMVectorMin(minV, v);
				maxV = XMVectorMax(maxV, v);
			}

			XMVECTOR worldUnitsPerTexel = JVector4<float>::Zero().ToXmV();
			XMVECTOR multiplySetzwToZeroV = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);
			if (isFitToScene)
			{
				XMVECTOR vDiagonal = XMLoadFloat3(&corner[0]) - XMLoadFloat3(&corner[6]);
				vDiagonal = XMVector3Length(vDiagonal);

				XMVECTOR vBoarderOffset = (vDiagonal - (maxV - minV)) * XMVectorSet(0.5f, 0.5f, 0.5f, 0.5f);
				vBoarderOffset *= multiplySetzwToZeroV;

				maxV += vBoarderOffset;
				minV -= vBoarderOffset;

				FLOAT fCascadeBound = XMVectorGetX(vDiagonal);
				FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)mapSize;
				worldUnitsPerTexel = XMVectorSet(fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f);
			}

			float minZ = XMVectorGetZ(minV);
			float maxZ = XMVectorGetZ(maxV);

			if (canMoveLightTexelSize)
			{
				minV /= worldUnitsPerTexel;
				minV = XMVectorFloor(minV);
				minV *= worldUnitsPerTexel;

				maxV /= worldUnitsPerTexel;
				maxV = XMVectorFloor(maxV);
				maxV *= worldUnitsPerTexel;
			}

			outMin = minV;
			outMax = maxV;
			outMin.z = minZ;
			outMax.z = maxZ;
			return XMMatrixOrthographicOffCenterLH(XMVectorGetX(minV), XMVectorGetX(maxV),
				XMVectorGetY(minV), XMVectorGetY(maxV),
				minZ, maxZ);
		}
		static XMMATRIX CalProj(const BoundingBox& sceneBBox,
			const JMatrix4x4& view,
			const size_t mapSize)noexcept
		{
			JVector3<float> minV3;
			JVector3<float> maxV3;
			return CalProj(sceneBBox, view, mapSize, minV3, maxV3);
		}

		static JGraphicResourceTypeSet ShadowMapTypeSet() noexcept
		{
			return JGraphicResourceTypeSet(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		}
		static JGraphicResourceTypeSet ShadowMapArrayTypeSet() noexcept
		{
			return JGraphicResourceTypeSet(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		}
		static JGraphicResourceTypeSet DebugTypeSet() noexcept
		{
			return JGraphicResourceTypeSet(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
		}
	}

	class JDirectionalLight::JDirectionalLightImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JDirectionalLightImpl)
	private:
		enum MANAGED_SET
		{
			MANAGED_SET_CSM = 0,
			MANAGED_SET_SHADOW_MAP,
			MANAGED_SET_DISPLAY_SHADOW_MAP,		//for debugging
			MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP,
			MANAGED_SET_HZB_CULLING,
			MANAGED_SET_HD_CULLING,
			MANAGED_SET_COUNT
		};
		struct SetParam
		{
		public:
			bool value;
			bool isCalledByAct;
		public:
			SetParam(bool value, bool isCalledByAct)
				:value(value), isCalledByAct(isCalledByAct)
			{}
		};
		using ManageFuncList = Core::JFuncList<MANAGED_SET_COUNT, JDirectionalLight::JDirectionalLightImpl, SetParam>;
		using CONDTION_MASK = ManageFuncList::CONDITION_MASK;
	public:
		JWeakPtr<JDirectionalLight> thisPointer = nullptr;
		JUserPtr<JGraphicModuleManagedDataFrame> graphicData = nullptr;
	public:
		JVector3<float> vSceneBBoxMinF;
		JVector3<float> vSceneBBoxMaxF;
		JVector3<float> direction;
		JMatrix4x4 view;
		JMatrix4x4 proj;
	public:
		//REGISTER_PROPERTY_EX(onCsm, IsCsmActivated, SetCsm, GUI_CHECKBOX(GUI_BOOL_CONDITION_REF_USER(OnShadow, true)))
		REGISTER_GUI_BOOL_CONDITION(OnShadow, IsShadowActivated, true)
		REGISTER_GUI_BOOL_CONDITION(OnCsm, onCsm, false)
		REGISTER_PROPERTY_EX(onCsm, IsCsmActivated, SetCsm, GUI_CHECKBOX(GUI_BOOL_CONDITION_USER(OnShadow, true)))
		bool onCsm = false;
	public:
		bool allowDisplayOccCullingDepthMap = false;
		bool allowHzbOcclusionCulling = false;
		bool allowHdOcclusionCulling = false;
	public:
		JDirectionalLightImpl(const InitData& initData, JDirectionalLight* thisLitRaw)
		{
		}
		~JDirectionalLightImpl()
		{ }
	public:
		J_LIGHT_TYPE GetLightType()const noexcept
		{
			return J_LIGHT_TYPE::DIRECTIONAL;
		}
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept
		{
			if (!thisPointer->IsShadowActivated())
				return J_SHADOW_MAP_TYPE::NONE;

			if (CanAllocateCsm())
				return J_SHADOW_MAP_TYPE::CSM;
			else
				return J_SHADOW_MAP_TYPE::NORMAL;
		}
		float GetFrustumNear()const noexcept
		{
			JVector3<float> vMinF;
			JVector3<float> vMaxF;
			const XMMATRIX proj = Private::CalProj(GetSceneBBox(), view, thisPointer->GetShadowResolution(), vMinF, vMaxF);
			return vMinF.z;
		}
		float GetFrustumFar()const noexcept
		{
			JVector3<float> vMinF;
			JVector3<float> vMaxF;
			const XMMATRIX proj = Private::CalProj(GetSceneBBox(), view, thisPointer->GetShadowResolution(), vMinF, vMaxF);
			return vMaxF.z;
		}
		DirectX::BoundingBox GetBBox()const noexcept
		{
			const XMMATRIX viewM = view.LoadXM();
			const XMMATRIX inverseViewM = XMMatrixInverse(nullptr, viewM);

			XMFLOAT3 corner[8];
			GetSceneBBox().GetCorners(corner);

			XMVECTOR minV = JVector3<float>::PositiveInfV().ToXmV();
			XMVECTOR maxV = JVector3<float>::NegativeInfV().ToXmV();

			for (int i = 0; i < 8; ++i)
			{
				const XMVECTOR v = XMVector3Transform(XMLoadFloat3(&corner[i]), viewM);
				minV = XMVectorMin(minV, v);
				maxV = XMVectorMax(maxV, v);
			}

			//world 
			return Core::JDirectXCollisionEx::CreateBoundingBox(XMVector3Transform(minV, inverseViewM), XMVector3Transform(maxV, inverseViewM));
		}
		JUserPtr<JTransform> GetTransform()const noexcept
		{
			return thisPointer->GetOwner()->GetTransform();
		}
		BoundingBox GetSceneBBox()const noexcept
		{
			return thisPointer->GetOwner()->GetOwnerScene()->GetSceneBBox();
		}
		JUserPtr<JMeshGeometry> GetMesh()const noexcept
		{
			return _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_CUBE);
		}
		DirectX::XMMATRIX GetMeshWorldM(const bool restrictScaledZ)const noexcept
		{
			float s = Constants::globarLightMaxDistance;
			return DirectX::XMMatrixAffineTransformation(JVector4F(s, s, restrictScaledZ ? 1 : s, 0.0f).ToXmV(), JVector4F::Zero().ToXmV(), JVector4F::Zero().ToXmV(), JVector4F::Zero().ToXmV());
		}
		XMMATRIX GetShadowMapTransform()const noexcept
		{
			return XMMatrixMultiply(XMMatrixMultiply(view.LoadXM(), proj.LoadXM()), JMatrix4x4::NdcToTextureSpaceXM());
		}
		uint GetSplitCount()const noexcept
		{
			return graphicData->GetCsmHandleUserInterface()->GetOption().GetSplitCount();
		}
		float GetSplitBlendRate()const noexcept
		{
			return graphicData->GetCsmHandleUserInterface()->GetOption().GetSplitBlendRate();
		}
		float GetShadowDistance()const noexcept
		{
			return graphicData->GetCsmHandleUserInterface()->GetOption().GetShadowDistance();
		}
		float GetLevelBlendRate()const noexcept
		{
			return graphicData->GetCsmHandleUserInterface()->GetOption().GetLevelBlendRate();
		}
	public:
		void SetShadow(bool value)noexcept
		{
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_SHADOW_MAP, this, SetParam(value, false));
		}
		void SetShadowResolution(const J_SHADOW_RESOLUTION newShadowResolution)noexcept
		{
			if (thisPointer->IsActivated() && thisPointer->IsShadowActivated())
			{
				DestroyShadowMapResource();
				CreateShadowMapResource(CanAllocateCsm());
			}
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void SetCsm(bool value)noexcept
		{
			if (onCsm == value)
				return;

			onCsm = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_CSM, this, SetParam(value, false));
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void SetAllowDisplayShadowMap(bool value)
		{
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_SHADOW_MAP, this, SetParam(value, false));
		}
		void SetAllowDisplayOccCullingDepthMap(bool value)noexcept
		{
			if (allowDisplayOccCullingDepthMap == value)
				return;

			SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, this, SetParam(value, false));
		}
		void SetAllowHzbOcclusionCulling(bool value)noexcept
		{
			if (allowHzbOcclusionCulling == value)
				return;

			if (AllowHdOcclusionCulling())
				SetAllowHdOcclusionCulling(false);

			SetFuncList().InvokePassLocalCondition(MANAGED_SET_HZB_CULLING, this, SetParam(value, false));
		}
		void SetAllowHdOcclusionCulling(bool value)noexcept
		{
			if (allowHdOcclusionCulling == value)
				return;

			if (AllowHzbOcclusionCulling())
				SetAllowHzbOcclusionCulling(false);

			SetFuncList().InvokePassLocalCondition(MANAGED_SET_HD_CULLING, this, SetParam(value, false));
		}
		void SetSplitCount(const uint newCount)noexcept
		{
			graphicData->GetCsmHandleUserInterface()->GetOption().SetSplitCount(newCount);
			if (CanAllocateCsm())
			{
				DestroyShadowMapResource();
				CreateShadowMapResource(CanAllocateCsm());
			}
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void SetSplitBlendRate(const float value)noexcept
		{
			graphicData->GetCsmHandleUserInterface()->GetOption().SetSplitBlendRate(value);
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void SetShadowDistance(const float value)noexcept
		{
			graphicData->GetCsmHandleUserInterface()->GetOption().SetShadowDistance(std::clamp(value, Constants::minCamFrustumNear, Constants::maxCamFrustumFar));
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void SetLevelBlendRate(const float value)noexcept
		{
			graphicData->GetCsmHandleUserInterface()->GetOption().SetLevelBlendRate(value);
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		static ManageFuncList& SetFuncList()
		{
			static ManageFuncList setFuncList;
			return setFuncList;
		}
	public:
		REGISTER_METHOD_GUI_WIDGET(CsmSplitCount, GetSplitCount, SetSplitCount, GUI_SLIDER(JCsmOption::minCountOfSplit, JCsmOption::maxCountOfSplit, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
		REGISTER_METHOD_GUI_WIDGET(CsmSplitBlendRate, GetSplitBlendRate, SetSplitBlendRate, GUI_SLIDER(JCsmOption::minSplitRate, JCsmOption::maxSplitRate, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
		REGISTER_METHOD_GUI_WIDGET(CsmShadowDistance, GetShadowDistance, SetShadowDistance, GUI_SLIDER(Constants::minCamFrustumNear, Constants::maxCamFrustumFar, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
		REGISTER_METHOD_GUI_WIDGET(CsmLevelBlendRate, GetLevelBlendRate, SetLevelBlendRate, GUI_SLIDER(JCsmOption::minLevelRate, JCsmOption::maxLevelRate, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
	public:
		bool IsActivated()const noexcept
		{
			return thisPointer->IsActivated();
		}
		REGISTER_METHOD(IsShadowActivated)
		bool IsShadowActivated()const noexcept
		{
			return thisPointer->IsShadowActivated();
		}
		//return trigger value
		bool IsCsmActivated()const noexcept
		{
			return onCsm;
		}
		//return lit type is dirctional light
		bool CanUseCsm()const noexcept
		{
			return GetLightType() == J_LIGHT_TYPE::DIRECTIONAL;
		}
		//return IsCsmActivated() && CanUseCsm()
		bool CanAllocateCsm()const noexcept
		{
			return IsCsmActivated() && CanUseCsm() && graphicData->GetCsmHandleUserInterface()->GetTargetCount() > 0;
		}
		bool AllowDisplayShadowMap()const noexcept
		{
			return thisPointer->AllowDisplayShadowMap();
		}
		bool AllowDisplayOccCullingDepthMap()const noexcept
		{
			return allowDisplayOccCullingDepthMap;
		}
		bool AllowHzbOcclusionCulling()const noexcept
		{
			return allowHzbOcclusionCulling;
		}
		bool AllowHdOcclusionCulling()const noexcept
		{
			return allowHdOcclusionCulling;
		}
	private:
		void CreateShadowMapResource(const bool canAllocCsm)noexcept
		{
			if (canAllocCsm)
			{
				//has order dependency 
				//1. RegisterCsmHandlerface();
				//2. RegisterCsmFrameData

				const uint targetCount = graphicData->GetCsmHandleUserInterface()->GetTargetCount();
				const JCsmOption csmOption = graphicData->GetCsmHandleUserInterface()->GetOption();
				const uint width = thisPointer->GetShadowMapSize();
				const uint arrayCount = csmOption.GetSplitCount();

				JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW, thisPointer->GetAreaGuid(), targetCount);
				JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO, thisPointer->GetAreaGuid(), targetCount);

				JGraphicResourceTypeSet typeSet = Private::ShadowMapArrayTypeSet();
				JGraphicResourceCreationDesc desc(typeSet, width, width, arrayCount);

				for (uint i = 0; i < targetCount; ++i)
					GMI()->CreateGraphicResource(graphicData.Get(), desc);
			}
			else
			{
				JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW, thisPointer->GetAreaGuid());

				JGraphicResourceTypeSet typeSet = Private::ShadowMapTypeSet();
				JGraphicResourceCreationDesc desc(typeSet, JVector2F(thisPointer->GetShadowMapSize()));

				GMI()->CreateGraphicResource(graphicData.Get(), desc);
			}

			if (thisPointer->AllowDisplayShadowMap())
				CreateShadowMapDebugResource(canAllocCsm);
	
			JGraphicRequestCreationDesc desc(J_GRAPHIC_REQUEST_TYPE::DRAW_SHADOW_MAP, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED);
			GMI()->RequestExecutableGraphicFeature(graphicData.Get(), desc);
		}
		void DestroyShadowMapResource()noexcept
		{
			GMI()->CancelExecutableGraphicFeature(graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::DRAW_SHADOW_MAP);

			DestroyShadowMapDebugResource();

			JGraphicResourceTypeSet smSet = Private::ShadowMapTypeSet();
			JGraphicResourceTypeSet smaSet = Private::ShadowMapArrayTypeSet();

			GMI()->DestroyGraphicResource(graphicData.Get(), smSet);
			GMI()->DestroyAllGraphicsResourcesOfType(graphicData.Get(), smaSet.resouce);

			//GMI()->DestroyAllGraphicsResources(graphicData.Get());
			//DeRegisterCsmHandlerface();
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO);
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW);
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);
		};
		void CreateShadowMapDebugResource(const bool canAllocCsm)
		{
			auto csmUser = graphicData->GetCsmHandleUserInterface();
			auto gUser = graphicData->GetGraphicResourceUserInterface();
			 
			//already exist handle
			if (gUser->IsValidHandle(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE))
				return;

			JGraphicResourceCreationDesc desc(Private::DebugTypeSet(), JVector2F(thisPointer->GetShadowMapSize()));
			const uint debugResourceCount = canAllocCsm ? (csmUser->GetOption().GetSplitCount() * csmUser->GetTargetCount()) : 1;
			for (uint i = 0; i < debugResourceCount; ++i)
				GMI()->CreateGraphicResource(graphicData.Get(), desc);
		}
		void DestroyShadowMapDebugResource()
		{
			GMI()->DestroyAllGraphicsResourcesOfType(graphicData.Get(), Private::DebugTypeSet().resouce);
		}
	public:
		void Activate()
		{ 
			IMPL_REGISTER_FRAME_UPDATE_ACTION_HOT();

			if (AllowHdOcclusionCulling() && AllowHzbOcclusionCulling())
				allowHdOcclusionCulling = allowHzbOcclusionCulling = false;

			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT, thisPointer->GetAreaGuid());
			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, SetParam(true, true));

			JGMUtil::SetFrameDirty(graphicData.Get());
		}
		void DeActivate()
		{ 
			//has order dependency 
			SetFuncList().InvokeAllReverse(this, CONDTION_MASK::PASS_NONE, SetParam(false, true));
			GMI()->DestroyAllGraphicsResources(graphicData.Get());
			GMI()->DestroyAllCullingData(graphicData.Get());
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT);
			IMPL_DEREGISTER_FRAME_UPDATE_ACTION();
		}
	private:
		void HotUpdate()
		{
			UpdateLightTransform();
		}
		void UpdateLightTransform()noexcept
		{
			const XMMATRIX viewM = Private::CalView(GetTransform());
			view.StoreXM(viewM);
			proj.StoreXM(Private::CalProj(GetSceneBBox(), view, thisPointer->GetShadowResolution(), vSceneBBoxMinF, vSceneBBoxMaxF));
			direction = Private::CalLightWorldDir(GetTransform());

			if (CanAllocateCsm())
				graphicData->GetCsmHandleUserInterface()->Update(viewM, GetSceneBBox(), thisPointer->GetShadowMapSize());
		}
		void UpdateCsmTargetCount(const uint index, const bool isRegister)
		{
			//can alloc csm = true
			auto csmUser = graphicData->GetCsmHandleUserInterface();
			auto gUser = graphicData->GetGraphicResourceUserInterface();

			const int smArrayCount = gUser->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY);
			const int targetCount = csmUser->GetTargetCount();
			const bool isSameCount = smArrayCount == targetCount;

			//DestroyShadowMapResource();
			//CreateShadowMapResource(CanAllocateCsm());

			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW);
			GMI()->DestroyFrameUploadData(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO);

			//Reflect new target count
			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW, thisPointer->GetAreaGuid(), targetCount);
			JGMUtil::CreateFrame(graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO, thisPointer->GetAreaGuid(), targetCount);

			JGraphicResourceTypeSet smaTypeSet = Private::ShadowMapArrayTypeSet();
			JGraphicResourceTypeSet debugTypeSet = Private::DebugTypeSet();
			if (isRegister)
			{
				//add case
				if (!isSameCount)
				{
					if (smArrayCount == 0 && targetCount == 1)
					{
						DestroyShadowMapResource();
						CreateShadowMapResource(CanAllocateCsm());
					}
					else
					{
						const JCsmOption csmOption = graphicData->GetCsmHandleUserInterface()->GetOption();
						const uint width = thisPointer->GetShadowMapSize();
						const uint arrayCount = csmOption.GetSplitCount();

						//add one target
						JGraphicResourceCreationDesc desc(smaTypeSet, width, width, arrayCount);
						GMI()->CreateGraphicResource(graphicData.Get(), desc);

						//add one debugMap
						if (thisPointer->AllowDisplayShadowMap())
						{
							JGraphicResourceCreationDesc debugDesc(debugTypeSet, JVector2F(thisPointer->GetShadowMapSize()));
							GMI()->CreateGraphicResource(graphicData.Get(), debugDesc, csmUser->GetOption().GetSplitCount());
						}
					}
				}
			}
			else if (!isSameCount)
			{
				//subtract case
				const uint debugResourceIndex = csmUser->GetOption().GetSplitCount() * index;
				GMI()->DestroyGraphicResource(graphicData.Get(), smaTypeSet, index);
				if (thisPointer->AllowDisplayShadowMap())
					GMI()->DestroyGraphicResource(graphicData.Get(), J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, debugResourceIndex, csmUser->GetOption().GetSplitCount());
			}
			JGMUtil::SetFrameDirty(graphicData.Get());
		}
	public:
		static bool DoCopy(JDirectionalLight* from, JDirectionalLight* to)
		{
			from->SetCsm(to->IsCsmActivated());
			from->impl->SetAllowHzbOcclusionCulling(to->impl->AllowHzbOcclusionCulling());
			from->impl->SetAllowHdOcclusionCulling(to->impl->AllowHdOcclusionCulling());
			from->impl->SetAllowDisplayOccCullingDepthMap(to->impl->AllowDisplayOccCullingDepthMap());

			JGMUtil::SetFrameDirty(to->impl->graphicData.Get());
			return true;
		}
	public:
		void NotifyReAlloc()
		{}
	public:
		void RegisterThisPointer(JDirectionalLight* lit)
		{
			thisPointer = Core::GetWeakPtr(lit);
		}
		void RegisterPostCreation()
		{
		}
		void DeRegisterPreDestruction()
		{
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JDirectionalLight::StaticTypeInfo(), Private::instance);
			IMPL_REALLOC_BIND()
			SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT_TO_CHILD);

			auto setCsmLam = [](JDirectionalLightImpl* impl, SetParam p)
			{
				if (p.value)
				{
					auto notifyAddLam = [](JUserPtr<JObject> obj, int index)
					{
						static_cast<JDirectionalLight*>(obj.Get())->impl->UpdateCsmTargetCount(index, true);
					};
					auto notifySubtractLam = [](JUserPtr<JObject> obj, int index)
					{
						static_cast<JDirectionalLight*>(obj.Get())->impl->UpdateCsmTargetCount(index, false);
					};

					NotifyAddCsmTargetF::Ptr addPtr = notifyAddLam;
					NotifySubtractCsmTargetF::Ptr subtractPtr = notifySubtractLam;

					JCsmHandleCreationDesc desc(impl->thisPointer->GetGuid(), impl->thisPointer->GetOwner()->GetOwnerGuid());
					desc.notifyAddCsmTargetB = Core::UniqueBind(addPtr, JUserPtr<JObject>(impl->thisPointer), Core::JEmptyType());
					desc.notifySubtractCsmTargetB = Core::UniqueBind(subtractPtr, JUserPtr<JObject>(impl->thisPointer), Core::JEmptyType());

					GMI()->CreateCsmHandler(impl->graphicData.Get(), desc);
				}
				else
					GMI()->DestroyCsmHandler(impl->graphicData.Get());

				//has order dependency in activate processs
				//csm -> shadow map
				if (impl->IsShadowActivated() && !p.isCalledByAct)
				{
					impl->DestroyShadowMapResource();
					impl->CreateShadowMapResource(impl->CanAllocateCsm());
				}
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setShadowMapLam = [](JDirectionalLightImpl* impl, SetParam p)
			{
				auto gUser = impl->graphicData->GetGraphicResourceUserInterface();
				JGraphicResourceTypeSet smSet = Private::ShadowMapTypeSet();
				JGraphicResourceTypeSet smaSet = Private::ShadowMapArrayTypeSet();

				if (p.value)
				{
					if (gUser->IsValidHandle(smSet.resouce, smSet.task) || gUser->IsValidHandle(smaSet.resouce, smaSet.task))
						return;

					impl->CreateShadowMapResource(impl->CanAllocateCsm());
				}
				else
				{
					if (!gUser->IsValidHandle(smSet.resouce, smSet.task) && !gUser->IsValidHandle(smaSet.resouce, smaSet.task))
						return;

					impl->DestroyShadowMapResource();
				}
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setDisplayShadowMapLam = [](JDirectionalLightImpl* impl, SetParam p)
			{
				if (p.value)
					impl->CreateShadowMapDebugResource(impl->CanAllocateCsm());
				else
					impl->DestroyShadowMapDebugResource();
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setDisplayOccDepthMapLam = [](JDirectionalLightImpl* impl, SetParam p)
			{
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
				if (p.value)
				{
					const bool hasHandle = impl->graphicData.Get()->GetGraphicResourceUserInterface()->IsValidHandle(typeSet.resouce, typeSet.task);
					if (hasHandle)
						return;

					JGraphicResourceCreationDesc desc(typeSet);
					desc.useEngineDefine = true;
					desc.bindDesc.allowMipmapBind = impl->allowHzbOcclusionCulling;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), desc);
				}
				else
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setHzbCullingLam = [](JDirectionalLightImpl* impl, SetParam p)
			{
				const JCullingTypeSet cullingTypeSet(J_CULLING_TYPE::HZB_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
				if (p.value)
				{
					JGMUtil::CreateFrame(impl->graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS, impl->thisPointer->GetAreaGuid());
					JGMUtil::CreateFrame(impl->graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS, impl->thisPointer->GetAreaGuid());

					JGraphicResourceTypeSet typeSet;
					JGraphicResourceCreationDesc desc(typeSet);
					desc.useEngineDefine = true;

					desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP;
					desc.type.task = J_GRAPHIC_TASK_TYPE::HZB_CULLING;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), desc);

					desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
					desc.type.task = J_GRAPHIC_TASK_TYPE::HZB_CULLING;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), desc);

					GMI()->CreateCullingData(impl->graphicData.Get(), cullingTypeSet);

					JGraphicRequestCreationDesc requestDesc(J_GRAPHIC_REQUEST_TYPE::HZB_OCCLUSION_CULLING, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED);
					GMI()->RequestExecutableGraphicFeature(impl->graphicData.Get(), requestDesc);

					auto gUser = impl->graphicData.Get()->GetGraphicResourceUserInterface();

					const uint mipSrvCount = gUser->GetViewCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_TASK_TYPE::HZB_CULLING);
					const uint debugSrvCount = gUser->GetViewCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
					const bool hasDebug = debugSrvCount != 0;

					if (!hasDebug)
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
					else if (mipSrvCount != debugSrvCount)
					{
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
					}
				}
				else
				{

					GMI()->CancelExecutableGraphicFeature(impl->graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::HZB_OCCLUSION_CULLING);
					GMI()->DestroyCullingData(impl->graphicData.Get(), cullingTypeSet);

					JGraphicResourceTypeSet typeSet;
					typeSet.resouce = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
					typeSet.task = J_GRAPHIC_TASK_TYPE::HZB_CULLING;
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);

					typeSet.resouce = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP;
					typeSet.task = J_GRAPHIC_TASK_TYPE::HZB_CULLING;
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);

					GMI()->DestroyFrameUploadData(impl->graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS);
					if (p.isCalledByAct || (!impl->AllowHzbOcclusionCulling() && !impl->AllowHdOcclusionCulling()))
						GMI()->DestroyFrameUploadData(impl->graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS);

					if (!impl->AllowHdOcclusionCulling())
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
				}
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			auto setHdCullingLam = [](JDirectionalLightImpl* impl, SetParam p)
			{
				const JCullingTypeSet cullingTypeSet(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
				if (p.value)
				{
					JGMUtil::CreateFrame(impl->graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS, impl->thisPointer->GetAreaGuid());
					GMI()->CreateCullingData(impl->graphicData.Get(), cullingTypeSet);

					JGraphicRequestCreationDesc requestDesc(J_GRAPHIC_REQUEST_TYPE::HARD_WARE_OCCLUSION_CULLING, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED);
					GMI()->RequestExecutableGraphicFeature(impl->graphicData.Get(), requestDesc);

					if (impl->AllowDisplayOccCullingDepthMap())
					{
						auto gUser = impl->graphicData->GetGraphicResourceUserInterface();
						const uint debugSrvCount = gUser->GetViewCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
						const bool hasDebug = debugSrvCount != 0;

						if (!hasDebug)
							impl->SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
						else if (debugSrvCount > 1)
						{
							//이전 hzb occ를 사용했고 debug map이 남아있을 경우 재생성
							impl->SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
							impl->SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
						}
					}
				}
				else
				{
					GMI()->CancelExecutableGraphicFeature(impl->graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::HARD_WARE_OCCLUSION_CULLING);
					GMI()->DestroyCullingData(impl->graphicData.Get(), cullingTypeSet);

					if (p.isCalledByAct || (!impl->AllowHzbOcclusionCulling() && !impl->AllowHdOcclusionCulling()))
						GMI()->DestroyFrameUploadData(impl->graphicData.Get(), J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS);
					if (!impl->AllowHzbOcclusionCulling())
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
				}
				JGMUtil::SetFrameDirty(impl->graphicData.Get());
			};
			using SetCallable = Core::JStaticCallable<void, JDirectionalLightImpl*, SetParam>;
			using CondCallable = Core::JMemberCNCallable<JDirectionalLightImpl, bool>;

			SetFuncList().Register(std::make_unique<SetCallable>(setCsmLam), std::make_unique<CondCallable>(&JDirectionalLightImpl::IsCsmActivated), MANAGED_SET_CSM);
			SetFuncList().Register(std::make_unique<SetCallable>(setShadowMapLam), std::make_unique<CondCallable>(&JDirectionalLightImpl::IsShadowActivated), MANAGED_SET_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayShadowMapLam), std::make_unique<CondCallable>(&JDirectionalLightImpl::AllowDisplayShadowMap), MANAGED_SET_DISPLAY_SHADOW_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayOccDepthMapLam), std::make_unique<CondCallable>(&JDirectionalLightImpl::AllowDisplayOccCullingDepthMap), MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setHzbCullingLam), std::make_unique<CondCallable>(&JDirectionalLightImpl::AllowHzbOcclusionCulling), MANAGED_SET_HZB_CULLING);
			SetFuncList().Register(std::make_unique<SetCallable>(setHdCullingLam), std::make_unique<CondCallable>(&JDirectionalLightImpl::AllowHdOcclusionCulling), MANAGED_SET_HD_CULLING);
			SetFuncList().RegisterGlobalCond(std::make_unique<CondCallable>(&JDirectionalLightImpl::IsActivated));
		}
	};

	JDirectionalLight::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JLight::InitData(JDirectionalLight::StaticTypeInfo(), owner)
	{}
	JDirectionalLight::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JLight::InitData(JDirectionalLight::StaticTypeInfo(), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JDirectionalLight::PrivateInterface()const noexcept
	{
		return Private::instance;
	}
	JGraphicModuleManagedDataFrame* JDirectionalLight::ModuleManagedData()const noexcept
	{
		return impl->graphicData.Get();
	}
	J_LIGHT_TYPE JDirectionalLight::GetLightType()const noexcept
	{
		return impl->GetLightType();
	}
	J_SHADOW_MAP_TYPE JDirectionalLight::GetShadowMapType()const noexcept
	{
		return impl->GetShadowMapType();
	}
	float JDirectionalLight::GetMinPower()const noexcept
	{
		return Private::minPower;
	}
	float JDirectionalLight::GetMaxPower()const noexcept
	{
		return Private::maxPower;
	}
	float JDirectionalLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JDirectionalLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	} 
	JVector3F JDirectionalLight::GetInitWorldDirection()const noexcept
	{
		return Private::GetInitDir();
	}
	JVector3F JDirectionalLight::GetWorldDirection()const noexcept
	{
		return Private::CalLightWorldDir(impl->GetTransform());
	}
	JVector3F JDirectionalLight::GetCachedWorldDirection()const noexcept
	{
		return impl->direction;
	}
	JVector3F JDirectionalLight::GetFrustumMinPoint()const noexcept
	{
		return impl->vSceneBBoxMinF;
	}
	JVector3F JDirectionalLight::GetFrustumMaxPoint()const noexcept
	{
		return impl->vSceneBBoxMaxF;
	}
	DirectX::BoundingBox JDirectionalLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
	}
	JUserPtr<JMeshGeometry> JDirectionalLight::GetMesh()const noexcept
	{
		return impl->GetMesh();
	}
	DirectX::XMMATRIX JDirectionalLight::GetMeshWorldM(const bool restrictScaledZ)const noexcept
	{
		return impl->GetMeshWorldM(restrictScaledZ);
	}
	DirectX::XMMATRIX JDirectionalLight::GetShadowMapTransform()const noexcept
	{
		return impl->GetShadowMapTransform();
	}
	JMatrix4x4 JDirectionalLight::GetView()const noexcept
	{
		return impl->view;
	}
	JMatrix4x4 JDirectionalLight::GetProj()const noexcept
	{
		return impl->proj;
	}
	uint JDirectionalLight::GetCsmSplitCount()const noexcept
	{
		return impl->GetSplitCount();
	}
	float JDirectionalLight::GetCsmSplitBlendRate()const noexcept
	{
		return impl->GetSplitBlendRate();
	}
	float JDirectionalLight::GetCsmShadowDistance()const noexcept
	{
		return impl->GetShadowDistance();
	}
	float JDirectionalLight::GetCsmLevelBlendRate()const noexcept
	{
		return impl->GetLevelBlendRate();
	}
	void JDirectionalLight::SetShadow(const bool value)noexcept
	{
		if (value == IsShadowActivated())
			return;

		JLight::SetShadow(value);
		impl->SetShadow(value);
	}
	void JDirectionalLight::SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept
	{
		if (sQuality == GetShadowResolutionType())
			return;

		JLight::SetShadowResolution(sQuality);
		impl->SetShadowResolution(sQuality);
	}
	void JDirectionalLight::SetAllowDisplayShadowMap(const bool value)noexcept
	{
		if (value == AllowDisplayShadowMap())
			return;

		JLight::SetAllowDisplayShadowMap(value);
		impl->SetAllowDisplayShadowMap(value);
	}
	void JDirectionalLight::SetCsm(const bool value)noexcept
	{
		impl->SetCsm(value);
	}
	void JDirectionalLight::SetCsmSplitCount(const uint newCount)noexcept
	{
		impl->SetSplitCount(newCount);
	}
	void JDirectionalLight::SetCsmSplitBlendRate(const float value)noexcept
	{
		impl->SetSplitBlendRate(value);
	}
	void JDirectionalLight::SetCsmShadowDistance(const float value)noexcept
	{
		impl->SetShadowDistance(value);
	}
	void JDirectionalLight::SetCsmLevelBlendRate(const float value)noexcept
	{
		impl->SetLevelBlendRate(value);
	}
	bool JDirectionalLight::IsCsmActivated()const noexcept
	{
		return impl->IsCsmActivated();
	}
	bool JDirectionalLight::CanAllocateCsm()const noexcept
	{
		return impl->CanAllocateCsm();
	}
	bool JDirectionalLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	bool JDirectionalLight::AllowFrustumCulling()const noexcept
	{
		return false;
	}
	bool JDirectionalLight::AllowHzbOcclusionCulling()const noexcept
	{
		return impl->AllowHzbOcclusionCulling();
	}
	bool JDirectionalLight::AllowHdOcclusionCulling()const noexcept
	{
		return impl->AllowHdOcclusionCulling();
	}
	bool JDirectionalLight::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
	}
	void JDirectionalLight::DoActivate()noexcept
	{
		//Caution 
		//Activate와 RegisterComponent는 순서에 종속성을 가진다.
		//RegisterComponent는 Scene과 가속구조에 Component에 대한 정보를 추가하는 작업으로
		//Activate Process중에 자기자신과 관련된 Scene component vector, Scene As관련 data에 대한 호출은 에러를 일으킬 수 있다.
		impl->graphicData = GraphicModuleInterface()->Allocate(impl->thisPointer);
		JLight::DoActivate();
		impl->Activate();
		RegisterComponent(impl->thisPointer, GetLitTypeComparePtr());
	}
	void JDirectionalLight::DoDeActivate()noexcept
	{
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate();
		JLight::DoDeActivate();
		GraphicModuleInterface()->DeAllocate(impl->graphicData);
	}
	JDirectionalLight::JDirectionalLight(const InitData& initData)
		:JLight(initData), impl(std::make_unique<JDirectionalLightImpl>(initData, this))
	{ }
	JDirectionalLight::~JDirectionalLight()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JDirectionalLightPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JDirectionalLightPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JDirectionalLightPrivate::AssetDataIOInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JDirectionalLight>(*static_cast<JDirectionalLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JDirectionalLight* lit = static_cast<JDirectionalLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit);
		lit->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JDirectionalLight::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JDirectionalLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		if (!JLightPrivate::CreateInstanceInterface::Copy(from, to))
			return false;

		return JDirectionalLight::JDirectionalLightImpl::DoCopy(static_cast<JDirectionalLight*>(from.Get()), static_cast<JDirectionalLight*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JDirectionalLight*>(ptr)->impl->DeRegisterPreDestruction();
		JLightPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectionalLight::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid = 0;
		J_OBJECT_FLAG flag = OBJECT_FLAG_NONE;
		bool isActivated = false;
		bool sOnCsm = false;
		float sSplitBlendRate = 0;
		uint sSplitCount = 0;
		float sShadowDistance = 0;
		float sLevelBlendRate = 0;
		float sPenumbraWidth = 0;
		float sPenumbraBlockerScale = 0;

		auto loadData = static_cast<JDirectionalLight::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JCsmOption csmOption;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto idenUser = Private::instance.GetCreateInstanceInterface().BeginCreate(std::make_unique<JDirectionalLight::InitData>(guid, flag, owner), &Private::instance);
		JUserPtr<JDirectionalLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(tool, litUser);
		JObjectFileIOHelper::LoadAtomicData(tool, sOnCsm, "OnCsm:");
		JObjectFileIOHelper::LoadAtomicData(tool, sSplitBlendRate, "CsmSplitBlendRate:");
		JObjectFileIOHelper::LoadAtomicData(tool, sSplitCount, "CsmSplitCount:");
		JObjectFileIOHelper::LoadAtomicData(tool, sShadowDistance, "CsmShadowDistance:");
		JObjectFileIOHelper::LoadAtomicData(tool, sLevelBlendRate, "CsmLevelBlendRate:");

		litUser->SetCsm(sOnCsm);
		litUser->impl->SetSplitBlendRate(sSplitBlendRate);
		litUser->impl->SetSplitCount(sSplitCount);
		litUser->impl->SetShadowDistance(sShadowDistance);
		litUser->impl->SetLevelBlendRate(sLevelBlendRate);
		if (!isActivated)
			litUser->DeActivate();

		return litUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectionalLight::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JDirectionalLight::StoreData*>(data);
		if (!storeData->HasCorrectType(JDirectionalLight::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JDirectionalLight> lit;
		lit.ConnnectChild(storeData->obj);

		JDirectionalLight::JDirectionalLightImpl* impl = lit->impl.get();
		JFileIOTool& tool = storeData->tool;
		auto option = impl->graphicData->GetCsmHandleUserInterface()->GetOption();

		JObjectFileIOHelper::StoreComponentIden(tool, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(tool, lit);
		JObjectFileIOHelper::StoreAtomicData(tool, impl->onCsm, "OnCsm:");
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetSplitBlendRate(), "CsmSplitBlendRate:");
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetSplitCount(), "CsmSplitCount:");
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetShadowDistance(), "CsmShadowDistance:");
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetLevelBlendRate(), "CsmLevelBlendRate:");
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JDirectionalLightPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JDirectionalLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JDirectionalLightPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}