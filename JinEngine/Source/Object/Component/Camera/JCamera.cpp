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


#include"JCamera.h"
#include"JCameraPrivate.h"
#include"JCameraConstants.h"
#include"../JComponentHint.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../Light/JLightConstants.h"
#include"../../JObjectTypeStatistics.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h"  
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"  
#include"../../GraphicRule/JGraphicModuleInterfaceHolder.h"
#include"../../GraphicRule/JGraphicModuleMacro.h"
#include"../../GraphicRule/GraphicResource/JGraphicModuleTextureResourceType.h"
#include"../../../Core/Guid/JGuidCreator.h"  
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Core/Func/JFuncList.h"    
#include"../../../Window/JWindow.h"  
#include<fstream>

//JCsmTargetInterface
//#include"../../../Develop/Debug/JDevelopDebug.h"

using namespace DirectX;
namespace JinEngine
{
	namespace
	{
		using WindowEventListener = Core::JEventListener<size_t, Window::J_WINDOW_EVENT>;
	}
	namespace Private
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JCameraPrivate instance;

		static constexpr float minSsaoRadius = 0.01f;
		static constexpr float maxSsaoRadius = 32.0f;
		static constexpr float minSsaoBias = -maxSsaoRadius;
		static constexpr float maxSsaoBias = maxSsaoRadius; ;
		static constexpr float minSsaoSharpness = 0.01f;
		static constexpr float maxSsaoSharpness = maxSsaoRadius;

		static constexpr float minSsaoAoScale = 0.0f;
		static constexpr float maxSsaoAoScale = 1.25f;

		static constexpr uint minSsaoBlurRadius = 0;
		static constexpr uint maxSsaoBlurRadius = (uint)J_KERNEL_SIZE::COUNT;

