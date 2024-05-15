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
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h"  
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"  
#include"../../../Core/Guid/JGuidCreator.h"  
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Core/Func/JFuncList.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/JGraphicDrawListInterface.h"
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/Culling/JCullingConstants.h"  
#include"../../../Graphic/Frameresource/JCameraConstants.h"    
#include"../../../Graphic/Frameresource/JFrameUpdate.h"  
#include"../../../Graphic/ShadowMap/JCsmTargetInterface.h"
#include"../../../Graphic/Image/JImageProcessingFilter.h"
#include"../../../Window/JWindow.h"  
#include<fstream>

//#include"../../../Develop/Debug/JDevelopDebug.h"
namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JCameraPrivate cPrivate;
	}

	namespace
	{
		using CameraFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder5<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, Graphic::JDrawSceneCameraConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, Graphic::JDepthTestConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, Graphic::JHzbOccComputeConstants&, const uint, const uint>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_CULLING_PASS, Graphic::JLightCullingCameraConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, Graphic::JSsaoConstants&>>,
			Graphic::JFrameDirty>;

		using JGraphicWideSingleAndRestrictMultiResourceHolder = Graphic::JGraphicWideSingleAndRestrictMultiResourceHolder<11,
			false,
			Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL,
			Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON,
			Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP,
			Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR>;
		using JCullingTypePerSingleTargetHolder = Graphic::JCullingTypePerSingleTargetHolder;
		using WindowEventListener = Core::JEventListener<size_t, Window::J_WINDOW_EVENT>;
		//first extra is occlusion 
	}
	namespace Private
	{
		static constexpr uint mainRtNumber = 0;
		static constexpr uint afterPostProcesseNumber = 1;
		static constexpr uint giRtNumber = 2;
		static constexpr uint litCullDebugRtNumber = 3;

		static constexpr uint mainDsNumber = 0;
		static constexpr uint previousDsNumber = 1;

		static constexpr uint depthDebuggingNumber = 0;
		static constexpr uint specularDebuggingNumber = 1;
		static constexpr uint normalDebuggingNumber = 2;
		static constexpr uint tangentDebuggingNumber = 3;
		static constexpr uint velocityDebuggingNumber = 4;
		static constexpr uint ssaoDebuggingNumber = 5;
		static constexpr uint debuggingMapCount = ssaoDebuggingNumber + 1;

		static constexpr float minSsaoRadius = 0.01f;
		static constexpr float maxSsaoRadius = 32.0f;
		static constexpr float minSsaoBias = -maxSsaoRadius;
		static constexpr float maxSsaoBias = maxSsaoRadius; ;
		static constexpr float minSsaoSharpness = 0.01f;
		static constexpr float maxSsaoSharpness = maxSsaoRadius;

		static constexpr float minSsaoAoScale = 0.0f;
		static constexpr float maxSsaoAoScale = 1.25f;

		static constexpr uint minSsaoBlurRadius = 0;
		static constexpr uint maxSsaoBlurRadius = (uint)Graphic::J_KERNEL_SIZE::COUNT;

		using GetFrameDataPtr = Graphic::JFrameUpdateData* (*)(JCamera*);
		static GetFrameDataPtr getFrameDataPtr[Graphic::CameraFrameLayer::setCount];
	}
	class JCamera::JCameraImpl : public Core::JTypeImplBase,
		public CameraFrameUpdate,
		public JGraphicWideSingleAndRestrictMultiResourceHolder,
		public JCullingTypePerSingleTargetHolder,
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCsmTargetInterface,
		public Graphic::GraphicEventListener,
		public WindowEventListener
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JCameraImpl)
			REGISTER_GUI_BOOL_CONDITION(IsOrthoCam, isOrtho, false)
	public:
		using CamFrame = JFrameInterface1;
		using DepthTestFrame = JFrameInterface2;
		using HzbOccReqFrame = JFrameInterface3;
		using LightCullFrame = JFrameInterface4;
		using SsaoFrame = JFrameInterface5;
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
	public:
		JWeakPtr<JCamera> thisPointer = nullptr;
	public:
		// Cache View/Proj matrices.
		JMatrix4x4 mView;
		JMatrix4x4 mProj;
		JMatrix4x4 mPreViewProj;
		DirectX::BoundingFrustum mCamFrustum;
		JVector2F uvToViewA = JVector2F::Zero();
		JVector2F uvToViewB = JVector2F::Zero();
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
		REGISTER_METHOD(GetFarViewWidth)
			REGISTER_METHOD_READONLY_GUI_WIDGET(camFarViewWidth, GetFarViewWidth, GUI_READONLY_TEXT())
			REGISTER_METHOD(GetFarViewHeight)
			REGISTER_METHOD_READONLY_GUI_WIDGET(camFarViewHeight, GetFarViewHeight, GUI_READONLY_TEXT())
			float camNearViewHeight = 0.0f;
		float camFarViewHeight = 0.0f;
	public:
		//Culling Option
		REGISTER_PROPERTY_EX(frustumCulingFrequency, GetFrustumCullingFrequency, SetFrustumCullingFrequency, GUI_SLIDER(Graphic::Constants::cullingUpdateFrequencyMin, Graphic::Constants::cullingUpdateFrequencyMax, true))
			float frustumCulingFrequency = 0;
		REGISTER_PROPERTY_EX(occlusionCulingFrequency, GetOcclusionCullingFrequency, SetOcclusionCullingFrequency, GUI_SLIDER(Graphic::Constants::cullingUpdateFrequencyMin, Graphic::Constants::cullingUpdateFrequencyMax, true))
			float occlusionCulingFrequency = 0;
	public:
		REGISTER_GUI_GROUP(Ssao)
			REGISTER_METHOD_GUI_WIDGET(SsaoRadius, GetSsaoRadius, SetSsaoRadius, GUI_SLIDER(Private::minSsaoRadius, Private::maxSsaoRadius, true, false, 3, GUI_GROUP_USER(Ssao)))
			REGISTER_METHOD_GUI_WIDGET(SsaoBias, GetSsaoBias, SetSsaoBias, GUI_SLIDER(Private::minSsaoBias, Private::maxSsaoBias, true, false, 3, GUI_GROUP_USER(Ssao)))
			REGISTER_METHOD_GUI_WIDGET(SsaoSharpness, GetSsaoSharpness, SetSsaoSharpness, GUI_SLIDER(Private::minSsaoSharpness, Private::maxSsaoSharpness, true, false, 3, GUI_GROUP_USER(Ssao)))
			REGISTER_METHOD_GUI_WIDGET(SsaoSmallAoScale, GetSsaoSmallAoScale, SetSsaoSmallAoScale, GUI_SLIDER(Private::minSsaoAoScale, Private::maxSsaoAoScale, true, false, 3, GUI_GROUP_USER(Ssao)))
			REGISTER_METHOD_GUI_WIDGET(SsaoLargeAoScale, GetSsaoLargeAoScale, SetSsaoLargeAoScale, GUI_SLIDER(Private::minSsaoAoScale, Private::maxSsaoAoScale, true, false, 3, GUI_GROUP_USER(Ssao)))
			REGISTER_METHOD_GUI_WIDGET(SsaoType, GetSsaoType, SetSsaoType, GUI_ENUM_COMBO(Graphic::J_SSAO_TYPE, "", GUI_GROUP_USER(Ssao)))
			REGISTER_METHOD_GUI_WIDGET(SsaoSample, GetSsaoSampleType, SetSsaoSampleType, GUI_ENUM_COMBO(Graphic::J_SSAO_SAMPLE_TYPE, "", GUI_GROUP_USER(Ssao)))
			REGISTER_METHOD_GUI_WIDGET(SsaoBlurRadius, GetSsaoBlurRadius, SetSsaoBlurRadius, GUI_SLIDER(Private::minSsaoBlurRadius, Private::maxSsaoBlurRadius, false, false, 1, GUI_GROUP_USER(Ssao)))
			Graphic::JSsaoDesc ssaoDesc;
	public:
		REGISTER_PROPERTY_EX(isOrtho, IsOrthoCamera, SetOrthoCamera, GUI_CHECKBOX())
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
		bool allowAllCamCullResult = false;	//use editor cam for check space spatial result
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
		float GetFrustumCullingFrequency()const noexcept
		{
			return frustumCulingFrequency;
		}
		float GetOcclusionCullingFrequency()const noexcept
		{
			return occlusionCulingFrequency;
		}
		DirectX::BoundingFrustum GetBoundingFrustum()const noexcept final
		{
			DirectX::BoundingFrustum worldCamFrustum;
			mCamFrustum.Transform(worldCamFrustum, thisPointer->GetOwner()->GetTransform()->GetWorldMatrix().LoadXM());
			return worldCamFrustum;
		}
		size_t GetCsmTargetGuid()const noexcept final
		{
			return thisPointer->GetGuid();
		}
		size_t GetCsmAreaGuid()const noexcept final
		{
			return thisPointer->GetOwner()->GetOwnerGuid();
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
		Graphic::J_SSAO_TYPE GetSsaoType()const noexcept
		{
			return ssaoDesc.ssaoType;
		}
		Graphic::J_SSAO_SAMPLE_TYPE GetSsaoSampleType()const noexcept
		{
			return ssaoDesc.sampleType;
		}
		uint GetSsaoBlurRadius()const noexcept
		{
			return ssaoDesc.useBlur ? (uint)ssaoDesc.blurKenelSize + 1 : 0;
		}
		Graphic::J_KERNEL_SIZE GetSsaoBlurKenelSize()const noexcept
		{
			return ssaoDesc.blurKenelSize;
		}
		/**
		* Multi resource indexing에 주의!
		*/
		int GetResourceDataIndex(const Graphic::J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
			{
				if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::RAYTRACING_GI || taskType == Graphic::J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
					return Private::previousDsNumber;
				else
					return Private::mainDsNumber;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
			{ 
				if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE)
					return Private::depthDebuggingNumber;
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::SPECULAR_MAP_VISUALIZE)
					return Private::specularDebuggingNumber;
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::NORMAL_MAP_VISUALIZE)
					return Private::normalDebuggingNumber;
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::TANGENT_MAP_VISUALIZE)
					return Private::tangentDebuggingNumber;
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::VELOCITY_MAP_VISUALIZE)
					return Private::velocityDebuggingNumber;
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE)
					return Private::ssaoDebuggingNumber;
				else
					return invalidIndex;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
			{
				if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::SCENE_DRAW)
					return Private::mainRtNumber;
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT)
					return Private::afterPostProcesseNumber;
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::RAYTRACING_GI || taskType == Graphic::J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
					return Private::giRtNumber - (OnImageProcessingTrigger() ? 0 : 1);
				else if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::LIGHT_LIST_DRAW)
					return Private::litCullDebugRtNumber - ((OnImageProcessingTrigger() ? 0 : 1) + (OnGITrigger() ? 0 : 1));
				else
					return invalidIndex;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
			{
				if (taskType == Graphic::J_GRAPHIC_TASK_TYPE::RAYTRACING_GI)
					return 0;
				else
					return invalidIndex;
			}
			default:
				return invalidIndex;
			}
		}
		int GetFrameDirtyMax()const noexcept final
		{
			//첫번째 frame에 update된 constants에  mPreViewProj이 유효한 값을 갖게 하기위해 +1
			//(1: delta), (2: zero), (3: zero), (1: zero)
			return Graphic::Constants::gNumFrameResources + 1;
		}
	public:
		void SetNear(const float value)noexcept
		{
			camNear = std::clamp(value, Constants::minCamFrustumNear, camFar);
			if (camNear < 0.1f)
				camNear = 0.1f;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFar(const float value) noexcept
		{
			camFar = std::clamp(value, camNear, Constants::maxCamFrustumFar);
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
		void SetFrustumCullingFrequency(const float value)noexcept
		{
			frustumCulingFrequency = value;
		}
		void SetOcclusionCullingFrequency(const float value)noexcept
		{
			occlusionCulingFrequency = value;
		}
		void SetAllowDisplayRenderResult(bool value)noexcept
		{
			if (allowDisplayRs == value)
				return;

			allowDisplayRs = value;
			SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_RENDER_RESULT, this, SetParam(value, false));
		}
		void SetAllowDisplayDebugObject(bool value)noexcept
		{
			if (allowDisplayDebugObject == value)
				return;

			allowDisplayDebugObject = value;
			SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_DEBUG_OBJECT, this, SetParam(value, false));
		}
		void SetAllowDisplayOccCullingDepthMap(bool value)noexcept
		{
			if (allowDisplayOccCullingDepthMap == value)
				return;

			allowDisplayOccCullingDepthMap = value;
			SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, this, SetParam(value, false));
		}
		void SetAllowDisplayLightCullingDebugging(bool value)noexcept
		{
			if (allowDisplayLightCullingDebug == value || !AllowLightCulling())
				return;

			allowDisplayLightCullingDebug = value;
			SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_LIGHT_CULLING_DEBUG, this, SetParam(value, false));
		}
		void SetAllowFrustumCulling(bool value)noexcept
		{
			if (allowFrustumCulling == value)
				return;

			allowFrustumCulling = value;
			SetFuncList().InvokePassCondition(MANAGED_SET_FURSTUM_CULLING, this, SetParam(value, false));
		}
		void SetAllowHzbOcclusionCulling(bool value)noexcept
		{
			if (allowHzbOcclusionCulling == value)
				return;

			allowHzbOcclusionCulling = value;
			if (allowHzbOcclusionCulling && AllowHdOcclusionCulling())
				SetAllowHdOcclusionCulling(false);

			SetFuncList().InvokePassCondition(MANAGED_SET_HZB_CULLING, this, SetParam(value, false));
		}
		void SetAllowHdOcclusionCulling(bool value)noexcept
		{
			if (allowHdOcclusionCulling == value)
				return;

			allowHdOcclusionCulling = value;
			if (allowHdOcclusionCulling && AllowHzbOcclusionCulling())
				SetAllowHzbOcclusionCulling(false);

			SetFuncList().InvokePassCondition(MANAGED_SET_HD_CULLING, this, SetParam(value, false));
		}
		void SetAllowLightCulling(bool value)
		{
			if (AllowLightCulling() == value || !thisPointer->GetOwner()->GetOwnerScene()->AllowLightCulling())
				return;

			SetFuncList().InvokePassCondition(MANAGED_SET_LIGHT_CULLING, this, SetParam(value, false));
		}
		void SetAllowAllCullingResult(const bool value)noexcept
		{
			allowAllCamCullResult = value;
			SetFrameDirty();
		}
		void SetAllowSsao(bool value)
		{
			if (allowSsao == value)
				return;

			allowSsao = value;
			SetFuncList().InvokePassCondition(MANAGED_SET_SSAO, this, SetParam(value, false));
		}
		void SetDeferred(bool value)noexcept
		{
			SetFuncList().InvokePassCondition(MANAGED_SET_DEFERRED_RESOURCE, this, SetParam(value, false));
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
			if (rtSizeRate == rate || rate.x == 0 || rate.y == 0)
				return;

			rtSizeRate = rate;  
			if (HasResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON))
			{
				DeActivate();
				Activate();
			}
			SetFrameDirty();
		}
		void SetSsaoDesc(const Graphic::JSsaoDesc& newDesc)
		{
			if (ssaoDesc == newDesc)
				return;

			ssaoDesc = newDesc;
			if (thisPointer->IsActivated() && allowSsao)
			{
				DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP);
				CreateResource(GetRtSize(), Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP);
			}
			SetFrameDirty();
		}
		void SetSsaoRadius(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.radius = std::clamp(value, Private::minSsaoRadius, Private::maxSsaoRadius);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoBias(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.bias = std::clamp(value, Private::minSsaoBias, Private::maxSsaoBias);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoSharpness(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.sharpness = std::clamp(value, Private::minSsaoSharpness, Private::maxSsaoSharpness);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoSmallAoScale(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.smallAoScale = std::clamp(value, Private::minSsaoAoScale, Private::maxSsaoAoScale);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoLargeAoScale(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.largeAoScale = std::clamp(value, Private::minSsaoAoScale, Private::maxSsaoAoScale);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoType(const Graphic::J_SSAO_TYPE type)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.ssaoType = type;
			SetSsaoDesc(newDesc);
		}
		void SetSsaoSampleType(const Graphic::J_SSAO_SAMPLE_TYPE type)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
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
				if (value >= (uint)Graphic::J_KERNEL_SIZE::COUNT)
					return;

				ssaoDesc.useBlur = true;
				SetSsaoBlurKenelSize((Graphic::J_KERNEL_SIZE)value);
			}
		}
		void SetSsaoBlurKenelSize(const Graphic::J_KERNEL_SIZE kenelSize)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
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
		bool AllowAllCullingResult()const noexcept
		{
			return allowAllCamCullResult;
		}
		bool AllowSsao()const noexcept
		{
			return allowSsao;
		}
		bool AllowBuildGBuffer()const noexcept
		{
			return JGraphic::Instance().GetGraphicOptionRef().rendering.allowDeferred;
			//return true;
			//return thisPointer->GetOwner()->GetOwnerScene()->IsMainScene();
		} 
		bool AllowTemporalResource()const noexcept
		{
			return OnGITrigger();
		}
		bool OnImageProcessingTrigger()const noexcept
		{
			//test code
			return thisPointer->GetOwner()->GetOwnerScene()->IsMainScene() &&
				!thisPointer->GetOwner()->IsEditorObject() &&
				JGraphic::Instance().GetGraphicOptionRef().IsPostProcessActivated();
			//return false;
		}
		bool OnGITrigger()const noexcept
		{
			//test code
			return thisPointer->GetOwner()->GetOwnerScene()->IsMainScene() &&
				!thisPointer->GetOwner()->IsEditorObject() &&
				JGraphic::Instance().GetGraphicOptionRef().rendering.allowRaytracing && 
				JGraphic::Instance().GetGraphicOptionRef().rendering.allowDeferred;
		}
		bool HasImageProcessingType()const noexcept
		{
			return HasResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP);
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
			RegisterEvent();
			RegisterFrame<CamFrame>();
			RegisterCsmTargetInterface();

			SetFuncList().InvokeAll(this, true, SetParam(true, true));
			if (thisPointer->GetOwner()->GetOwnerScene()->GetUseCaseType() == J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW)
				AddDrawSceneRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			else
				AddDrawSceneRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::ALWAYS);
		}
		void DeActivate()noexcept
		{
			DeRegisterEvent();
			DeRegisterFrame<CamFrame>();
			DeRegisterCsmTargetInterface();
			PopDrawRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			SetFuncList().InvokeAllReverse(this, true, SetParam(false, true));
			//if (allowDisplayOccCullingDepthMap)
			//	SetAllowDisplayOccCullingDepthMapEx(false, true);

			DestroyAllCullingData();
			DestroyAllTexture(); 
		}
	private:
		void OnEvent(const size_t& senderGuid, const Graphic::J_GRAPHIC_EVENT_TYPE& evType, Graphic::JGraphicEventStruct* evStruct)final
		{
			if (!thisPointer->IsActivated())
				return;

			if (evType == Graphic::J_GRAPHIC_EVENT_TYPE::OPTION_CHANGED)
			{
				auto optEvStruct = static_cast<Graphic::JGraphicOptionChangedEvStruct*>(evStruct);
				bool isPostProcessingChanged = optEvStruct->preOption.IsPostProcessActivated() != optEvStruct->newOption.IsPostProcessActivated();
				bool isRaytracingChanged = optEvStruct->preOption.rendering.allowRaytracing != optEvStruct->newOption.rendering.allowRaytracing;

				bool isRtDependencyChanged = isPostProcessingChanged || isRaytracingChanged;
				if (isRtDependencyChanged)
				{
					constexpr auto rtType = Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON;
					SetFuncList().Invoke(MANAGED_SET_DISPLAY_LIGHT_CULLING_DEBUG, this, SetParam(false, false));
					if (isRtDependencyChanged)
					{
						SetFuncList().InvokePassCondition(MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE, this, SetParam(false, false));
						SetFuncList().InvokePassCondition(MANAGED_SET_GI, this, SetParam(false, false)); 
					}
					if (isPostProcessingChanged)
					{
						SetFuncList().InvokePassCondition(MANAGED_SET_IMAGE_PROCESSING, this, SetParam(false, false));
						SetFuncList().InvokePassCondition(MANAGED_SET_DEFERRED_RESOURCE, this, SetParam(false, false));
						SetFuncList().InvokePassCondition(MANAGED_SET_MAIN_RENDER_TARGET, this, SetParam(false, false));
						
						SetFuncList().Invoke(MANAGED_SET_MAIN_RENDER_TARGET, this, SetParam(true, false));
						SetFuncList().Invoke(MANAGED_SET_DEFERRED_RESOURCE, this, SetParam(true, false));
						SetFuncList().Invoke(MANAGED_SET_IMAGE_PROCESSING, this, SetParam(true, false));
					}
					if (isRtDependencyChanged)
					{ 
						SetFuncList().Invoke(MANAGED_SET_GI, this, SetParam(true, false)); 
						SetFuncList().Invoke(MANAGED_SET_SPATIAL_TEMPORAL_RESOURCE, this, SetParam(true, false));
					}
					SetFuncList().Invoke(MANAGED_SET_DISPLAY_LIGHT_CULLING_DEBUG, this, SetParam(true, false));
				}

				bool isGBufferChanged = optEvStruct->preOption.rendering.allowDeferred != optEvStruct->newOption.rendering.allowDeferred;
				if (isGBufferChanged)
					SetFuncList().InvokePassCondition(MANAGED_SET_DEFERRED_RESOURCE, this, SetParam(optEvStruct->newOption.rendering.allowDeferred, false));
			}
		}
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
	public:
		//has order dependency
		//1. update JDrawSceneCameraConstants
		//2. rest all
		void UpdateFrame(Graphic::JDrawSceneCameraConstants& constant)noexcept final
		{ 
			const XMMATRIX view = mView.LoadXM();
			const XMMATRIX viewProj = XMMatrixMultiply(view, mProj.LoadXM());
			constant.invView.StoreXM(XMMatrixTranspose(XMMatrixInverse(nullptr, view)));
			constant.viewProj.StoreXM(XMMatrixTranspose(viewProj));  
			constant.preViewProj.StoreXM(XMMatrixTranspose(mPreViewProj.LoadXM()));
			constant.renderTargetSize = GetRtSize();
			constant.invRenderTargetSize = JVector2F::One() / constant.renderTargetSize;
			constant.uvToViewA = uvToViewA;
			constant.uvToViewB = uvToViewB;
			constant.eyePosW = thisPointer->GetTransform()->GetPosition();
			constant.nearZ = camNear;
			constant.farZ = camFar;
			if (HasCsmTargetInfo())
				constant.csmLocalIndex = GetCsmTargetIndex();
			constant.hasAoTexture = AllowSsao() && JGraphic::Instance().GetGraphicOptionRef().CanUseSSAO();
			CamFrame::MinusMovedDirty(); 
		}
		void UpdateFrame(Graphic::JDepthTestConstants& constant)noexcept final
		{
			constant.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(mView.LoadXM(), mProj.LoadXM())));
			DepthTestFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JHzbOccComputeConstants& constant, const uint queryCount, const uint queryOffset)noexcept final
		{
			//static const BoundingBox drawBBox = _JResourceManager::Instance().Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE)->GetBoundingBox();
			auto info = JGraphic::Instance().GetGraphicInfo();
			auto option = JGraphic::Instance().GetGraphicOption();

			constant.view.StoreXM(XMMatrixTranspose(mView.LoadXM()));
			constant.proj.StoreXM(XMMatrixTranspose(mProj.LoadXM()));

			const BoundingFrustum frustum = GetBoundingFrustum();
			XMVECTOR planeV[6];
			frustum.GetPlanes(&planeV[0], &planeV[1], &planeV[2], &planeV[3], &planeV[4], &planeV[5]);
			for (uint i = 0; i < 6; ++i)
				XMStoreFloat4(&constant.frustumPlane[i], planeV[i]);

			constant.frustumDir = frustum.Orientation;
			constant.frustumPos = frustum.Origin;
			constant.viewWidth = GetRenderViewWidth();
			constant.viewHeight = GetRenderViewHeight();
			constant.camNear = camNear;
			constant.camFar = camFar;
			constant.validQueryCount = queryCount;
			constant.validQueryOffset = queryOffset;
			constant.occMapCount = info.resource.occlusionMapCount;
			constant.occIndexOffset = JMathHelper::Log2Int(info.resource.occlusionMinSize);
			constant.correctFailTrigger = (int)option.culling.allowHZBCorrectFail;
			constant.usePerspective = true;

			HzbOccReqFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JLightCullingCameraConstants& constant)noexcept final
		{
			constant.camView.StoreXM(XMMatrixTranspose(mView.LoadXM()));
			constant.camProj.StoreXM(XMMatrixTranspose(mProj.LoadXM()));
			constant.camRenderTargetSize = GetRtSize();
			constant.camInvRenderTargetSize = JVector2F::One() / constant.camRenderTargetSize;
			constant.camNearZ = camNear;
			constant.camFarZ = camFar;
			LightCullFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JSsaoConstants& constant)noexcept final
		{
			/*
			if (ssaoDesc.blurKenelSize == Graphic::J_KERNEL_SIZE::_5x5)
			{
				using namespace Graphic;
				JFilter::ComputeFilter<JKenelType::Size<J_KERNEL_SIZE::_5x5>(), JKenelType::MaxSize(), true, true>(constant.kernel, &JFilter::Gaus, ssaoDesc.blurIntensity);
			}
			else
			{
				using namespace Graphic;
				JFilter::ComputeFilter<JKenelType::Size<J_KERNEL_SIZE::_3x3>(), JKenelType::MaxSize(), true, true>(constant.kernel, &JFilter::Gaus, ssaoDesc.blurIntensity);
			}
			*/
			constant.camView.StoreXM(XMMatrixTranspose(mView.LoadXM()));
			constant.camProj.StoreXM(XMMatrixTranspose(mProj.LoadXM()));

			constant.radius = ssaoDesc.radius;
			constant.radius2 = constant.radius * constant.radius;
			constant.bias = ssaoDesc.bias;
			constant.sharpness = ssaoDesc.sharpness;

			//x camRenderTargetWidth, y camRenderTargetHeight, z near, w far,
			constant.camNearFar = JVector2F(camNear, camFar);

			constant.camRtSize = GetRtSize();
			constant.camInvRtSize = JVector2F::One() / constant.camRtSize;

			constant.aoRtSize = constant.camRtSize;
			constant.aoInvRtSize = constant.camInvRtSize;

			const JVector2F aoQuaterRtSize = (constant.camRtSize + 4.0f - 1.0f) / 4.0f;
			constant.aoInvQuaterRtSize = JVector2F(1.0f / aoQuaterRtSize.x, 1.0f / aoQuaterRtSize.y);

			float radiusToScreen = constant.radius * 0.5f / tanHalfFovY * GetRtSize().y;
			constant.radiusToScreen = radiusToScreen;

			constant.uvToViewA = uvToViewA;
			constant.uvToViewB = uvToViewB;

			constant.negInvR2 = -1.0f / constant.radius2;
			//constant.tanBias = tan(30.0f * JMathHelper::DegToRad);		 
			constant.tanBias = std::clamp(constant.bias, tan(-45.0f * JMathHelper::DegToRad), tan(45.0f * JMathHelper::DegToRad));

			const float AOAmountScaleFactor = 1.0f / (1.0f - constant.tanBias);
			constant.smallScaleAOAmount = std::clamp(ssaoDesc.smallAoScale, Private::minSsaoAoScale, Private::maxSsaoAoScale) * AOAmountScaleFactor * 2.0f;
			constant.largeScaleAOAmount = std::clamp(ssaoDesc.largeAoScale, Private::minSsaoAoScale, Private::maxSsaoAoScale) * AOAmountScaleFactor;
			constant.viewPortTopLeft = JVector2F::Zero();
			constant.camNearMulFar = camNear * camFar;
			//constant.depthThresholdNegInv = -1.0f / JMathHelper::Epsilon;
			//constant.depthThresholdSharpness = Max(Params.Sharpness, 0.f);

			SsaoFrame::MinusMovedDirty();
		}
		void UpdateViewMatrix() noexcept
		{
			mPreViewProj.StoreXM(XMMatrixMultiply(mView.LoadXM(), mProj.LoadXM()));
			if (IsFrameHotDirted())
				thisPointer->GetTransform()->CalTransformMatrix(mView);
 
			//test 
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
		void UpdateProjMatrixDependency()
		{
			tanHalfFovX = 1.0f / fabs(mProj(0, 0));
			tanHalfFovY = 1.0f / fabs(mProj(1, 1));

			//uv -> view = z * (2.0f * -1.0f) * (inv proj) * screen coord
			//mul z는 shader에서 수행
			uvToViewA.x = 2.0f * tanHalfFovX;
			uvToViewA.y = -2.0f * tanHalfFovY;
			uvToViewB.x = -1.0f * tanHalfFovX;
			uvToViewB.y = 1.0f * tanHalfFovY;
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
			auto transform = thisPointer->GetOwner()->GetTransform();
			if (transform.IsValid())
			{
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(transform.Get(), thisPointer->GetGuid());
				JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(transform.Get(), this, thisPointer->GetGuid());
			}
			RegisterInterfacePointer();
			TrySetGetFrustumPtr();

			CamFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, (CamFrame*)this);
			DepthTestFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, (DepthTestFrame*)this);
			HzbOccReqFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, (HzbOccReqFrame*)this);
			LightCullFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_CULLING_PASS, (LightCullFrame*)this);
			SsaoFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, (SsaoFrame*)this);

			Graphic::GraphicEventListener::ResetEventListenerPointer(*JGraphic::Instance().EventInterface(), thisPointer->GetGuid());
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
		void RegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(this);
			Graphic::JCullingInterface::SetInterfacePointer(this);
			Graphic::JCsmTargetInterface::SetInterfacePointer(this);
		}
		void RegisterPostCreation()
		{
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), this, thisPointer->GetGuid());
		}
		template<typename T>
		void RegisterFrame(const ushort indexSize = 1)
		{
			if constexpr (std::is_base_of_v<JFrameUpdateData, T>)
				this->T::Register(thisPointer->GetOwner()->GetOwnerGuid(), indexSize);
		}
		void RegisterEvent()
		{
			Graphic::GraphicEventListener::AddEventListener(*JGraphic::Instance().EventInterface(), thisPointer->GetGuid(), Graphic::J_GRAPHIC_EVENT_TYPE::OPTION_CHANGED);
			if (CanResizeTarget())
				WindowEventListener::AddEventListener(*JWindow::EvInterface(), thisPointer->GetGuid(), Window::J_WINDOW_EVENT::WINDOW_RESIZE);
		}
		void DeRegisterPreDestruction()
		{
			if (thisPointer->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), thisPointer->GetGuid());
		}
		template<typename T>
		void DeRegisterFrame()
		{
			if constexpr (std::is_base_of_v<JFrameUpdateData, T>)
				this->T::DeRegister();
		}
		void DeRegisterEvent()
		{
			Graphic::GraphicEventListener::RemoveEventListener(*JGraphic::Instance().EventInterface(), thisPointer->GetGuid(), Graphic::J_GRAPHIC_EVENT_TYPE::OPTION_CHANGED);
			if (CanResizeTarget())
				WindowEventListener::RemoveEventListener(*JWindow::EvInterface(), thisPointer->GetGuid(), Window::J_WINDOW_EVENT::WINDOW_RESIZE);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JCamera::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
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

			static auto setFrameLam = [](JComponent* component) {static_cast<JCamera*>(component)->impl->SetFrameDirty(); };
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable, isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

			JComponent::RegisterCTypeInfo(JCamera::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JCamera::StaticTypeInfo(), cPrivate);

			IMPL_REALLOC_BIND(JCamera::JCameraImpl, thisPointer)
				 
			auto setMainRtLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
				{
					Graphic::JGraphicResourceCreationDesc rtDesc(impl->GetRtSize());
					const bool canBindUav = impl->OnImageProcessingTrigger() || impl->OnGITrigger();
					if (canBindUav)
					{
						rtDesc.bindDesc.useEngineDefinedBindType = false;
						rtDesc.bindDesc.requestAdditionalBind[(uint)Graphic::J_GRAPHIC_BIND_TYPE::UAV] = true;
					}
					impl->CreateResource(rtDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
				}
				else
					impl->DestroyMultiTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Private::mainRtNumber);
				impl->SetFrameDirty();
			};
			auto setMainDsLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
					impl->CreateResource(impl->GetRtSize(), Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);
				else
					impl->DestroyMultiTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, Private::mainDsNumber);
				impl->SetFrameDirty();
			};
			auto setDisplayRsLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
				{
					for (uint i = 0; i < Private::debuggingMapCount; ++i)
						impl->CreateResource(impl->GetRtSize(), Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP);
				}
				else
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP);
				impl->SetFrameDirty();
			};
			auto setDisplayDebugObjLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
					impl->CreateResource(impl->GetRtSize(), Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL);
				else
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL);
				impl->SetFrameDirty();
			};
			auto setDisplayOccDepthMapLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
				{
					auto gUser = impl->GetGraphicInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, 0);
					if (gUser != nullptr)
						return;

					auto gInfo = JGraphic::Instance().GetGraphicInfo();
					Graphic::JGraphicResourceCreationDesc desc(JVector2F(gInfo.resource.occlusionWidth, gInfo.resource.occlusionHeight));
					if (impl->allowHzbOcclusionCulling)
						desc.bindDesc.allowMipmapBind = true;
					impl->CreateResource(desc, Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
				}
				else
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
				impl->SetFrameDirty();
			};
			auto setDisplayLightCullLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
					impl->CreateResource(impl->GetRtSize(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
				else
					impl->DestroyMultiTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Private::litCullDebugRtNumber);
				impl->SetFrameDirty();
			};
			auto setFrustumCullLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
				{
					impl->CreateFrustumCullingData(Graphic::J_CULLING_TARGET::RENDERITEM);
					impl->AddFrustumCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
				}
				else
				{
					impl->DestroyCullingData(Graphic::J_CULLING_TYPE::FRUSTUM, Graphic::J_CULLING_TARGET::RENDERITEM);
					impl->PopFrustumCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer);
				}
				impl->SetFrameDirty();
			};
			auto setHzbCullLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
				{
					auto gInfo = JGraphic::Instance().GetGraphicInfo();
					impl->RegisterFrame<DepthTestFrame>();
					impl->RegisterFrame<HzbOccReqFrame>();
					impl->CreateResource(JVector2F(gInfo.resource.occlusionWidth, gInfo.resource.occlusionHeight), Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
					impl->CreateResource(JVector2F(gInfo.resource.occlusionWidth, gInfo.resource.occlusionHeight), Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
					impl->CreateHzbOccCullingData();
					AddHzbOccCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);

					auto mipmapInfo = impl->GetGraphicInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, 0);
					auto debugInfo = impl->GetGraphicInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, 0);

					if (debugInfo == nullptr)
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
					else if (mipmapInfo->GetViewCount(Graphic::J_GRAPHIC_BIND_TYPE::SRV) != debugInfo->GetViewCount(Graphic::J_GRAPHIC_BIND_TYPE::SRV))
					{
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
					}
				}
				else
				{
					if (p.isCalledByAct || (!impl->AllowHzbOcclusionCulling() && !impl->AllowHdOcclusionCulling()))
						impl->DeRegisterFrame<DepthTestFrame>();

					impl->DeRegisterFrame<HzbOccReqFrame>();
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
					impl->DestroyCullingData(Graphic::J_CULLING_TYPE::HZB_OCCLUSION, Graphic::J_CULLING_TARGET::RENDERITEM);
					PopHzbOccCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer);
					if (!impl->AllowHdOcclusionCulling())
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
				}
				impl->SetFrameDirty();
			};
			auto setHdCullLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
				{
					impl->RegisterFrame<DepthTestFrame>();
					impl->CreateHdOccCullingData();
					AddHdOccCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
					if (impl->AllowDisplayOccCullingDepthMap())
					{
						auto debugInfo = impl->GetGraphicInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, 0);
						if (debugInfo == nullptr)
							impl->SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
						else if (debugInfo->GetViewCount(Graphic::J_GRAPHIC_BIND_TYPE::SRV) > 1)
						{
							impl->SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
							impl->SetFuncList().InvokePassCondition(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(true, p.isCalledByAct));
						}
					}
				}
				else
				{
					if (p.isCalledByAct || (!impl->AllowHzbOcclusionCulling() && !impl->AllowHdOcclusionCulling()))
						impl->DeRegisterFrame<DepthTestFrame>();
					impl->DestroyCullingData(Graphic::J_CULLING_TYPE::HD_OCCLUSION, Graphic::J_CULLING_TARGET::RENDERITEM);
					PopHdOccCullingRequest(impl->thisPointer->GetOwner()->GetOwnerScene(), impl->thisPointer);
					if (!impl->AllowHzbOcclusionCulling())
						impl->SetFuncList().Invoke(MANAGED_SET_DISPLAY_OCC_CULLING_DEPTH_MAP, impl, SetParam(false, p.isCalledByAct));
				}
				impl->SetFrameDirty();
			};
			auto setLitCullLam = [](JCameraImpl* impl, SetParam p)
			{
				if (!impl->thisPointer->GetOwner()->GetOwnerScene()->AllowLightCulling())
					return;

				if (p.value)
				{
					impl->RegisterFrame<LightCullFrame>();
					impl->CreateFrustumCullingData(Graphic::J_CULLING_TARGET::LIGHT);

					auto gInfo = JGraphic::Instance().GetGraphicInfo();
					auto gOption = JGraphic::Instance().GetGraphicOption();
					Graphic::JGraphicResourceCreationDesc creationDesc;
					creationDesc.width = gOption.GetClusterIndexCount();
					creationDesc.height = 1;
					creationDesc.arraySize = 1;
					impl->CreateResource(creationDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST);

					creationDesc.width = gOption.GetClusterTotalCount();
					impl->CreateResource(creationDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET);
				}
				else
				{
					impl->DeRegisterFrame<LightCullFrame>();
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET);
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST);
					impl->DestroyCullingData(Graphic::J_CULLING_TYPE::FRUSTUM, Graphic::J_CULLING_TARGET::LIGHT);
				}
				impl->SetFrameDirty();
			};
			auto setSsaoLam = [](JCameraImpl* impl, SetParam p)
			{
				if (p.value)
				{
					impl->RegisterFrame<SsaoFrame>();
					impl->CreateResource(impl->GetRtSize(), Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP);
				}
				else
				{
					impl->DeRegisterFrame<SsaoFrame>();
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP);
				}
				impl->SetFrameDirty();
			};
			auto setImageProcessingRtLam = [](JCameraImpl* impl, SetParam p)
			{
				constexpr auto rType = Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON;
				constexpr auto taskType = Graphic::J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT; 
				if (p.value)
				{
					Graphic::JGraphicResourceCreationDesc rtDesc(impl->GetRtSize());
					rtDesc.bindDesc.useEngineDefinedBindType = false;
					rtDesc.bindDesc.requestAdditionalBind[(uint)Graphic::J_GRAPHIC_BIND_TYPE::UAV] = true;
					impl->CreateResource(rtDesc, rType);

					Graphic::JGraphicResourceCreationDesc exposureDesc;
					exposureDesc.useEngineDefine = true;
					exposureDesc.uploadBufferDesc = std::make_unique<Graphic::JUploadBufferCreationDesc>();
					exposureDesc.uploadBufferDesc->useEngineDefine = true;
					impl->CreateResource(exposureDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE);
				}
				else
				{
					impl->DestroyMultiTexture(rType, impl->GetResourceDataIndex(rType, taskType));
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE);
				}
				impl->SetFrameDirty();
			};
			auto setGIRtLam = [](JCameraImpl* impl, SetParam p)
			{
				constexpr auto rtType = Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON;
				constexpr auto taskType = Graphic::J_GRAPHIC_TASK_TYPE::RAYTRACING_GI; 
				if (p.value)
				{
					Graphic::JGraphicResourceCreationDesc rtDesc(impl->GetRtSize());
					rtDesc.bindDesc.useEngineDefinedBindType = false;
					rtDesc.bindDesc.requestAdditionalBind[(uint)Graphic::J_GRAPHIC_BIND_TYPE::UAV] = true;
					impl->CreateResource(rtDesc, rtType);

					Graphic::JGraphicResourceCreationDesc reserviorDesc;
					reserviorDesc.width = rtDesc.width * rtDesc.height;
					impl->CreateResource(reserviorDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE);
					impl->CreateResource(reserviorDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR);
					impl->CreateResource(reserviorDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR);
					impl->CreateResource(reserviorDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR);
					impl->CreateResource(reserviorDesc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR); 
				}
				else
				{ 
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE);
					impl->DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR);
					impl->DestroyMultiTexture(rtType, impl->GetResourceDataIndex(rtType, taskType));
				}
				impl->SetFrameDirty();
			};
			auto setDeferredLam = [](JCameraImpl* impl, SetParam p)
			{
				auto gUser = impl->GetGraphicInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Private::mainRtNumber);
				if (gUser == nullptr)
					return;

				if (p.value)
				{
					if (gUser->HasOption(Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP))
						return;

					impl->CreateOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP); 
					impl->CreateOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY); 
					impl->CreateOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
				}
				else
				{ 
					impl->DestroyGraphicOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
					impl->DestroyGraphicOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
					impl->DestroyGraphicOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP);
				}
				impl->SetFrameDirty();
			};
			auto setSpatialTemporalResourceLam = [](JCameraImpl* impl, SetParam p)
			{
				auto rType = Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON;
				auto taskType = Graphic::J_GRAPHIC_TASK_TYPE::RAYTRACING_GI;
				auto mainRsUser = impl->GetGraphicInfo(rType, Private::mainRtNumber);
				auto rtRsUser = impl->GetGraphicInfo(rType, impl->GetResourceDataIndex(rType, taskType));
				if (mainRsUser == nullptr || rtRsUser == nullptr)
					return;

				if (p.value)
				{
					if (!mainRsUser->HasOption(Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY))
						impl->CreateOption(mainRsUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
					if (!rtRsUser->HasOption(Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP))
					{
						impl->CreateResource(impl->GetRtSize(), Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);
						impl->CreateOption(rtRsUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
						impl->CreateOption(rtRsUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
					}
				}
				else
				{
					impl->DestroyGraphicOption(rtRsUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
					impl->DestroyGraphicOption(rtRsUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
					impl->DestroyMultiTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, Private::previousDsNumber);

					impl->DestroyGraphicOption(mainRsUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
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
			SetFuncList().Register(std::make_unique<SetCallable>(setImageProcessingRtLam), std::make_unique<CondCallable>(&JCameraImpl::OnImageProcessingTrigger), MANAGED_SET_IMAGE_PROCESSING);
			SetFuncList().Register(std::make_unique<SetCallable>(setGIRtLam), std::make_unique<CondCallable>(&JCameraImpl::OnGITrigger), MANAGED_SET_GI);

			SetFuncList().Register(std::make_unique<SetCallable>(setDeferredLam), std::make_unique<CondCallable>(&JCameraImpl::AllowBuildGBuffer), MANAGED_SET_DEFERRED_RESOURCE);
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

			auto getDrawSceneFrameLam = [](JCamera* cam)->JFrameUpdateData* {return (CamFrame*)(cam->impl.get()); };
			auto getDepthTestFrameLam = [](JCamera* cam)->JFrameUpdateData* {return (DepthTestFrame*)(cam->impl.get()); };
			auto getHzbFrameLam = [](JCamera* cam)->JFrameUpdateData* {return (HzbOccReqFrame*)(cam->impl.get()); };
			auto getLightCullFrameLam = [](JCamera* cam)->JFrameUpdateData* {return (LightCullFrame*)(cam->impl.get()); };
			auto getSsaoFrameLam = [](JCamera* cam)->JFrameUpdateData* {return (SsaoFrame*)(cam->impl.get()); };

			Private::getFrameDataPtr[Graphic::CameraFrameLayer::drawScene] = getDrawSceneFrameLam;
			Private::getFrameDataPtr[Graphic::CameraFrameLayer::depthTest] = getDepthTestFrameLam;
			Private::getFrameDataPtr[Graphic::CameraFrameLayer::hzb] = getHzbFrameLam;
			Private::getFrameDataPtr[Graphic::CameraFrameLayer::lightCulling] = getLightCullFrameLam;
			Private::getFrameDataPtr[Graphic::CameraFrameLayer::ssao] = getSsaoFrameLam;
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
		return cPrivate;
	}
	const Graphic::JGraphicResourceUserInterface JCamera::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface{ impl.get() };
	}
	const Graphic::JCullingUserInterface JCamera::CullingUserInterface()const noexcept
	{
		return Graphic::JCullingUserInterface{ impl.get() };
	}
	J_COMPONENT_TYPE JCamera::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JUserPtr<JTransform> JCamera::GetTransform()noexcept
	{
		return GetOwner()->GetTransform();
	}
	XMMATRIX JCamera::GetView()const noexcept
	{
		return impl->mView.LoadXM();
	}
	JMatrix4x4 JCamera::GetView4x4()const noexcept
	{
		return impl->mView;
	}
	DirectX::XMMATRIX JCamera::GetInvView()const noexcept
	{
		return XMMatrixInverse(nullptr, impl->mView.LoadXM());
	} 
	XMMATRIX JCamera::GetProj()const noexcept
	{
		return impl->mProj.LoadXM();
	}
	JMatrix4x4 JCamera::GetProj4x4()const noexcept
	{
		return impl->mProj;
	}
	DirectX::XMMATRIX JCamera::GetPreViewProj()const noexcept
	{
		return impl->mPreViewProj.LoadXM();
	}
	void JCamera::GetUvToView(JVector2F& a, JVector2F& b)const noexcept
	{
		a = impl->uvToViewA;
		b = impl->uvToViewB;
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
	Graphic::JSsaoDesc JCamera::GetSsaoDesc()const noexcept
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
	void JCamera::SetSsaoDesc(const Graphic::JSsaoDesc& desc)noexcept
	{
		impl->SetSsaoDesc(desc);
	}
	bool JCamera::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
	}
	bool JCamera::IsOrthoCamera()const noexcept
	{
		return impl->isOrtho;
	}
	bool JCamera::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
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
	bool JCamera::AllowSsao()const noexcept
	{
		return impl->AllowSsao();
	}
	bool JCamera::AllowPostProcess()const noexcept
	{
		return impl->OnImageProcessingTrigger();
	}
	bool JCamera::AllowRaytracingGI()const noexcept
	{
		return impl->OnGITrigger();
	}
	void JCamera::DoActivate()noexcept
	{
		//Caution 
		//Activate와 RegisterComponent는 순서에 종속성을 가진다.
		//RegisterComponent는 Scene과 가속구조에 Component에 대한 정보를 추가하는 작업으로
		//Activate Process중에 자기자신과 관련된 Scene component vector, Scene As관련 data에 대한 호출은 에러를 일으킬 수 있다.
		JComponent::DoActivate();
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			impl->Activate();
			RegisterComponent(impl->thisPointer);
		}
		impl->SetFrameDirty();
	}
	void JCamera::DoDeActivate()noexcept
	{
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			DeRegisterComponent(impl->thisPointer);
			impl->DeActivate();
		}
		impl->OffFrameDirty();
		JComponent::DoDeActivate();
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
	using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JCameraPrivate::FrameIndexInterface;
	using EditorSettingInterface = JCameraPrivate::EditorSettingInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JCamera>(*static_cast<JCamera::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JCamera* cam = static_cast<JCamera*>(createdPtr);
		cam->impl->RegisterThisPointer(cam);
		cam->impl->RegisterInterfacePointer();
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
		Graphic::JSsaoDesc ssaoDesc;

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

		auto idenUser = cPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &cPrivate);
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

	bool FrameUpdateInterface::UpdateStart(JCamera* cam, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			cam->impl->SetFrameDirty();

		cam->impl->SetLastFrameUpdatedTrigger(false);
		cam->impl->SetLastFrameHotUpdatedTrigger(false);
		if (cam->impl->IsFrameDirted())
			cam->impl->UpdateViewMatrix();
		return cam->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JCamera* cam, Graphic::JCameraConstantsSet& set)noexcept
	{
		auto impl = cam->impl.get();
		if (set.updateStart)
		{
			impl->UpdateFrame(set.drawScene);
			set.SetUpdated(Graphic::CameraFrameLayer::drawScene, impl->CamFrame::GetFrameIndex());

			if (impl->DepthTestFrame::HasValidFrameIndex())
			{
				impl->UpdateFrame(set.depthTest);
				set.SetUpdated(Graphic::CameraFrameLayer::depthTest, impl->DepthTestFrame::GetFrameIndex());
			}
			if (impl->HzbOccReqFrame::HasValidFrameIndex())
			{
				impl->UpdateFrame(set.hzb, set.hzbQueryCount, set.hzbQueryOffset);
				set.SetUpdated(Graphic::CameraFrameLayer::hzb, impl->HzbOccReqFrame::GetFrameIndex());
			}
			if (impl->LightCullFrame::HasValidFrameIndex())
			{
				impl->UpdateFrame(set.lightCulling);
				set.SetUpdated(Graphic::CameraFrameLayer::lightCulling, impl->LightCullFrame::GetFrameIndex());
			}
			if (impl->SsaoFrame::HasValidFrameIndex())
			{
				impl->UpdateFrame(set.ssao);
				set.SetUpdated(Graphic::CameraFrameLayer::ssao, impl->SsaoFrame::GetFrameIndex());
			}
		}
		else
		{
			if (impl->CamFrame::HasMovedDirty())
			{
				impl->UpdateFrame(set.drawScene);
				set.SetUpdated(Graphic::CameraFrameLayer::drawScene, impl->CamFrame::GetFrameIndex());
			}
			if (impl->DepthTestFrame::HasMovedDirty())
			{
				impl->UpdateFrame(set.depthTest);
				set.SetUpdated(Graphic::CameraFrameLayer::depthTest, impl->DepthTestFrame::GetFrameIndex());
			}
			if (impl->HzbOccReqFrame::HasMovedDirty())
			{
				impl->UpdateFrame(set.hzb, set.hzbQueryCount, set.hzbQueryOffset);
				set.SetUpdated(Graphic::CameraFrameLayer::hzb, impl->HzbOccReqFrame::GetFrameIndex());
			}
			if (impl->LightCullFrame::HasMovedDirty())
			{
				impl->UpdateFrame(set.lightCulling);
				set.SetUpdated(Graphic::CameraFrameLayer::lightCulling, impl->LightCullFrame::GetFrameIndex());
			}
			if (impl->SsaoFrame::HasMovedDirty())
			{
				impl->UpdateFrame(set.ssao);
				set.SetUpdated(Graphic::CameraFrameLayer::ssao, impl->SsaoFrame::GetFrameIndex());
			}
		}
	}
	void FrameUpdateInterface::UpdateEnd(JCamera* cam)noexcept
	{
		if (cam->impl->IsFrameHotDirted())
			cam->impl->SetLastFrameHotUpdatedTrigger(true);
		cam->impl->SetLastFrameUpdatedTrigger(true);
		cam->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetFrameIndex(JCamera* cam, const uint layerIndex)noexcept
	{
		return Private::getFrameDataPtr[layerIndex](cam)->GetFrameIndex();
	}
	bool FrameUpdateInterface::IsLastFrameHotUpdated(JCamera* cam)noexcept
	{
		return cam->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JCamera* cam)noexcept
	{
		return cam->impl->IsLastFrameUpdated();
	}

	int FrameIndexInterface::GetFrameIndex(JCamera* cam, const uint layerIndex)noexcept
	{
		return Private::getFrameDataPtr[layerIndex](cam)->GetFrameIndex();
	}

	void EditorSettingInterface::SetAllowAllCullingResult(const JUserPtr<JCamera>& cam, const bool value)noexcept
	{
		cam->impl->SetAllowAllCullingResult(value);
	}
	bool EditorSettingInterface::AllowAllCullingResult(const JUserPtr<JCamera>& cam)noexcept
	{
		return cam->impl->AllowAllCullingResult();
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