		static constexpr float cullingUpdateFrequencyMin = 0.0f;
		static constexpr float cullingUpdateFrequencyMax = 1.0f;
	}

	class JCamera::JCameraImpl : public Core::JTypeImplBase, public WindowEventListener
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JCameraImpl)
		REGISTER_GUI_BOOL_CONDITION(IsOrthoCam, isOrtho, false)
	public:
		//manage set func releated graphic resource
		enum MANAGED_SET
		{
			MANAGED_SET_MAIN_RENDER_TARGET = 0,
			MANAGED_SET_MAIN_DEPTH_STENCIL,
			MANAGED_SET_DISPLAY_RENDER_RESULT,
			MANAGED_SET_DISPLAY_DEBUG_OBJECT,
			MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP,
			MANAGED_SET_DISPLAY_LIGHT_CULLING_DEBUG,
			MANAGED_SET_FURSTUM_CULLING,
			MANAGED_SET_HZB_CULLING,
			MANAGED_SET_HD_CULLING,
			MANAGED_SET_LIGHT_CULLING,
			MANAGED_SET_SSAO,
			MANAGED_SET_IMAGE_PROCESSING,
			MANAGED_SET_GI,
			MANAGED_SET_DEFERRED_RESOURCE,
			MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE,
			MANAGED_SET_COUNT
		};
		enum GROUP_SET
		{
			GROUP_SET_CLIENT_SIZE_DEPENDENCY,
			GROUP_SET_COUNT
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
		using ManageFuncList = Core::JFuncListG<MANAGED_SET_COUNT, GROUP_SET_COUNT, JCamera::JCameraImpl, SetParam>;
		using CONDTION_MASK = ManageFuncList::CONDITION_MASK;
	public:
		JWeakPtr<JCamera> thisPointer = nullptr;
	public:
		JFastPtr<JGraphicModuleManagedDataFrame> graphicData = nullptr;
	public:
		// Cache View/Proj matrices.
		JMatrix4x4 mView;
		JMatrix4x4 mProj;
		JMatrix4x4 mPreView;
		JMatrix4x4 mPreViewProj;
	public:
		DirectX::BoundingFrustum mCamFrustum;
	public:
		JVector2F uvToViewA = JVector2F::Zero();
		JVector2F uvToViewB = JVector2F::Zero();
		JVector2F preUvToViewA = JVector2F::Zero();
		JVector2F preUvToViewB = JVector2F::Zero();
		float tanHalfFovX = 0;
		float tanHalfFovY = 0;
	public:
		JVector2F rtSizeRate = JVector2F::One();
	public:
		//JTransform* ownerTransform;
		J_CAMERA_STATE camState = J_CAMERA_STATE::RENDER;
		REGISTER_GUI_GROUP(camera)
		// Cache frustum properties.
		REGISTER_PROPERTY_EX(camNear, GetNear, SetNear, GUI_SLIDER(Constants::minCamFrustumNear, Constants::maxCamFrustumFar, true))
		float camNear = 0.0f;
		REGISTER_PROPERTY_EX(camFar, GetFar, SetFar, GUI_SLIDER(Constants::minCamFrustumNear, Constants::maxCamFrustumFar, true))
		float camFar = 0.0f;
		REGISTER_PROPERTY_EX(camAspect, GetAspect, SetAspect, GUI_SLIDER(0.1f, 32.0f, true, false, 3, GUI_BOOL_CONDITION_USER(IsOrthoCam, false)))
		float camAspect = 0.0f;		// Perspective일때 사용
		REGISTER_PROPERTY_EX(camFov, GetFovYDegree, SetFovYDegree, GUI_SLIDER(1, 185, true))
		float camFov = 0.0f;		//fovY
	public:
		REGISTER_PROPERTY_EX(camOrthoViewWidth, GetOrthoViewWidth, SetOrthoViewWidth, GUI_SLIDER(1, 3840, true, false, 3, GUI_BOOL_CONDITION_USER(IsOrthoCam, true)))
		float camOrthoViewWidth = 0.0f;		// Ortho일때 사용
		REGISTER_PROPERTY_EX(camOrthoViewHeight, GetOrthoViewHeight, SetOrthoViewHeight, GUI_SLIDER(1, 2160, true, false, 3, GUI_BOOL_CONDITION_USER(IsOrthoCam, true)))
		float camOrthoViewHeight = 0.0f;	// Ortho일때 사용
		REGISTER_GET_METHOD_EX(camFarViewWidth, GetFarViewWidth, GUI_READONLY_TEXT())
		REGISTER_GET_METHOD_EX(camFarViewHeight, GetFarViewHeight, GUI_READONLY_TEXT())
		float camNearViewHeight = 0.0f;
		float camFarViewHeight = 0.0f;
	public:
		//Culling Option
		//REGISTER_PROPERTY_EX(frustumCulingFrequency, GetFrustumCullingFrequency, SetFrustumCullingFrequency, GUI_SLIDER(Private::cullingUpdateFrequencyMin, Private::cullingUpdateFrequencyMax, true))
		//float frustumCulingFrequency = 0;
		//REGISTER_PROPERTY_EX(occlusionCulingFrequency, GetOcclusionCullingFrequency, SetOcclusionCullingFrequency, GUI_SLIDER(Private::cullingUpdateFrequencyMin, Private::cullingUpdateFrequencyMax, true))
		//float occlusionCulingFrequency = 0;
	public:
		REGISTER_GUI_GROUP(Ssao)
		REGISTER_GET_SET_METHOD_EX(SsaoRadius, GetSsaoRadius, SetSsaoRadius, GUI_SLIDER(Private::minSsaoRadius, Private::maxSsaoRadius, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_GET_SET_METHOD_EX(SsaoBias, GetSsaoBias, SetSsaoBias, GUI_SLIDER(Private::minSsaoBias, Private::maxSsaoBias, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_GET_SET_METHOD_EX(SsaoSharpness, GetSsaoSharpness, SetSsaoSharpness, GUI_SLIDER(Private::minSsaoSharpness, Private::maxSsaoSharpness, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_GET_SET_METHOD_EX(SsaoSmallAoScale, GetSsaoSmallAoScale, SetSsaoSmallAoScale, GUI_SLIDER(Private::minSsaoAoScale, Private::maxSsaoAoScale, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_GET_SET_METHOD_EX(SsaoLargeAoScale, GetSsaoLargeAoScale, SetSsaoLargeAoScale, GUI_SLIDER(Private::minSsaoAoScale, Private::maxSsaoAoScale, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_GET_SET_METHOD_EX(SsaoType, GetSsaoType, SetSsaoType, GUI_ENUM_COMBO(J_SSAO_TYPE, "", GUI_GROUP_USER(Ssao)))
		REGISTER_GET_SET_METHOD_EX(SsaoSample, GetSsaoSampleType, SetSsaoSampleType, GUI_ENUM_COMBO(J_SSAO_SAMPLE_TYPE, "", GUI_GROUP_USER(Ssao)))
		REGISTER_GET_SET_METHOD_EX(SsaoBlurRadius, GetSsaoBlurRadius, SetSsaoBlurRadius, GUI_SLIDER(Private::minSsaoBlurRadius, Private::maxSsaoBlurRadius, false, false, 1, GUI_GROUP_USER(Ssao)))
			JSsaoDesc ssaoDesc;
	public:
		REGISTER_PROPERTY_EX(isOrtho, IsOrthoCamera, SetOrthoCamera, GUI_CHECKBOX());
		bool isOrtho = false;
		REGISTER_PROPERTY_EX(allowDisplayRs, AllowDisplayRenderResult, SetAllowDisplayRenderResult, GUI_CHECKBOX())
		bool allowDisplayRs = false;
		REGISTER_PROPERTY_EX(allowDisplayDebugObject, AllowDisplayDebugObject, SetAllowDisplayDebugObject, GUI_CHECKBOX())
		bool allowDisplayDebugObject = false;
		REGISTER_PROPERTY_EX(allowDisplayOccCullingDepthMap, AllowDisplayOccCullingDepthMap, SetAllowDisplayOccCullingDepthMap, GUI_CHECKBOX())
		bool allowDisplayOccCullingDepthMap = false;
		REGISTER_PROPERTY_EX(allowDisplayLightCullingDebug, AllowDisplayLightCullingDebug, SetAllowDisplayLightCullingDebugging, GUI_CHECKBOX())
		bool allowDisplayLightCullingDebug = false;
		REGISTER_PROPERTY_EX(allowFrustumCulling, AllowFrustumCulling, SetAllowFrustumCulling, GUI_CHECKBOX())
		bool allowFrustumCulling = false;
		REGISTER_PROPERTY_EX(allowHzbOcclusionCulling, AllowHzbOcclusionCulling, SetAllowHzbOcclusionCulling, GUI_CHECKBOX())
		bool allowHzbOcclusionCulling = false;
		REGISTER_PROPERTY_EX(allowHdOcclusionCulling, AllowHdOcclusionCulling, SetAllowHdOcclusionCulling, GUI_CHECKBOX())
		bool allowHdOcclusionCulling = false;
		//REGISTER_PROPERTY_EX(allowLightCulling, AllowLightCulling, SetAllowLightCulling, GUI_CHECKBOX())
		//bool allowLightCulling = false;
		bool allowReflectAllCullResult = false;	//use editor cam for check space spatial result
		REGISTER_PROPERTY_EX(allowSsao, AllowSsao, SetAllowSsao, GUI_CHECKBOX())
		bool allowSsao = false;
	public:
		//Caution
		//Impl생성자에서 interface class 참조시 interface class가 함수내에서 impl을 참조할 경우 error
		//impl이 아직 생성되지 않았으므로
		JCameraImpl(const InitData& initData, JCamera* thisCamRaw)
		{
			rtSizeRate = initData.rtSizeRate;
		}
		~JCameraImpl()
		{}
	public:
		float GetNear()const noexcept
		{
			return camNear;
		}
		float GetFar()const noexcept
		{
			return camFar;
		}
		float GetFovX()const noexcept
		{
			float halfWidth = 0.5f * GetNearViewWidth();
			return 2.0f * atan(halfWidth / camNear);
		}
		float GetFovXDegree()const noexcept
		{
			return JMathHelper::RadToDeg * GetFovX();
		}
		float GetFovY()const noexcept
		{
			return camFov;
		}
		float GetFovYDegree()const noexcept
		{
			return JMathHelper::RadToDeg * camFov;
		}
		float GetAspect()const noexcept
		{
			return camAspect;
		}
		float GetNearViewWidth()const noexcept
		{
			return camAspect * camNearViewHeight;
		}
		float GetNearViewHeight()const noexcept
		{
			return camNearViewHeight;
		}
		float GetFarViewWidth()const noexcept
		{
			return camAspect * camFarViewHeight;
		}
		float GetFarViewHeight()const noexcept
		{
			return camFarViewHeight;
		}
		float GetOrthoViewWidth()const noexcept
		{
			return camOrthoViewWidth;
		}
		float GetOrthoViewHeight()const noexcept
		{
			return camOrthoViewHeight;
		}
		float GetRenderViewWidth()const noexcept
		{
			return isOrtho ? camOrthoViewWidth : GetFarViewWidth();
		}
		float GetRenderViewHeight()const noexcept
		{
			return isOrtho ? camOrthoViewHeight : GetFarViewHeight();
		}
		/*
		float GetFrustumCullingFrequency()const noexcept
		{
			return frustumCulingFrequency;
		}
		float GetOcclusionCullingFrequency()const noexcept
		{
			return occlusionCulingFrequency;
		}
		*/
		DirectX::BoundingFrustum GetBoundingFrustum()const noexcept
		{
			DirectX::BoundingFrustum worldCamFrustum;
			mCamFrustum.Transform(worldCamFrustum, thisPointer->GetOwner()->GetTransform()->GetWorldMatrix().LoadXM());
			return worldCamFrustum;
		}
		JVector2F GetRtSize()const noexcept
		{
			JVector2<uint> rtSize = JWindow::GetClientSize() * rtSizeRate;
			rtSize.x = JMathHelper::AlignT(rtSize.x, 8);
			rtSize.y = JMathHelper::AlignT(rtSize.y, 8);
			return rtSize;
		}
		float GetSsaoRadius()const noexcept
		{
			return ssaoDesc.radius;
		}
		float GetSsaoBias()const noexcept
		{
			return ssaoDesc.bias;
		}
		float GetSsaoSharpness()const noexcept
		{
			return ssaoDesc.sharpness;
		}
		float GetSsaoSmallAoScale()const noexcept
		{
			return ssaoDesc.smallAoScale;
		}
		float GetSsaoLargeAoScale()const noexcept
		{
			return ssaoDesc.largeAoScale;
		}
		J_SSAO_TYPE GetSsaoType()const noexcept
		{
			return ssaoDesc.ssaoType;
		}
		J_SSAO_SAMPLE_TYPE GetSsaoSampleType()const noexcept
		{
			return ssaoDesc.sampleType;
		}
		uint GetSsaoBlurRadius()const noexcept
		{
			return ssaoDesc.useBlur ? (uint)ssaoDesc.blurKenelSize + 1 : 0;
		}
		J_KERNEL_SIZE GetSsaoBlurKenelSize()const noexcept
		{
			return ssaoDesc.blurKenelSize;
		}
	public:
		void SetFrameDirty()
		{
			if (graphicData == nullptr)
				return;

			auto fUser = graphicData->GetFrameUpdateUserInterface();
			if (fUser == nullptr)
				return;

			fUser->SetFrameDirty();
		}
		void SetNear(const float value)noexcept
		{
			camNear = std::clamp(value, Constants::minCamFrustumNear, camFar - Constants::minCamFrustumDistance);
			if (camNear < 0.1f)
				camNear = 0.1f;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFar(const float value) noexcept
		{
			camFar = std::clamp(value, camNear + Constants::minCamFrustumDistance, Constants::maxCamFrustumFar);
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFovY(const float value) noexcept
		{
			camFov = value;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetFovYDegree(const float value) noexcept
		{
			camFov = value * JMathHelper::DegToRad;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetAspect(float value) noexcept
		{
			camAspect = value;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetOrthoViewWidth(const float value)noexcept
		{
			camOrthoViewWidth = value;
			if (isOrtho)
				CalOrthoLens();
		}
		void SetOrthoViewHeight(const float value)noexcept
		{
			camOrthoViewHeight = value;
			if (isOrtho)
				CalOrthoLens();
		}
		void SetOrthoViewSize(const float width, const float height)
		{
			camOrthoViewWidth = width;
			camOrthoViewHeight = height;
			if (isOrtho)
				CalOrthoLens();
		}
		void SetOrthoCamera(const bool value)noexcept
		{
			isOrtho = value;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		/*
		void SetFrustumCullingFrequency(const float value)noexcept
		{
			frustumCulingFrequency = value;
		}
		void SetOcclusionCullingFrequency(const float value)noexcept
		{
			occlusionCulingFrequency = value;
		}
		*/
		void SetAllowDisplayRenderResult(bool value)noexcept
		{
			if (allowDisplayRs == value)
				return;

			allowDisplayRs = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_RENDER_RESULT, this, SetParam(value, false));
		}
		void SetAllowDisplayDebugObject(bool value)noexcept
		{
			if (allowDisplayDebugObject == value)
				return;

			allowDisplayDebugObject = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_DEBUG_OBJECT, this, SetParam(value, false));
		}
		void SetAllowDisplayOccCullingDepthMap(bool value)noexcept
		{
			if (allowDisplayOccCullingDepthMap == value)
				return;

			allowDisplayOccCullingDepthMap = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, this, SetParam(value, false));
		}
		void SetAllowDisplayLightCullingDebugging(bool value)noexcept
		{
			if (allowDisplayLightCullingDebug == value || !AllowLightCulling())
				return;

			allowDisplayLightCullingDebug = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_DISPLAY_LIGHT_CULLING_DEBUG, this, SetParam(value, false));
		}
		void SetAllowFrustumCulling(bool value)noexcept
		{
			if (allowFrustumCulling == value)
				return;

			allowFrustumCulling = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_FURSTUM_CULLING, this, SetParam(value, false));
		}
		void SetAllowHzbOcclusionCulling(bool value)noexcept
		{
			if (allowHzbOcclusionCulling == value)
				return;

			allowHzbOcclusionCulling = value;
			if (allowHzbOcclusionCulling && AllowHdOcclusionCulling())
				SetAllowHdOcclusionCulling(false);

			SetFuncList().InvokePassLocalCondition(MANAGED_SET_HZB_CULLING, this, SetParam(value, false));
		}
		void SetAllowHdOcclusionCulling(bool value)noexcept
		{
			if (allowHdOcclusionCulling == value)
				return;

			allowHdOcclusionCulling = value;
			if (allowHdOcclusionCulling && AllowHzbOcclusionCulling())
				SetAllowHzbOcclusionCulling(false);

			SetFuncList().InvokePassLocalCondition(MANAGED_SET_HD_CULLING, this, SetParam(value, false));
		}
		void SetAllowLightCulling(bool value)
		{
			if (AllowLightCulling() == value || !thisPointer->GetOwner()->GetOwnerScene()->AllowLightCulling())
				return;

			SetFuncList().InvokePassLocalCondition(MANAGED_SET_LIGHT_CULLING, this, SetParam(value, false));
		}
		void SetReflectAllCullingResult(const bool value)noexcept
		{
			allowReflectAllCullResult = value;
			SetFrameDirty();
		}
		void SetAllowSsao(bool value)
		{
			if (allowSsao == value)
				return;

			allowSsao = value;
			SetFuncList().InvokePassLocalCondition(MANAGED_SET_SSAO, this, SetParam(value, false));
		}
		void SetCameraState(const J_CAMERA_STATE state)noexcept
		{
			if (camState == state)
				return;

			camState = state;
			if (camState == J_CAMERA_STATE::RENDER)
			{
				if (thisPointer->IsActivated())
				{
					Activate();
					RegisterComponent(thisPointer);
				}
			}
			else
			{
				if (thisPointer->IsActivated())
				{
					DeRegisterComponent(thisPointer);
					DeActivate();
				}
			}
			SetFrameDirty();
		}
		void SetRenderTargetRate(const JVector2F rate)noexcept
		{
			auto gUser = graphicData->GetGraphicResourceUserInterface();
			if (rtSizeRate == rate || rate.x == 0 || rate.y == 0)
				return;

			rtSizeRate = rate;
			if (gUser->IsValidHandle(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW))
			{
				DeActivate();
				Activate();
			}
			SetFrameDirty();
		}
		void SetSsaoDesc(const JSsaoDesc& newDesc)
		{
			if (ssaoDesc == newDesc)
				return;

			ssaoDesc = newDesc;
			if (thisPointer->IsActivated() && allowSsao)
			{
				JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);
				JGraphicResourceCreationDesc desc(typeSet, GetRtSize());

				GraphicModuleInterface()->DestroyAllGraphicsResourcesOfType(graphicData.Get(), J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP);
				GraphicModuleInterface()->CreateGraphicResource(graphicData.Get(), desc);
			}
			SetFrameDirty();
		}
		void SetSsaoRadius(const float value)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.radius = std::clamp(value, Private::minSsaoRadius, Private::maxSsaoRadius);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoBias(const float value)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.bias = std::clamp(value, Private::minSsaoBias, Private::maxSsaoBias);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoSharpness(const float value)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.sharpness = std::clamp(value, Private::minSsaoSharpness, Private::maxSsaoSharpness);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoSmallAoScale(const float value)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.smallAoScale = std::clamp(value, Private::minSsaoAoScale, Private::maxSsaoAoScale);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoLargeAoScale(const float value)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.largeAoScale = std::clamp(value, Private::minSsaoAoScale, Private::maxSsaoAoScale);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoType(const J_SSAO_TYPE type)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.ssaoType = type;
			SetSsaoDesc(newDesc);
		}
		void SetSsaoSampleType(const J_SSAO_SAMPLE_TYPE type)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.sampleType = type;
			SetSsaoDesc(newDesc);
		}
		void SetSsaoBlurRadius(uint value)noexcept
		{
			if (value == 0)
				ssaoDesc.useBlur = false;
			else
			{
				value -= 1;	//dispaly range(1 ~ count) to enum range(0 ~ count -1)
				if (value >= (uint)J_KERNEL_SIZE::COUNT)
					return;

				ssaoDesc.useBlur = true;
				SetSsaoBlurKenelSize((J_KERNEL_SIZE)value);
			}
		}
		void SetSsaoBlurKenelSize(const J_KERNEL_SIZE kenelSize)noexcept
		{
			JSsaoDesc newDesc = ssaoDesc;
			newDesc.blurKenelSize = kenelSize;
			SetSsaoDesc(newDesc);
		}
		static ManageFuncList& SetFuncList()
		{
			static ManageFuncList setFuncList;
			return setFuncList;
		}
	public:
		bool IsOrthoCamera()const noexcept
		{
			return isOrtho;
		}
		bool IsActivated()const noexcept
		{
			return thisPointer->IsActivated();
		}
		bool CanResizeTarget()const
		{
			return thisPointer->GetOwner()->GetOwnerScene()->IsMainScene();
		}
		bool AllowDisplayRenderResult()const noexcept
		{
			return allowDisplayRs;
		}
		bool AllowDisplayDebugObject()const noexcept
		{
			return allowDisplayDebugObject;
		}
		bool AllowDisplayOccCullingDepthMap()const noexcept
		{
			return allowDisplayOccCullingDepthMap;
		}
		bool AllowDisplayLightCullingDebug()const noexcept
		{
			return AllowLightCulling() && allowDisplayLightCullingDebug;
		}
		bool AllowFrustumCulling()const noexcept
		{
			return allowFrustumCulling;
		}
		bool AllowHzbOcclusionCulling()const noexcept
		{
			return allowHzbOcclusionCulling;
		}
		bool AllowHdOcclusionCulling()const noexcept
		{
			return allowHdOcclusionCulling;
		}
		bool AllowLightCulling()const noexcept
		{
			return thisPointer->GetOwner()->GetOwnerScene()->IsMainScene() && !thisPointer->GetOwner()->IsEditorObject();
			//return allowLightCulling;
			// !thisPointer->GetOwner()->HasFlag(OBJECT_FLAG_ONLY_USED_IN_EDITOR)
		}
		bool AllowReflectAllCullingResult()const noexcept
		{
			return allowReflectAllCullResult;
		}
		bool AllowSsao()const noexcept
		{
			return allowSsao;
		}
		/*
		bool AllowBuildGBuffer()const noexcept
		{
			return JGraphic::Instance().GetGraphicOptionRef().rendering.allowDeferred;
			//return true;
			//return thisPointer->GetOwner()->GetOwnerScene()->IsMainScene();
		}
		*/
		bool AllowDeferred()const noexcept
		{
			return GMI()->IsActivated(J_GRAPHIC_OPTIONAL_FEATURE::DEFERRED_RENDERING);
		}
		bool AllowHighCostRendering()const noexcept
		{
			return thisPointer->GetOwner()->GetOwnerScene()->IsMainScene() && !thisPointer->GetOwner()->IsEditorObject();
		}
		bool AllowTemporalResource()const noexcept
		{
			return AllowHighCostRendering();
		}
		bool AllowPostProcess()const noexcept
		{
			return AllowHighCostRendering() && GMI()->IsActivated(J_GRAPHIC_OPTIONAL_FEATURE::POST_PROCESSING);
		}
		bool AllowRaytracingGI()const noexcept
		{
			return AllowHighCostRendering() && GMI()->IsActivated(J_GRAPHIC_OPTIONAL_FEATURE::RAYTRACING_GI);
			//JGraphic::Instance().GetGraphicOptionRef().rendering.allowRaytracing && 
			//JGraphic::Instance().GetGraphicOptionRef().rendering.allowDeferred;
		}
	public:
		void CalPerspectiveLens() noexcept
		{
			camNearViewHeight = 2.0f * camNear * tanf(0.5f * camFov);
			camFarViewHeight = 2.0f * camFar * tanf(0.5f * camFov);

			const XMMATRIX P = XMMatrixPerspectiveFovLH(camFov, camAspect, camNear, camFar);
			mProj.StoreXM(P);
			UpdateProjMatrixDependency();

			BoundingFrustum::CreateFromMatrix(mCamFrustum, mProj.LoadXM());
			SetFrameDirty();
		}
		void CalOrthoLens() noexcept
		{
			//XMMatrixOrthographicOffCenterLH
			const XMMATRIX P = XMMatrixOrthographicLH(camOrthoViewWidth, camOrthoViewHeight, camNear, camFar);
			mProj.StoreXM(P);
			UpdateProjMatrixDependency();

			BoundingFrustum::CreateFromMatrix(mCamFrustum, mProj.LoadXM());
			SetFrameDirty();
		}
	public:
		void Activate()noexcept
		{  
			IMPL_REGISTER_TRANFORM_FRAME_DRITY_LISTENER();
			IMPL_REGISTER_FRAME_UPDATE_ACTION();

			RegisterEvent();
			RegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA>();
			RegisterCsmTarget();

			SetFuncList().InvokeAll(this, CONDTION_MASK::PASS_NONE, SetParam(true, true));
			if (thisPointer->GetOwner()->GetOwnerScene()->GetUseCaseType() == J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW)
				GraphicModuleInterface()->RequestExecutableGraphicFeature(graphicData.Get(), JGraphicRequestCreationDesc(J_GRAPHIC_REQUEST_TYPE::DRAW_SCENE, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED));
			else
				GraphicModuleInterface()->RequestExecutableGraphicFeature(graphicData.Get(), JGraphicRequestCreationDesc(J_GRAPHIC_REQUEST_TYPE::DRAW_SCENE));
			SetFrameDirty();
		}
		void DeActivate()noexcept
		{
			GraphicModuleInterface()->CancelExecutableGraphicFeature(graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::DRAW_SCENE);
			SetFuncList().InvokeAllReverse(this, CONDTION_MASK::PASS_NONE, SetParam(false, true));

			GraphicModuleInterface()->DestroyAllGraphicsResources(graphicData.Get());
			GraphicModuleInterface()->DestroyAllCullingData(graphicData.Get());

			DeRegisterCsmTarget();
			DeRegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA>();
			DeRegisterEvent();

			IMPL_DEREGISTER_FRAME_UPDATE_ACTION();
			IMPL_DEREGISTER_TRANFORM_FRAME_DRITY_LISTENER()
		}
	private:
		void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
		{
			if (senderGuid == thisPointer->GetGuid())
				return;

			if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
			{
				const JVector2F clientSize = JWindow::GetClientSize();
				SetOrthoViewWidth(clientSize.x);
				SetOrthoViewHeight(clientSize.y);

				SetFuncList().InvokeGroupReverse(GROUP_SET_CLIENT_SIZE_DEPENDENCY, this, SetParam(false, false));
				SetFuncList().InvokeGroup(GROUP_SET_CLIENT_SIZE_DEPENDENCY, this, SetParam(true, false));
			}
		}
	private:
		void HotUpdate()
		{
			thisPointer->GetTransform()->CalTransformMatrix(mView);
		}
		void AlwaysUpdate()
		{
			if (AllowTemporalResource())
			{
				mPreView = mView;
				mPreViewProj.StoreXM(XMMatrixMultiply(mView.LoadXM(), mProj.LoadXM()));
				preUvToViewA = uvToViewA;
				preUvToViewB = uvToViewB;
			}
		}
		void UpdateProjMatrixDependency()
		{
			tanHalfFovX = 1.0f / fabs(mProj(0, 0));
			tanHalfFovY = 1.0f / fabs(mProj(1, 1));

			//uv -> view = (screen coord * (2.0f, -2.0f) + (- 1.0f,  1.0f)) * z * (inv proj)
			//z와 screen coord는 shader에서 수행하므로
			//uvToView =  ((2.0f, -2.0f) +  (- 1.0f,  1.0f)) * inv proj

			uvToViewA.x = 2.0f * tanHalfFovX;
			uvToViewA.y = -2.0f * tanHalfFovY;
			uvToViewB.x = -1.0f * tanHalfFovX;
			uvToViewB.y = 1.0f * tanHalfFovY;

			//uvToView test 
			/*
			if (IsFrameHotDirted())
			{
				const XMMATRIX view = mView.LoadXM();
				const XMMATRIX viewProj = XMMatrixMultiply(view, mProj.LoadXM());
				XMMATRIX reProj;

				for (uint i = 0; i < 1; ++i)
				{
					reProj = (XMMatrixMultiply(XMMatrixInverse(nullptr, viewProj), mPreViewProj.LoadXM()));
					JVector4F posW(0, 10, 10, 1);
					JVector4F posH = DirectX::XMVector4Transform(posW.ToXmV(), viewProj);
					JVector4F prePosH = DirectX::XMVector4Transform(posH.ToXmV(), reProj);

					Develop::JDevelopDebug::PushLog("posH: " + posH.ToString());
					Develop::JDevelopDebug::PushLog("prePosH: " + prePosH.ToString());

					JVector3F nowPos = posH.XYZ() / posH.w;
					JVector3F prePos1 = prePosH.XYZ() / posH.w;
					JVector3F prePos2 = prePosH.XYZ() / prePosH.w;

					Develop::JDevelopDebug::PushLog("nowPos H: " + nowPos.ToString());
					Develop::JDevelopDebug::PushLog("prePos / nowPosW H: " + prePos1.ToString());
					Develop::JDevelopDebug::PushLog("prePos / prePosW H: " + prePos2.ToString());

					//	return ((far * near) / (far - v * (far - near)) - near) / (far - near);
					nowPos.x = nowPos.x * 0.5f + 0.5f;
					nowPos.y = nowPos.y * -0.5f + 0.5f;
					nowPos.z = LinearDepth(nowPos.z);

					prePos1.x = prePos1.x * 0.5f + 0.5f;
					prePos1.y = prePos1.y * -0.5f + 0.5f;
					prePos1.z = LinearDepth(prePos1.z);

					prePos2.x = prePos2.x * 0.5f + 0.5f;
					prePos2.y = prePos2.y * -0.5f + 0.5f;
					prePos2.z = LinearDepth(prePos2.z);

					Develop::JDevelopDebug::PushLog("nowPos uv: " + nowPos.ToString());
					Develop::JDevelopDebug::PushLog("prePos / nowPosW uv: " + prePos1.ToString());
					Develop::JDevelopDebug::PushLog("prePos / prePosW uv: " + prePos2.ToString());

					Develop::JDevelopDebug::PushLog("prePos1 - nowPos: " + (prePos1 - nowPos).ToString());
					Develop::JDevelopDebug::PushLog("prePos2 - nowPos: " + (prePos2 - nowPos).ToString());
				}
				Develop::JDevelopDebug::PushLog("\n");
				Develop::JDevelopDebug::Write();
			}
			*/
			//결과값에 차이가 없거나 있어도 미세할것으로 판단.
			/*
			JVector2F uv = JVector2F(0.3, 0.3f);
			float depth = 0.975f;
			JVector4F worldPos00 = JVector4F(GetWorldPos(uv, depth), 1.0f);

			JVector4F viewPos = DirectX::XMVector4Transform(worldPos00.ToXmV(), mView.LoadXM());
			JVector4F projPos = DirectX::XMVector4Transform(viewPos.ToXmV(), mProj.LoadXM());
			JVector3F ndc = projPos.XYZ() / projPos.w;
			JVector2F screen = ndc.XY() * JVector2F(0.5f, -0.5f) + 0.5f;

			MessageBoxA(0, worldPos00.ToString().c_str(), "worldPos00", 0);
			MessageBoxA(0, viewPos.ToString().c_str(), "viewPos", 0);
			MessageBoxA(0, projPos.ToString().c_str(), "ProjPos", 0);
			MessageBoxA(0, ndc.ToString().c_str(), "Ndc", 0);
			MessageBoxA(0, screen.ToString().c_str(), "Screen", 0);
			*/
		}
		/*
		float NdcToViewPZ(const float v)
		{
			return (camNear * camFar) / (camFar - v * (camFar - camNear));
		}
		JVector3F UVToViewSpace(JVector2F uv, float z)
		{
			uv = uvToViewA * uv + uvToViewB;
			return JVector3F(uv * z, z);
		}
		JVector3F GetWorldPos(JVector2F uv, float depth)
		{
			return DirectX::XMVector3Transform(UVToViewSpace(uv, NdcToViewPZ(depth)).ToXmV(), XMMatrixInverse(nullptr, mView.LoadXM()));
		}
		*/
		float LinearDepth(float v)
		{
			return ((camFar * camNear) / (camFar - v * (camFar - camNear)) - camNear) / (camFar - camNear);
		}
	public:
		static bool DoCopy(JCamera* from, JCamera* to)
		{
			to->impl->camNear = from->impl->camNear;
			to->impl->camFar = from->impl->camFar;
			to->impl->camAspect = from->impl->camAspect;
			to->impl->camFov = from->impl->camFov;
			to->impl->camNearViewHeight = from->impl->camNearViewHeight;
			to->impl->camFarViewHeight = from->impl->camFarViewHeight;
			to->impl->isOrtho = from->impl->isOrtho;
			if (!to->impl->isOrtho)
				to->impl->CalPerspectiveLens();
			else
				to->impl->CalOrthoLens();
			return true;
		}
	public:
		void NotifyReAlloc()
		{
			if (CanResizeTarget())
				WindowEventListener::ResetEventListenerPointer(*JWindow::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void Initialize()
		{
			camFov = Constants::defaultCamFovDeg * JMathHelper::DegToRad;
			//camFov = 0.25f * JMathHelper::Pi;
			camNear = Constants::defaultCamFrustumNear;
			camFar = Constants::defaultCamFrustumFar;
			mView = JMatrix4x4::Identity();
			mProj = JMatrix4x4::Identity();
			mPreView = JMatrix4x4::Identity();
			mPreViewProj = JMatrix4x4::Identity();

			const JVector2F clientSize = JWindow::GetClientSize();
			SetAspect(clientSize.x / clientSize.y);
			SetOrthoViewWidth(clientSize.x);
			SetOrthoViewHeight(clientSize.y);
			//ownerTransform->SetPosition(XMFLOAT3(0.0f, 2.0f, -15.0f));
			CalPerspectiveLens();

			BoundingFrustum::CreateFromMatrix(mCamFrustum, mProj.LoadXM());
		}
		void RegisterThisPointer(JCamera* cam)
		{
			thisPointer = Core::GetWeakPtr(cam);
		}
		void RegisterPostCreation()
		{
		}
		template<J_FRAME_RESOURCE_UPLOAD_TYPE T, const uint indexSize = 1>
		void RegisterFrame()
		{
			JFrameUploadDataCreationDesc desc(T, thisPointer->GetAreaGuid(), indexSize);
			GMI()->CreateFrameUploadData(graphicData.Get(), desc);
		}
		void RegisterCsmTarget()
		{
			auto getFrustumLam = [](JUserPtr<JObject> obj)
			{
				return static_cast<JCamera*>(obj.Get())->GetBoundingFrustum();
			};
			using FrustumF = GetCsmTargetBoundingFrustumF;

			JCsmTargetCreationDesc desc(thisPointer->GetGuid(),
				thisPointer->GetAreaGuid(),
				FrustumF::CreateCompletelyBind(getFrustumLam, JUserPtr<JObject>(thisPointer)));

			GraphicModuleInterface()->CreateCsmTarget(graphicData.Get(), desc);
		}
		void RegisterEvent()
		{
			if (CanResizeTarget())
				WindowEventListener::AddEventListener(*JWindow::EvInterface(), thisPointer->GetGuid(), Window::J_WINDOW_EVENT::WINDOW_RESIZE);
		}
		void DeRegisterPreDestruction()
		{
		}
		template<J_FRAME_RESOURCE_UPLOAD_TYPE T>
		bool DeRegisterFrame()
		{
			return GraphicModuleInterface()->DestroyFrameUploadData(graphicData.Get(), T);
		}
		void DeRegisterCsmTarget()
		{
			GraphicModuleInterface()->DestroyCsmTarget(graphicData.Get());
		}
		void DeRegisterEvent()
		{
			if (CanResizeTarget())
				WindowEventListener::RemoveEventListener(*JWindow::EvInterface(), thisPointer->GetGuid(), Window::J_WINDOW_EVENT::WINDOW_RESIZE);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JCamera::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ Private::isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](const Core::JTypeInfo& typeInfo, JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JCamera::InitData>(ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JCamera::InitData>(parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable, isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ };

			JComponent::RegisterCTypeInfo(JCamera::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JCamera::StaticTypeInfo(), Private::instance);

			IMPL_REALLOC_BIND()

			auto setMainRtLam = [](JCameraImpl* impl, SetParam p)
			{
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
				if (p.value)
				{
					JGraphicResourceCreationDesc rtDesc(typeSet, impl->GetRtSize());
					const bool canBindUav = impl->AllowPostProcess() || impl->AllowRaytracingGI();
					const bool canUseVelocity = canBindUav;
					if (canBindUav)
					{
						rtDesc.bindDesc.useEngineDefinedBindType = false;
						rtDesc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
					}
					GMI()->CreateGraphicResource(impl->graphicData.Get(), rtDesc);

					if (canUseVelocity)
					{
						rtDesc.type.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY; 
						GMI()->CreateGraphicResourceOption(impl->graphicData.Get(), rtDesc.type);
					}

					if (impl->AllowDeferred())
						impl->SetFuncList().Invoke(MANAGED_SET_DEFERRED_RESOURCE, impl, SetParam(true, p.isCalledByAct));
				}
				else
				{
					if (impl->AllowDeferred())
						impl->SetFuncList().Invoke(MANAGED_SET_DEFERRED_RESOURCE, impl, SetParam(false, p.isCalledByAct));
		 
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);
				}
				impl->SetFrameDirty();
			};
			auto setMainDsLam = [](JCameraImpl* impl, SetParam p)
			{
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
				if (p.value)
					GMI()->CreateGraphicResource(impl->graphicData.Get(), JGraphicResourceCreationDesc(typeSet, impl->GetRtSize()));
				else
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);
				impl->SetFrameDirty();
			};
			auto setDisplayRsLam = [](JCameraImpl* impl, SetParam p)
			{
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
				if (p.value)
				{
					auto gInterface = impl->graphicData.Get()->GetGraphicResourceUserInterface();
					J_GRAPHIC_TASK_TYPE taskType[]
					{
						J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE,
						J_GRAPHIC_TASK_TYPE::ALBEDO_MAP_VISUALIZE,
						J_GRAPHIC_TASK_TYPE::SPECULAR_MAP_VISUALIZE,
						J_GRAPHIC_TASK_TYPE::NORMAL_MAP_VISUALIZE,
						J_GRAPHIC_TASK_TYPE::TANGENT_MAP_VISUALIZE,
						J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE,
						J_GRAPHIC_TASK_TYPE::VELOCITY_MAP_VISUALIZE
					};
					JGraphicResourceCreationDesc desc(typeSet, impl->GetRtSize());
					for (uint i = 0; i < SIZE_OF_ARRAY(taskType); ++i)
					{
						if (!gInterface->GetAllocableResourceCount(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, taskType[i]))
							continue;

						desc.type.task = taskType[i];
						GMI()->CreateGraphicResource(impl->graphicData.Get(), desc);
					}
				}
				else
					GMI()->DestroyAllGraphicsResourcesOfType(impl->graphicData.Get(), typeSet.resouce);
				impl->SetFrameDirty();
			};
			auto setDisplayDebugObjLam = [](JCameraImpl* impl, SetParam p)
			{
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
				if (p.value)
					GMI()->CreateGraphicResource(impl->graphicData.Get(), JGraphicResourceCreationDesc(typeSet, impl->GetRtSize()));
				else
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);
				impl->SetFrameDirty();
			};
			auto setDisplayOccDepthMapLam = [](JCameraImpl* impl, SetParam p)
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
				impl->SetFrameDirty();
			};
			auto setDisplayLightCullLam = [](JCameraImpl* impl, SetParam p)
			{
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
				if (p.value)
					GMI()->CreateGraphicResource(impl->graphicData.Get(), JGraphicResourceCreationDesc(typeSet, impl->GetRtSize()));
				else
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);
				impl->SetFrameDirty();
			};
			auto setFrustumCullLam = [](JCameraImpl* impl, SetParam p)
			{
				const JCullingTypeSet typeSet(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM);
				if (p.value)
				{
					JGraphicRequestCreationDesc desc(J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED);

					GMI()->CreateCullingData(impl->graphicData.Get(), typeSet);
					GMI()->RequestExecutableGraphicFeature(impl->graphicData.Get(), desc);
				}
				else
				{
					GMI()->CancelExecutableGraphicFeature(impl->graphicData.Get(), J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING);
					GMI()->DestroyCullingData(impl->graphicData.Get(), typeSet);
				}
				impl->SetFrameDirty();
			};
			auto setHzbCullLam = [](JCameraImpl* impl, SetParam p)
			{
				const JCullingTypeSet cullingTypeSet(J_CULLING_TYPE::HZB_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
				if (p.value)
				{
					impl->RegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS>();
					impl->RegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS>();

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

					if (debugSrvCount == 0)
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

					impl->DeRegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS>();
					if (p.isCalledByAct || (!impl->AllowHzbOcclusionCulling() && !impl->AllowHdOcclusionCulling()))
						impl->DeRegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS>();

					if (!impl->AllowHdOcclusionCulling())
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
				}
				impl->SetFrameDirty();
			};
			auto setHdCullLam = [](JCameraImpl* impl, SetParam p)
			{
				const JCullingTypeSet cullingTypeSet(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
				if (p.value)
				{
					impl->RegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS>();
					GMI()->CreateCullingData(impl->graphicData.Get(), cullingTypeSet);

					JGraphicRequestCreationDesc requestDesc(J_GRAPHIC_REQUEST_TYPE::HARD_WARE_OCCLUSION_CULLING, J_GRAPHIC_REQUEST_EXECUTE_FREQUENCY::UPDATED);
					GMI()->RequestExecutableGraphicFeature(impl->graphicData.Get(), requestDesc);

					if (impl->AllowDisplayOccCullingDepthMap())
					{
						auto gUser = impl->graphicData->GetGraphicResourceUserInterface();
						const uint debugSrvCount = gUser->GetViewCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
						 
						if (debugSrvCount == 0)
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
						impl->DeRegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS>();
					if (!impl->AllowHzbOcclusionCulling())
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
				}
				impl->SetFrameDirty();
			};
			auto setLitCullLam = [](JCameraImpl* impl, SetParam p)
			{
				if (!impl->thisPointer->GetOwner()->GetOwnerScene()->AllowLightCulling())
					return;

				const JCullingTypeSet cullingTypeSet(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::LIGHT);
				if (p.value)
				{
					impl->RegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::LIGHT_CULLING_PASS>();
					GMI()->CreateCullingData(impl->graphicData.Get(), cullingTypeSet);

					JGraphicResourceTypeSet typeSet;
					JGraphicResourceCreationDesc desc(typeSet);
					desc.useEngineDefine = true;

					desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST;
					desc.type.task = J_GRAPHIC_TASK_TYPE::LIGHT_CULLING;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), desc);

					desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET;
					desc.type.task = J_GRAPHIC_TASK_TYPE::LIGHT_CULLING;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), desc);
				}
				else
				{
					JGraphicResourceTypeSet typeSet;
					typeSet.resouce = J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET;
					typeSet.task = J_GRAPHIC_TASK_TYPE::LIGHT_CULLING;
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);

					typeSet.resouce = J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST;
					typeSet.task = J_GRAPHIC_TASK_TYPE::LIGHT_CULLING;
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);

					GMI()->DestroyCullingData(impl->graphicData.Get(), cullingTypeSet);

					impl->DeRegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::LIGHT_CULLING_PASS>();
				}
				impl->SetFrameDirty();
			};
			auto setSsaoLam = [](JCameraImpl* impl, SetParam p)
			{
				JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);
				if (p.value)
				{
					impl->RegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::SSAO_PASS>();
					GMI()->CreateGraphicResource(impl->graphicData.Get(), JGraphicResourceCreationDesc(typeSet, impl->GetRtSize()));
				}
				else
				{
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), typeSet);
					impl->DeRegisterFrame<J_FRAME_RESOURCE_UPLOAD_TYPE::SSAO_PASS>();
				}
				impl->SetFrameDirty();
			};
			auto setImageProcessingRtLam = [](JCameraImpl* impl, SetParam p)
			{
				JGraphicResourceTypeSet rtTypeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT);
				JGraphicResourceTypeSet exposureTypeSet(J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE, J_GRAPHIC_TASK_TYPE::MANAGE_POST_PROCESS_EXPOSURE);
				if (p.value)
				{
					JGraphicResourceCreationDesc rtDesc(rtTypeSet, impl->GetRtSize());
					rtDesc.bindDesc.useEngineDefinedBindType = false;
					rtDesc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), rtDesc);

					JGraphicResourceCreationDesc exposureDesc(exposureTypeSet);
					exposureDesc.useEngineDefine = true;
					exposureDesc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>();
					exposureDesc.uploadBufferDesc->useEngineDefine = true;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), exposureDesc);
				}
				else
				{
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), exposureTypeSet);
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), rtTypeSet); 
				}
				impl->SetFrameDirty();
			};
			auto setGIRtLam = [](JCameraImpl* impl, SetParam p)
			{
				JGraphicResourceTypeSet rtTypeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
				JGraphicResourceTypeSet initialSampleTypeSet(J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
				JGraphicResourceTypeSet reserviorTypeSet(J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);

				if (p.value)
				{
					JGraphicResourceCreationDesc rtDesc(rtTypeSet, impl->GetRtSize());
					rtDesc.bindDesc.useEngineDefinedBindType = false;
					rtDesc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), rtDesc);

					JGraphicResourceCreationDesc initialSampleDesc(initialSampleTypeSet);
					initialSampleDesc.width = rtDesc.width * rtDesc.height * 0.25f;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), initialSampleDesc);

					JGraphicResourceCreationDesc reserviorDesc(reserviorTypeSet);
					reserviorDesc.width = initialSampleDesc.width;
					GMI()->CreateGraphicResource(impl->graphicData.Get(), reserviorDesc);
					GMI()->CreateGraphicResource(impl->graphicData.Get(), reserviorDesc);
					GMI()->CreateGraphicResource(impl->graphicData.Get(), reserviorDesc);
					GMI()->CreateGraphicResource(impl->graphicData.Get(), reserviorDesc);
				}
				else
				{
					GMI()->DestroyAllGraphicsResourcesOfType(impl->graphicData.Get(), reserviorTypeSet.resouce);
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), initialSampleTypeSet);
					GMI()->DestroyGraphicResource(impl->graphicData.Get(), rtTypeSet); 
				}
				impl->SetFrameDirty();
			};
			auto setDeferredLam = [](JCameraImpl* impl, SetParam p)
			{
				JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
				const int resourceIndex = impl->graphicData->GetGraphicResourceUserInterface()->GetResourceIndexOffset(typeSet.resouce, typeSet.task);
				if (!impl->graphicData->GetGraphicResourceUserInterface()->IsValidHandle(typeSet.resouce, resourceIndex))
					return;

				if (p.value)
				{
					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
					if (impl->graphicData->GetGraphicResourceUserInterface()->HasOption(typeSet.resouce, typeSet.option, typeSet.task))
						return;

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
					GMI()->CreateGraphicResourceOption(impl->graphicData.Get(), typeSet);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
					GMI()->CreateGraphicResourceOption(impl->graphicData.Get(), typeSet);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
					GMI()->CreateGraphicResourceOption(impl->graphicData.Get(), typeSet);
				}
				else
				{
					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
					GMI()->DestroyGraphicResourceOption(impl->graphicData.Get(), typeSet);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
					GMI()->DestroyGraphicResourceOption(impl->graphicData.Get(), typeSet);

					typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
					GMI()->DestroyGraphicResourceOption(impl->graphicData.Get(), typeSet);
				}
				impl->graphicData->GetFrameUpdateUserInterface()->SetFrameDirty();
			};
			auto setSpatialTemporalResourceLam = [](JCameraImpl* impl, SetParam p)
			{ 
				JGraphicResourceTypeSet rtTypeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA);
				JGraphicResourceTypeSet depthTypeSet(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA);
				if (p.value)
				{  
					JGraphicResourceCreationDesc rtDesc(rtTypeSet, impl->GetRtSize());
					GMI()->CreateGraphicResource(impl->graphicData.Get(), rtDesc);

					JGraphicResourceCreationDesc depthDesc(depthTypeSet, impl->GetRtSize());
					GMI()->CreateGraphicResource(impl->graphicData.Get(), depthDesc);

					rtTypeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
					GMI()->CreateGraphicResourceOption(impl->graphicData.Get(), rtTypeSet);

					rtTypeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
					GMI()->CreateGraphicResourceOption(impl->graphicData.Get(), rtTypeSet);

					rtTypeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY;
					GMI()->CreateGraphicResourceOption(impl->graphicData.Get(), rtTypeSet);
				}
				else
				{
					rtTypeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY;
					GMI()->DestroyGraphicResourceOption(impl->graphicData.Get(), rtTypeSet);

					rtTypeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
					GMI()->DestroyGraphicResourceOption(impl->graphicData.Get(), rtTypeSet);

					rtTypeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
					GMI()->DestroyGraphicResourceOption(impl->graphicData.Get(), rtTypeSet);

					GMI()->DestroyGraphicResource(impl->graphicData.Get(), rtTypeSet);

					GMI()->DestroyGraphicResource(impl->graphicData.Get(), depthTypeSet);			 
				}
				impl->SetFrameDirty();
			};
			using SetCallable = Core::JStaticCallable<void, JCameraImpl*, SetParam>;
			using CondCallable = Core::JMemberCNCallable<JCameraImpl, bool>;

			SetFuncList().Register(std::make_unique<SetCallable>(setMainRtLam), MANAGED_SET_MAIN_RENDER_TARGET);
			SetFuncList().Register(std::make_unique<SetCallable>(setMainDsLam), MANAGED_SET_MAIN_DEPTH_STENCIL);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayRsLam), std::make_unique<CondCallable>(&JCameraImpl::AllowDisplayRenderResult), MANAGED_SET_DISPLAY_RENDER_RESULT);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayDebugObjLam), std::make_unique<CondCallable>(&JCameraImpl::AllowDisplayDebugObject), MANAGED_SET_DISPLAY_DEBUG_OBJECT);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayOccDepthMapLam), std::make_unique<CondCallable>(&JCameraImpl::AllowDisplayOccCullingDepthMap), MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP);
			SetFuncList().Register(std::make_unique<SetCallable>(setDisplayLightCullLam), std::make_unique<CondCallable>(&JCameraImpl::AllowDisplayLightCullingDebug), MANAGED_SET_DISPLAY_LIGHT_CULLING_DEBUG);
			SetFuncList().Register(std::make_unique<SetCallable>(setFrustumCullLam), std::make_unique<CondCallable>(&JCameraImpl::AllowFrustumCulling), MANAGED_SET_FURSTUM_CULLING);
			SetFuncList().Register(std::make_unique<SetCallable>(setHzbCullLam), std::make_unique<CondCallable>(&JCameraImpl::AllowHzbOcclusionCulling), MANAGED_SET_HZB_CULLING);
			SetFuncList().Register(std::make_unique<SetCallable>(setHdCullLam), std::make_unique<CondCallable>(&JCameraImpl::AllowHdOcclusionCulling), MANAGED_SET_HD_CULLING);
			SetFuncList().Register(std::make_unique<SetCallable>(setLitCullLam), std::make_unique<CondCallable>(&JCameraImpl::AllowLightCulling), MANAGED_SET_LIGHT_CULLING);
			SetFuncList().Register(std::make_unique<SetCallable>(setSsaoLam), std::make_unique<CondCallable>(&JCameraImpl::AllowSsao), MANAGED_SET_SSAO);
			SetFuncList().Register(std::make_unique<SetCallable>(setImageProcessingRtLam), std::make_unique<CondCallable>(&JCameraImpl::AllowPostProcess), MANAGED_SET_IMAGE_PROCESSING);
			SetFuncList().Register(std::make_unique<SetCallable>(setGIRtLam), std::make_unique<CondCallable>(&JCameraImpl::AllowRaytracingGI), MANAGED_SET_GI);
			SetFuncList().Register(std::make_unique<SetCallable>(setDeferredLam), std::make_unique<CondCallable>(&JCameraImpl::AllowDeferred), MANAGED_SET_DEFERRED_RESOURCE);
			SetFuncList().Register(std::make_unique<SetCallable>(setSpatialTemporalResourceLam), std::make_unique<CondCallable>(&JCameraImpl::AllowTemporalResource), MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE);

			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_MAIN_RENDER_TARGET);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_MAIN_DEPTH_STENCIL);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_IMAGE_PROCESSING);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_GI);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_DISPLAY_RENDER_RESULT);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_DISPLAY_DEBUG_OBJECT);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_DISPLAY_LIGHT_CULLING_DEBUG);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_SSAO);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_DEFERRED_RESOURCE);
			SetFuncList().RegisterG(GROUP_SET_CLIENT_SIZE_DEPENDENCY, MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE);
			SetFuncList().RegisterGlobalCond(std::make_unique<CondCallable>(&JCameraImpl::IsActivated));

			auto deferredChangedLam = [](JObject* obj, const bool value)
			{
				JCameraImpl* cam = static_cast<JCamera*>(obj)->impl.get();
				SetFuncList().InvokePassLocalCondition(MANAGED_SET_DEFERRED_RESOURCE, cam, SetParam(value, false));
			};
			auto postProcessChangedLam = [](JObject* obj, const bool value)
			{
				JCameraImpl* cam = static_cast<JCamera*>(obj)->impl.get();
				if (!cam->AllowHighCostRendering())
					return;

				if (!cam->AllowRaytracingGI())
				{
					//recreate mainRt
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_MAIN_RENDER_TARGET, cam, SetParam(false, false));
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_MAIN_RENDER_TARGET, cam, SetParam(true, false));

					//ta resource
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE, cam, SetParam(value, false));
				} 
				SetFuncList().InvokePassLocalCondition(MANAGED_SET_IMAGE_PROCESSING, cam, SetParam(cam->AllowPostProcess(), false));
			};
			auto raytracingChangedLam = [](JObject* obj, const bool value)
			{
				JCameraImpl* cam = static_cast<JCamera*>(obj)->impl.get();
				if (!cam->AllowHighCostRendering())
					return;
				 
				if (!cam->AllowPostProcess())
				{
					//recreate mainRt
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_MAIN_RENDER_TARGET, cam, SetParam(false, false));
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_MAIN_RENDER_TARGET, cam, SetParam(true, false));

					//ta resource
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE, cam, SetParam(value, false));
				}
				SetFuncList().InvokePassLocalCondition(MANAGED_SET_GI, cam, SetParam(cam->AllowRaytracingGI(), false));
			};
			auto formatChangedLam = [](JObject* obj, const bool value)
			{
				JCameraImpl* cam = static_cast<JCamera*>(obj)->impl.get();
				if (!cam->AllowHighCostRendering())
					return;

				if (!cam->AllowPostProcess())
				{
					//recreate mainRt
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_MAIN_RENDER_TARGET, cam, SetParam(false, false));
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_MAIN_RENDER_TARGET, cam, SetParam(true, false));

					//ta resource
					SetFuncList().InvokePassLocalCondition(MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE, cam, SetParam(value, false));
				}
				SetFuncList().InvokePassLocalCondition(MANAGED_SET_GI, cam, SetParam(cam->AllowRaytracingGI(), false));
			};

			JGraphicOptionalFeatureObserverDesc graphicFeatureObserver;
			graphicFeatureObserver.uniqueIndex = compVariationIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>;

			graphicFeatureObserver.type = J_GRAPHIC_OPTIONAL_FEATURE::DEFERRED_RENDERING;
			graphicFeatureObserver.ptr = deferredChangedLam;
			Rule::JGraphicModuleInterface::RegisterOptionalFeatureObserver(graphicFeatureObserver);

			graphicFeatureObserver.type = J_GRAPHIC_OPTIONAL_FEATURE::POST_PROCESSING;
			graphicFeatureObserver.ptr = postProcessChangedLam;
			Rule::JGraphicModuleInterface::RegisterOptionalFeatureObserver(graphicFeatureObserver);

			graphicFeatureObserver.type = J_GRAPHIC_OPTIONAL_FEATURE::RAYTRACING;
			graphicFeatureObserver.ptr = raytracingChangedLam;
			Rule::JGraphicModuleInterface::RegisterOptionalFeatureObserver(graphicFeatureObserver);
		}
	};

	JCamera::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(JCamera::StaticTypeInfo(), owner)
	{}
	JCamera::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(JCamera::StaticTypeInfo(), GetDefaultName(JCamera::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JCamera::PrivateInterface()const noexcept
	{
		return Private::instance;
	}
	JGraphicModuleManagedDataFrame* JCamera::ModuleManagedData()const noexcept
	{
		return impl->graphicData.Get();
	}
	uint JCamera::GetSubTypeIndex()const noexcept
	{
		return 0;
	}
	J_COMPONENT_TYPE JCamera::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JUserPtr<JTransform> JCamera::GetTransform()noexcept
	{
		return GetOwner()->GetTransform();
	}
	JMatrix4x4 JCamera::GetView()const noexcept
	{
		return impl->mView;
	} 
	DirectX::XMMATRIX JCamera::GetInvView()const noexcept
	{
		return XMMatrixInverse(nullptr, impl->mView.LoadXM());
	}
	JMatrix4x4 JCamera::GetProj()const noexcept
	{
		return impl->mProj;
	}
	JMatrix4x4 JCamera::GetPreView()const noexcept
	{
		return impl->mPreView;
	}
	DirectX::XMMATRIX JCamera::GetPreInvView()const noexcept
	{
		return XMMatrixInverse(nullptr, impl->mPreView.LoadXM());
	}
	JMatrix4x4 JCamera::GetPreViewProj()const noexcept
	{
		return impl->mPreViewProj;
	}
	void JCamera::GetUvToView(JVector2F& a, JVector2F& b)const noexcept
	{
		a = impl->uvToViewA;
		b = impl->uvToViewB;
	}
	void JCamera::GetPreUvToView(JVector2F& a, JVector2F& b)const noexcept
	{
		a = impl->preUvToViewA;
		b = impl->preUvToViewB;
	}
	DirectX::BoundingFrustum JCamera::GetBoundingFrustum()const noexcept
	{
		return impl->GetBoundingFrustum();
	}
	DirectX::BoundingFrustum JCamera::GetLocalBoundingFrustum()const noexcept
	{
		return impl->mCamFrustum;
	}
	float JCamera::GetNear()const noexcept
	{
		return impl->GetNear();
	}
	float JCamera::GetFar()const noexcept
	{
		return impl->GetFar();
	}
	float JCamera::GetFovX()const noexcept
	{
		return impl->GetFovX();
	}
	float JCamera::GetFovXDegree()const noexcept
	{
		return impl->GetFovXDegree();
	}
	float JCamera::GetFovY()const noexcept
	{
		return impl->camFov;
	}
	float JCamera::GetFovYDegree()const noexcept
	{
		return impl->GetFovYDegree();
	}
	float JCamera::GetTanHalfFovY()const noexcept
	{
		return impl->tanHalfFovY;
	}
	float JCamera::GetAspect()const noexcept
	{
		return impl->camAspect;
	}
	float JCamera::GetOrthoViewWidth()const noexcept
	{
		return impl->GetOrthoViewWidth();
	}
	float JCamera::GetOrthoViewHeight()const noexcept
	{
		return impl->GetOrthoViewHeight();
	}
	float JCamera::GetNearViewWidth()const noexcept
	{
		return impl->GetNearViewWidth();
	}
	float JCamera::GetNearViewHeight()const noexcept
	{
		return impl->camNearViewHeight;
	}
	float JCamera::GetFarViewWidth()const noexcept
	{
		return impl->GetFarViewWidth();
	}
	float JCamera::GetFarViewHeight()const noexcept
	{
		return impl->camFarViewHeight;
	}
	float JCamera::GetRenderViewWidth()const noexcept
	{
		return impl->GetRenderViewWidth();
	}
	float JCamera::GetRenderViewHeight()const noexcept
	{
		return impl->GetRenderViewHeight();
	}
	J_CAMERA_STATE JCamera::GetCameraState()const noexcept
	{
		return impl->camState;
	}
	JVector2F JCamera::GetRenderTargetSize()const noexcept
	{
		return impl->GetRtSize();
	}
	JVector2F JCamera::GetRenderTargetRate()const noexcept
	{
		return impl->rtSizeRate;
	}
	JSsaoDesc JCamera::GetSsaoDesc()const noexcept
	{
		return impl->ssaoDesc;
	}
	void JCamera::SetNear(float value)noexcept
	{
		impl->SetNear(value);
	}
	void JCamera::SetFar(float value) noexcept
	{
		impl->SetFar(value);
	}
	void JCamera::SetFov(float value) noexcept
	{
		impl->SetFovY(value);
	}
	void JCamera::SetFovDegree(float value) noexcept
	{
		impl->SetFovYDegree(value);
	}
	void JCamera::SetAspect(const float value) noexcept
	{
		impl->SetAspect(value);
	}
	void JCamera::SetOrthoViewWidth(const float value) noexcept
	{
		impl->SetOrthoViewWidth(value);
	}
	void JCamera::SetOrthoViewHeight(const float value) noexcept
	{
		impl->SetOrthoViewHeight(value);
	}
	void JCamera::SetOrthoViewSize(const float width, const float height) noexcept
	{
		impl->SetOrthoViewSize(width, height);
	}
	void JCamera::SetOrthoCamera(bool value)noexcept
	{
		impl->SetOrthoCamera(value);
	}
	void JCamera::SetAllowDisplayRenderResult(const bool value)noexcept
	{
		impl->SetAllowDisplayRenderResult(value);
	}
	void JCamera::SetAllowDisplayDebugObject(const bool value)noexcept
	{
		impl->SetAllowDisplayDebugObject(value);
	}
	void JCamera::SetAllowFrustumCulling(const bool value)noexcept
	{
		impl->SetAllowFrustumCulling(value);
	}
	void JCamera::SetAllowHzbOcclusionCulling(const bool value)noexcept
	{
		impl->SetAllowHzbOcclusionCulling(value);
	}
	void JCamera::SetAllowHdOcclusionCulling(const bool value)noexcept
	{
		impl->SetAllowHdOcclusionCulling(value);
	}
	void JCamera::SetAllowDisplayOccCullingDepthMap(const bool value)noexcept
	{
		impl->SetAllowDisplayOccCullingDepthMap(value);
	}
	void JCamera::SetReflectAllCullingResult(const bool value)noexcept
	{
		impl->SetReflectAllCullingResult(value);
	}
	void JCamera::SetAllowSsao(const bool value)noexcept
	{
		impl->SetAllowSsao(value);
	}
	void JCamera::SetCameraState(const J_CAMERA_STATE state)noexcept
	{
		impl->SetCameraState(state);
	}
	void JCamera::SetRenderTargetRate(const JVector2F rate)noexcept
	{
		impl->SetRenderTargetRate(rate);
	}
	void JCamera::SetSsaoDesc(const JSsaoDesc& desc)noexcept
	{
		impl->SetSsaoDesc(desc);
	}
	bool JCamera::IsOrthoCamera()const noexcept
	{
		return impl->isOrtho;
	}
	bool JCamera::IsAvailableOverlap()const noexcept
	{
		return Private::isAvailableoverlapLam();
	}
	bool JCamera::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && impl->camState == J_CAMERA_STATE::RENDER)
			return true;
		else
			return false;
	}
	bool JCamera::AllowDisplayRenderResult()const noexcept
	{
		return impl->AllowDisplayRenderResult();
	}
	bool JCamera::AllowDisplayDebugObject()const noexcept
	{
		return impl->AllowDisplayDebugObject();
	}
	bool JCamera::AllowFrustumCulling()const noexcept
	{
		return impl->AllowFrustumCulling();
	}
	bool JCamera::AllowHzbOcclusionCulling()const noexcept
	{
		return impl->AllowHzbOcclusionCulling();
	}
	bool JCamera::AllowHdOcclusionCulling()const noexcept
	{
		return impl->AllowHdOcclusionCulling();
	}
	bool JCamera::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
	}
	bool JCamera::AllowLightCulling()const noexcept
	{
		return impl->AllowLightCulling();
	}
	bool JCamera::AllowDisplayLightCullingDebug()const noexcept
	{
		return impl->AllowDisplayLightCullingDebug();
	}
	bool JCamera::AllowReflectAllCullingResult()const noexcept
	{
		return impl->AllowDisplayLightCullingDebug();
	}
	bool JCamera::AllowSsao()const noexcept
	{
		return impl->AllowSsao();
	}
	bool JCamera::AllowPostProcess()const noexcept
	{
		return impl->AllowPostProcess();
	}
	bool JCamera::AllowRaytracingGI()const noexcept
	{
		return impl->AllowRaytracingGI();
	}
	void JCamera::DoActivate()noexcept
	{
		//Caution 
		//Activate와 RegisterComponent는 순서에 종속성을 가진다.
		//RegisterComponent는 Scene과 가속구조에 Component에 대한 정보를 추가하는 작업으로
		//Activate Process중에 자기자신과 관련된 Scene component vector, Scene As관련 data에 대한 호출은 에러를 일으킬 수 있다.
		INTERFACE_ALLOC_GRAPHIC_MODULE_DATA();
		JComponent::DoActivate();
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			impl->Activate();
			RegisterComponent(impl->thisPointer);
		}
	}
	void JCamera::DoDeActivate()noexcept
	{
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			DeRegisterComponent(impl->thisPointer);
			impl->DeActivate();
		}
		JComponent::DoDeActivate();
		DEALLOC_GRAPHIC_MODULE_DATA();
	}
	JCamera::JCamera(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JCameraImpl>(initData, this))
	{}
	JCamera::~JCamera()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JCameraPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JCameraPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JCameraPrivate::AssetDataIOInterface; 

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JCamera>(*static_cast<JCamera::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JCamera* cam = static_cast<JCamera*>(createdPtr);
		cam->impl->RegisterThisPointer(cam);
		cam->impl->RegisterPostCreation();
		cam->impl->Initialize();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JCamera::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JCamera::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JCamera::JCameraImpl::DoCopy(static_cast<JCamera*>(from.Get()), static_cast<JCamera*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)noexcept
	{
		static_cast<JCamera*>(ptr)->impl->DeRegisterPreDestruction();
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JCamera::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;

		J_CAMERA_STATE camState;
		JVector3F pos = JVector3F::Zero();
		float camNear = 0;
		float camFar = 0;
		float camFov = 0;
		float camAspect = 0;
		float camOrthoViewWidth = 0;
		float camOrthoViewHeight = 0;
		bool isOrtho = false;
		bool allowDisplayRs = false;
		bool allowDisplayDebugObject = false;
		bool allowDisplayOccCullingDepthMap = false;
		bool allowDisplayLightCullingDebug = false;
		bool allowFrustumCulling = false;
		bool allowHzbOcclusionCulling = false;
		bool allowHdOcclusionCulling = false;
		//bool allowLightCulling = false;
		bool allowSsao = false;
		JVector2F rtSizeRate = JVector2F::One();
		JSsaoDesc ssaoDesc;

		auto loadData = static_cast<JCamera::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		JObjectFileIOHelper::LoadEnumData(tool, camState, "CamState:");
		JObjectFileIOHelper::LoadVector3(tool, pos, "Pos:");
		JObjectFileIOHelper::LoadAtomicData(tool, camNear, "CamNear:");
		JObjectFileIOHelper::LoadAtomicData(tool, camFar, "CamFar:");
		JObjectFileIOHelper::LoadAtomicData(tool, camFov, "CamFov:");
		JObjectFileIOHelper::LoadAtomicData(tool, camAspect, "CamAspect:");
		JObjectFileIOHelper::LoadAtomicData(tool, camOrthoViewWidth, "camOrthoViewWidth:");
		JObjectFileIOHelper::LoadAtomicData(tool, camOrthoViewHeight, "CamOrthoViewHeight:");
		JObjectFileIOHelper::LoadAtomicData(tool, isOrtho, "IsOrtho:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowDisplayRs, "AllowDisplayRs:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowDisplayDebugObject, "AllowDisplayDebugObject:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowDisplayOccCullingDepthMap, "AllowDislplayCullingDepthMap:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowDisplayLightCullingDebug, "AllowDisplayLightCullingDebug:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowFrustumCulling, "AllowFrustumCulling:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowHzbOcclusionCulling, "AllowHzbOcclusionCulling:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowHdOcclusionCulling, "AllowHdOcclusionCulling:");
		//JObjectFileIOHelper::LoadAtomicData(tool, allowLightCulling, "AllowLightCulling:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowSsao, "AllowSsao:");
		JObjectFileIOHelper::LoadVector2(tool, rtSizeRate, "RtSizeRate:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.radius, "SsaoRadius:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.bias, "SsaoBias:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.sharpness, "SsaoSharpness :");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.smallAoScale, "SsaoSmallAoScale:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.largeAoScale, "SsaoLargeAoScale :");
		JObjectFileIOHelper::LoadEnumData(tool, ssaoDesc.ssaoType, "SsaoType:");
		JObjectFileIOHelper::LoadEnumData(tool, ssaoDesc.sampleType, "SsaoSampleType:");
		JObjectFileIOHelper::LoadEnumData(tool, ssaoDesc.blurKenelSize, "SsaoBlurKenelSize:");

		auto initData = std::make_unique<JCamera::InitData>(guid, flag, owner);
		initData->rtSizeRate = rtSizeRate;

		auto idenUser = Private::instance.GetCreateInstanceInterface().BeginCreate(std::move(initData), &Private::instance);
		JUserPtr<JCamera> camUser;
		camUser.ConnnectChild(idenUser);

		JCamera::JCameraImpl* impl = camUser->impl.get();
		impl->camNear = camNear;
		impl->camFar = camFar;
		camUser->SetNear(camNear);
		camUser->SetFar(camFar);

		impl->camFov = camFov;
		impl->camAspect = camAspect;
		impl->camOrthoViewWidth = camOrthoViewWidth;
		impl->camOrthoViewHeight = camOrthoViewHeight;
		camUser->GetTransform()->SetPosition(pos);

		if (isOrtho)
			impl->CalOrthoLens();
		else
			impl->CalPerspectiveLens();

		impl->SetSsaoDesc(ssaoDesc);
		impl->SetAllowDisplayRenderResult(allowDisplayRs);
		impl->SetAllowDisplayDebugObject(allowDisplayDebugObject);
		impl->SetAllowDisplayOccCullingDepthMap(allowDisplayOccCullingDepthMap);
		impl->SetAllowDisplayLightCullingDebugging(allowDisplayLightCullingDebug);
		impl->SetAllowFrustumCulling(allowFrustumCulling);
		impl->SetAllowHzbOcclusionCulling(allowHzbOcclusionCulling);
		impl->SetAllowHdOcclusionCulling(allowHdOcclusionCulling);
		//impl->SetAllowLightCulling(allowLightCulling);
		impl->SetAllowSsao(allowSsao);
		impl->SetRenderTargetRate(rtSizeRate);
		impl->SetCameraState(camState);
		if (!isActivated)
			camUser->DeActivate();

		return camUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JCamera::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JCamera::StoreData*>(data);
		if (!storeData->HasCorrectType(JCamera::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JCamera> cam;
		cam.ConnnectChild(storeData->obj);
		JCamera::JCameraImpl* impl = cam->impl.get();
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, cam.Get());

		JObjectFileIOHelper::StoreEnumData(tool, impl->camState, "CamState:");
		JObjectFileIOHelper::StoreVector3(tool, cam->GetTransform()->GetPosition(), "Pos:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camNear, "CamNear:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camFar, "CamFar:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camFov, "CamFov:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camAspect, "CamAspect:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camOrthoViewWidth, "camOrthoViewWidth:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camOrthoViewHeight, "CamOrthoViewHeight:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->isOrtho, "IsOrtho:");

		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowDisplayRs, "AllowDisplayRs:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowDisplayDebugObject, "AllowDisplayDebugObject:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowDisplayOccCullingDepthMap, "AllowDislplayCullingDepthMap:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowDisplayLightCullingDebug, "AllowDisplayLightCullingDebug:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowFrustumCulling, "AllowFrustumCulling:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowHzbOcclusionCulling, "AllowHzbOcclusionCulling:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowHdOcclusionCulling, "AllowHdOcclusionCulling:");
		//JObjectFileIOHelper::StoreAtomicData(tool, impl->allowLightCulling, "AllowLightCulling:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowSsao, "AllowSsao:");

		JObjectFileIOHelper::StoreVector2(tool, impl->rtSizeRate, "RtSizeRate:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.radius, "SsaoRadius:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.bias, "SsaoBias:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.sharpness, "SsaoSharpness :");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.smallAoScale, "SsaoSmallAoScale:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.largeAoScale, "SsaoLargeAoScale :");
		JObjectFileIOHelper::StoreEnumData(tool, impl->ssaoDesc.ssaoType, "SsaoType:");
		JObjectFileIOHelper::StoreEnumData(tool, impl->ssaoDesc.sampleType, "SsaoSampleType:");
		JObjectFileIOHelper::StoreEnumData(tool, impl->ssaoDesc.blurKenelSize, "SsaoBlurKenelSize:");
		return Core::J_FILE_IO_RESULT::SUCCESS;
	} 

	Core::JIdentifierPrivate::CreateInstanceInterface& JCameraPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JCameraPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JCameraPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}
