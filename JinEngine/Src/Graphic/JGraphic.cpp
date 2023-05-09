#pragma once
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include"JGraphic.h" 
#include"JGraphicPrivate.h" 
#include"JGraphicConstants.h"
#include"JGraphicDrawList.h" 
#include"GraphicResource/JGraphicResourceManager.h"
#include"GraphicResource/JGraphicResourceInfo.h"
#include"GraphicResource/JGraphicResourceInterface.h"
#include"Utility/JDepthMapDebug.h"
#include"OcclusionCulling/JHardwareOccCulling.h"
#include"OcclusionCulling/JHZBOccCulling.h"
#include"Outline/JOutline.h"

#include"../Window/JWindow.h"
#include"../Window/JWindowPrivate.h"
#include"FrameResource/JFrameResource.h"
#include"FrameResource/JFrameResource.h" 
#include"FrameResource/JObjectConstants.h" 
#include"FrameResource/JAnimationConstants.h" 
#include"FrameResource/JMaterialConstants.h" 
#include"FrameResource/JPassConstants.h" 
#include"FrameResource/JCameraConstants.h" 
#include"FrameResource/JLightConstants.h" 
#include"FrameResource/JShadowMapConstants.h" 
#include"FrameResource/JBoundingObjectConstants.h"

#include"../Core/Time/JGameTimer.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/File/JFileConstant.h"
#include"../Core/Exception/JExceptionMacro.h"	
#include"../Core/Memory/JMemoryCapture.h"	
#include"../Core/Unit/JByteUnit.h"	
#include"../Object/GameObject/JGameObject.h"
#include"../Object/Component/RenderItem/JRenderItem.h"
#include"../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../Object/Component/Transform/JTransform.h"
#include"../Object/Component/Camera/JCamera.h"
#include"../Object/Component/Camera/JCameraPrivate.h"
#include"../Object/Component/Animator/JAnimator.h"
#include"../Object/Component/Animator/JAnimatorPrivate.h"
#include"../Object/Component/Light/JLight.h"
#include"../Object/Component/Light/JLightPrivate.h"

#include"../Object/Resource/JResourceObjectDefualtData.h"
#include"../Object/Resource/JResourceManager.h"
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Resource/Scene/JScenePrivate.h"
#include"../Object/Resource/Mesh/JMeshGeometry.h"
#include"../Object/Resource/Mesh/JMeshGeometryPrivate.h"
#include"../Object/Resource/Material/JMaterial.h"
#include"../Object/Resource/Material/JMaterialPrivate.h"
#include"../Object/Resource/Shader/JShader.h"
#include"../Object/Resource/Shader/JShaderPrivate.h"
#include"../Object/Resource/Shader/JShaderData.h"
#include"../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../Object/Resource/Scene/JSceneManager.h"
#include"../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"
#include"../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../Object/Resource/Texture/JTexture.h" 

#include"../Application/JApplicationEngine.h"
#include"../Application/JApplicationEnginePrivate.h"
#include"../Application/JApplicationProject.h"
#include"../Utility/JCommonUtility.h" 
#include"../Editor/GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../Lib/imgui/imgui_impl_dx12.h"
#include"../../Lib/imgui/imgui_impl_win32.h"
#include"../../Lib/DirectX/TK/Inc/ScreenGrab.h"

#include<DirectXColors.h>
#include<functional>
#include <dxgidebug.h>
#include<WindowsX.h>
#include<wrl/client.h>
#include<memory>
#include<dxgi1_4.h>
#include<d3d12.h>
#include<vector>
#include<unordered_map>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

//#define DEBUG 
// 
//#include"../Core/File/JFileIOHelper.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Graphic
	{
		namespace
		{
			using CamEditorSettingInterface = JCameraPrivate::EditorSettingInterface;
		}
#pragma region Helper
		struct JGraphic::UpdateHelper
		{
		public:
			using GetElementCountT = Core::JStaticCallableType<uint>;
			using NotifyUpdateCapacityT = Core::JStaticCallableType<void, JGraphic&>;
		public:
			using GetTextureCountT = Core::JStaticCallableType<uint, const JGraphic&>;
			using GetTextureCapacityT = Core::JStaticCallableType<uint, const JGraphic&>;
			using SetCapacityT = Core::JStaticCallableType<void, JGraphic&>;
		public:
			struct UploadData
			{
			public:
				std::unique_ptr<GetElementCountT::Callable> getElementCountCallable = nullptr;
				std::vector<std::unique_ptr<NotifyUpdateCapacityT::Callable>> notifyUpdateCapacityCallable;
			public:
				uint count = 0;
				uint capacity = 0;
				uint offset = 0;
				bool setFrameDirty = false;
				J_UPLOAD_CAPACITY_CONDITION rebuildCondition;
			};
			struct BindingTextureData
			{
			public:
				std::unique_ptr<GetTextureCountT::Callable> getTextureCountCallable = nullptr;
				std::unique_ptr<GetTextureCapacityT::Callable> getTextureCapacityCallable = nullptr;
				std::unique_ptr< SetCapacityT::Callable> setCapacityCallable = nullptr;
			public:
				uint count = 0;
				uint capacity = 0;
				J_UPLOAD_CAPACITY_CONDITION recompileCondition;
				bool hasCallable = false;
			public:
				bool HasCallable()const noexcept
				{
					return hasCallable;
				}
			};
		public:
			std::vector<UploadData> uData;
			std::vector<BindingTextureData> bData;
			bool hasRebuildCondition;
			bool hasRecompileShader;
		public:
			void Clear()
			{
				const uint fVCount = (uint)uData.size();
				for (uint i = 0; i < fVCount; ++i)
				{
					uData[i].count = 0;
					uData[i].capacity = 0;
					uData[i].offset = 0;
					uData[i].setFrameDirty = false;
					uData[i].rebuildCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
				}

				const uint bVCount = (uint)bData.size();
				for (uint i = 0; i < bVCount; ++i)
				{
					bData[i].count = 0;
					bData[i].capacity = 0;
					bData[i].recompileCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
				}
				hasRebuildCondition = false;
				hasRecompileShader = false;
			}
			void RegisterCallable(J_UPLOAD_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr)
			{
				if (getCountPtr == nullptr)
					return;

				uData[(int)type].getElementCountCallable = std::make_unique<GetElementCountT::Callable>(*getCountPtr);
				//uData[(int)type].getElementCapacityCallable = std::make_unique<GetElementCapacityT::Callable>(*getCapaPtr);
				//uData[(int)type].rebuildCallable = std::make_unique<RebuildT::Callable>(*rPtr);
			}
			void RegisterCallable(J_GRAPHIC_RESOURCE_TYPE type, GetTextureCountT::Ptr* getCountPtr, GetTextureCapacityT::Ptr* getCapaPtr, SetCapacityT::Ptr* sPtr)
			{
				if (getCountPtr == nullptr || getCapaPtr == nullptr || sPtr == nullptr)
					return;

				bData[(int)type].getTextureCountCallable = std::make_unique<GetTextureCountT::Callable>(*getCountPtr);
				bData[(int)type].getTextureCapacityCallable = std::make_unique< GetTextureCapacityT::Callable>(*getCapaPtr);
				bData[(int)type].setCapacityCallable = std::make_unique<SetCapacityT::Callable>(*sPtr);
				bData[(int)type].hasCallable = true;
			}
			void RegisterListener(J_UPLOAD_RESOURCE_TYPE type, std::unique_ptr<NotifyUpdateCapacityT::Callable>&& listner)
			{
				uData[(int)type].notifyUpdateCapacityCallable.push_back(std::move(listner));
			}
			void WriteGraphicInfo(JGraphicInfo& info)const noexcept
			{
				info.upObjCount = uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].count;
				info.upPassCount = uData[(int)J_UPLOAD_RESOURCE_TYPE::PASS].count;
				info.upAniCount = uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].count;
				info.upCameraCount = uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].count;
				info.upLightCount = uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].count;
				info.upSmLightCount = uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].count;
				info.upMaterialCount = uData[(int)J_UPLOAD_RESOURCE_TYPE::MATERIAL].count;

				info.upObjCapacity = uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].capacity;
				info.upPassCapacity = uData[(int)J_UPLOAD_RESOURCE_TYPE::PASS].capacity;
				info.upAniCapacity = uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].capacity;
				info.upCameraCapacity = uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].capacity;
				info.upLightCapacity = uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].capacity;
				info.upSmLightCapacity = uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].capacity;
				info.upMaterialCapacity = uData[(int)J_UPLOAD_RESOURCE_TYPE::MATERIAL].capacity;

				info.binding2DTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].count;
				info.bindingCubeMapCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].count;
				info.bindingShadowTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].count;

				info.binding2DTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].capacity;
				info.bindingCubeMapCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].capacity;
				info.bindingShadowTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].capacity;
			}
			void NotifyUpdateFrameCapacity(JGraphic& grpahic)
			{
				for (uint i = 0; i < (uint)J_UPLOAD_RESOURCE_TYPE::COUNT; ++i)
				{
					if (uData[i].rebuildCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
					{
						const uint listenerCount = (uint)uData[i].notifyUpdateCapacityCallable.size();
						for (uint j = 0; j < listenerCount; ++j)
							(*uData[i].notifyUpdateCapacityCallable[j])(nullptr, grpahic);
					}
				}
			}
		};
		//draw data
		struct JGraphic::DrawHelper
		{
		public:
			JUserPtr<JScene> scene = nullptr;
			JUserPtr<JCamera> cam = nullptr;
			JUserPtr<JLight> lit = nullptr;
		public:
			uint objectMeshOffset = 0;
			uint objectRitemOffset = 0;
			uint passOffset = 0;
			uint aniOffset = 0;
			uint camOffset = 0;
			uint litIndexOffset = 0;
			uint shadowOffset = 0;
		public:
			//draw outline and debug layer object
			bool allowDrawDebug = false;
			//execute frustum and occlusion culling
			bool allowCulling = true;
			//record scene culling result per camera for check space spatial result
			bool allowRecordCullingResult = JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME;
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE mainDsv;
			CD3DX12_CPU_DESCRIPTOR_HANDLE mainRtv;
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE editorDsv;
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE* dsvArr[2];	// main .. editor
			CD3DX12_CPU_DESCRIPTOR_HANDLE* rtvArr[2];	// main .. editor
		public:
			void StuffHandleArray()noexcept
			{
				dsvArr[0] = &mainDsv;
				dsvArr[1] = &editorDsv;
				rtvArr[0] = &mainRtv;
				rtvArr[1] = nullptr;
			}
		};
		//draw detail condition
		struct JGraphic::DrawCondition
		{
		public:
			//Draw 
			bool allowAnimation = false;
			//Culling part
		public:
			bool allowFrustumCulling = false;
			bool allowHZBOcclusionCulling = false;
			bool allowHDOcclusionCulling = false;
			//Debug part
		public:
			bool allowDebugOutline = false;
			bool allowAllCullingResult = false;	//for check other cam spacespatial 
		public:
			DrawCondition() = default;
			DrawCondition(const JGraphicOption& option,
				const DrawHelper& helper,
				const bool newAllowAnimation,
				const bool newAllowOcclusionCulling,
				const bool newAllowDebugOutline)
				:allowAnimation(allowAnimation),
				allowAllCullingResult(CamEditorSettingInterface::AllowAllCullingResult(helper.cam))
			{
				allowAnimation = newAllowAnimation;
				allowFrustumCulling = helper.allowCulling;
				allowHZBOcclusionCulling = newAllowOcclusionCulling &&
					helper.allowCulling &&
					option.isOcclusionQueryActivated && option.isHZBOcclusionActivated;
				allowHDOcclusionCulling = newAllowOcclusionCulling &&
					helper.allowCulling &&
					option.isOcclusionQueryActivated && option.isHDOcclusionAcitvated;
				allowDebugOutline = newAllowDebugOutline && helper.allowDrawDebug && option.allowDebugOutline;
			}
		};
#pragma endregion

#pragma region Impl
		class JGraphic::JGraphicImpl : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>
		{
		public:
			JGraphic* thisGraphic;
		public:
			const size_t guid;
		public:
			std::vector<std::unique_ptr<JFrameResource>> frameResources;
			JFrameResource* currFrameResource = nullptr;
			int currFrameResourceIndex = 0;
		public:
			// Set true to use 4X MSAA (?.1.8).  The default is false.
			uint      m4xMsaaQuality = 0;      // quality level of 4X MSAA
			bool      m4xMsaaState = false;    // 4X MSAA enabled
		public:
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> directCmdListAlloc;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
		public:
			Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
			Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
			Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
			Microsoft::WRL::ComPtr<ID3D12Fence> fence;
			UINT64 currentFence = 0;
			D3D12_VIEWPORT screenViewport;
			D3D12_RECT scissorRect;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			int currBackBuffer = 0;
		public:
			bool stCommand = false;
		public:
			JGraphicInfo info;
			JGraphicOption option;
			UpdateHelper updateHelper;
			std::unique_ptr<JGraphicResourceManager> graphicResource;
			std::unique_ptr<JHardwareOccCulling> hdOccHelper;
			std::unique_ptr<JHZBOccCulling> hzbOccHelper;
			std::unique_ptr<JDepthMapDebug> depthMapDebug;
			std::unique_ptr<JOutline> outlineHelper;
			JOccBase* occBase = nullptr;
		public:
			bool* cullResult = nullptr;
		public:
			JGraphicImpl(const size_t guid, JGraphic* thisGraphic)
				:JEventListener(guid), guid(guid), thisGraphic(thisGraphic)
			{
				const uint occMipMapViewCapa = JGraphicResourceManager::GetOcclusionMipMapViewCapacity();
				info.occlusionWidth = std::pow(2, occMipMapViewCapa - 1);
				info.occlusionHeight = std::pow(2, occMipMapViewCapa - 1);
				info.occlusionMinSize = JGraphicResourceManager::GetOcclusionMinSize();
				info.occlusionMapCapacity = occMipMapViewCapa;
				info.occlusionMapCount = JMathHelper::Log2Int(info.occlusionWidth) - JMathHelper::Log2Int(JGraphicResourceManager::GetOcclusionMinSize()) + 1;

				updateHelper.uData.resize((int)J_UPLOAD_RESOURCE_TYPE::COUNT);
				updateHelper.bData.resize((int)J_GRAPHIC_RESOURCE_TYPE::COUNT);

				auto objGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetMeshCount();
					return sum;
				};
				auto passGetElemenLam = []() {return JGraphicDrawList::GetListCount(); };
				auto aniGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
					return sum;
				};
				auto camGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
					return sum;
				};
				auto litGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
					return sum;
				};
				auto litIndexGetElementLam = []() {return JGraphicDrawList::GetListCount(); };
				auto shadowLitGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->shadowRequestor.size();
					return sum;
				};
				auto shadowMapElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->shadowRequestor.size();
					return sum;
				};
				auto materialGetElementLam = []() {return _JResourceManager::Instance().GetResourceCount<JMaterial>(); };
				auto boundObjGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
					return sum;
				};

				using GetElementCount = UpdateHelper::GetElementCountT::Ptr;
				std::unordered_map<J_UPLOAD_RESOURCE_TYPE, GetElementCount> frameGetFunc
				{
					{J_UPLOAD_RESOURCE_TYPE::OBJECT, objGetElementLam}, {J_UPLOAD_RESOURCE_TYPE::PASS, passGetElemenLam},
					{J_UPLOAD_RESOURCE_TYPE::ANIMATION, aniGetElementLam},{J_UPLOAD_RESOURCE_TYPE::CAMERA, camGetElementLam},
					{J_UPLOAD_RESOURCE_TYPE::LIGHT, litGetElementLam},	{J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX, litIndexGetElementLam},
					{J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT, shadowLitGetElementLam},{J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP, shadowMapElementLam},
					{J_UPLOAD_RESOURCE_TYPE::MATERIAL, materialGetElementLam},{J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT, boundObjGetElementLam}
				};

				using NotifyUpdateCapacity = UpdateHelper::NotifyUpdateCapacityT::Callable;
				auto occUpdateObjCapaLam = [](JGraphic& graphic)
				{
					JGraphic::JGraphicImpl* impl = graphic.impl.get();
					impl->hzbOccHelper->UpdateObjectCapacity(impl->d3dDevice.Get(), impl->currFrameResource->GetElementCount(J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT));
				};
				auto occUpdatePassCapaLam = [](JGraphic& graphic)
				{
					JGraphic::JGraphicImpl* impl = graphic.impl.get();
					impl->hzbOccHelper->UpdatePassCapacity(impl->d3dDevice.Get(), impl->currFrameResource->GetElementCount(J_UPLOAD_RESOURCE_TYPE::CAMERA));
				};

				for (uint i = 0; i < (uint)J_UPLOAD_RESOURCE_TYPE::COUNT; ++i)
				{
					J_UPLOAD_RESOURCE_TYPE type = (J_UPLOAD_RESOURCE_TYPE)i;
					updateHelper.RegisterCallable(type, &frameGetFunc.find(type)->second);
				}

				auto updateCullingResultVecLam = [](JGraphic& graphic)
				{
					if (JApplicationEngine::GetApplicationState() != J_APPLICATION_STATE::EDIT_GAME)
						return;

					JGraphic::JGraphicImpl* impl = graphic.impl.get();
					const uint newCount = impl->currFrameResource->GetElementCount(J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT);
					impl->cullResult = (bool*)_recalloc(impl->cullResult, newCount, sizeof(bool) * newCount);
				};
				updateHelper.RegisterListener(J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT, std::make_unique<NotifyUpdateCapacity>(updateCullingResultVecLam));
				updateHelper.RegisterListener(J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT, std::make_unique<NotifyUpdateCapacity>(occUpdateObjCapaLam));
				updateHelper.RegisterListener(J_UPLOAD_RESOURCE_TYPE::CAMERA, std::make_unique<NotifyUpdateCapacity>(occUpdatePassCapaLam));

				auto texture2DGetCountLam = [](const JGraphic& graphic) {return graphic.impl->graphicResource->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D); };
				auto cubeMapGetCountLam = [](const JGraphic& graphic) {return graphic.impl->graphicResource->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE); };
				auto shadowMapGetCountLam = [](const JGraphic& graphic) {return graphic.impl->graphicResource->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP); };

				auto texture2DGetCapacityLam = [](const JGraphic& graphic) {return graphic.impl->info.binding2DTextureCapacity; };
				auto cubeMapGetCapacityLam = [](const JGraphic& graphic) {return graphic.impl->info.bindingCubeMapCapacity; };
				auto shadowMapGetCapacityLam = [](const JGraphic& graphic) {return graphic.impl->info.bindingShadowTextureCapacity; };

				auto texture2DSetCapaLam = [](JGraphic& graphic)
				{
					graphic.impl->SetInfoCapacity(graphic.impl->info.binding2DTextureCapacity,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].count,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].recompileCondition);
				};
				auto cubeMapeSetCapaLam = [](JGraphic& graphic)
				{
					graphic.impl->SetInfoCapacity(graphic.impl->info.bindingCubeMapCapacity,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].count,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].recompileCondition);
				};
				auto shadowMapSetCapaLam = [](JGraphic& graphic)
				{
					graphic.impl->SetInfoCapacity(graphic.impl->info.bindingShadowTextureCapacity,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].count,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].recompileCondition);
				};

				using BindTextureGetCount = UpdateHelper::GetTextureCountT::Ptr;
				using BindTextureGetCapacity = UpdateHelper::GetTextureCapacityT::Ptr;
				using BindTextureSetCapacity = UpdateHelper::SetCapacityT::Ptr;

				std::unordered_map <J_GRAPHIC_RESOURCE_TYPE, bool> hasCallable
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, true},
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, true},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, true}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureGetCount> bindTextureGetCountFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DGetCountLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapGetCountLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapGetCountLam}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureGetCapacity> bindTextureGetCapacityFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DGetCapacityLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapGetCapacityLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapGetCapacityLam}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureSetCapacity> bindTextureSetCapaFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DSetCapaLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapeSetCapaLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapSetCapaLam}
				};

				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					J_GRAPHIC_RESOURCE_TYPE type = (J_GRAPHIC_RESOURCE_TYPE)i;
					auto data = hasCallable.find(type);
					if (data != hasCallable.end())
					{
						updateHelper.RegisterCallable(type,
							&bindTextureGetCountFunc.find(type)->second,
							&bindTextureGetCapacityFunc.find(type)->second,
							&bindTextureSetCapaFunc.find(type)->second);
					}
				}
			}
			~JGraphicImpl() {}
		public:
			ID3D12Device* GetDevice()const noexcept
			{
				return d3dDevice.Get();
			}
			ID3D12CommandQueue* GetCommandQueue()const noexcept
			{
				return commandQueue.Get();
			}
			ID3D12CommandAllocator* GetCommandAllocator()const noexcept
			{
				return directCmdListAlloc.Get();
			}
			ID3D12GraphicsCommandList* GetCommandList()const noexcept
			{
				return commandList.Get();
			}
			JGraphicInfo GetGraphicInfo()const noexcept
			{
				return info;
			}
			JGraphicOption GetGraphicOption()const noexcept
			{
				return option;
			}
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return graphicResource->GetCpuRtvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return graphicResource->GetCpuDsvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return graphicResource->GetCpuSrvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return graphicResource->GetCpuSrvDescriptorHandle(index);
				default:
					return CD3DX12_CPU_DESCRIPTOR_HANDLE();
				}
			}
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return graphicResource->GetGpuRtvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return graphicResource->GetGpuDsvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return graphicResource->GetGpuSrvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return graphicResource->GetGpuSrvDescriptorHandle(index);
				default:
					return CD3DX12_GPU_DESCRIPTOR_HANDLE();
				}
			}
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex)
			{
				return GetCpuDescriptorHandle(bType, graphicResource->GetInfo(rType, rIndex)->GetHeapIndexStart(bType) + bIndex);
			}
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex)
			{
				return GetGpuDescriptorHandle(bType, graphicResource->GetInfo(rType, rIndex)->GetHeapIndexStart(bType) + bIndex);
			}
		public:
			void SetGraphicOption(JGraphicOption newGraphicOption)noexcept
			{
				if (newGraphicOption.isHDOcclusionAcitvated && newGraphicOption.isHZBOcclusionActivated)
				{
					if (!option.isHDOcclusionAcitvated)
						newGraphicOption.isHZBOcclusionActivated = false;
					else
						newGraphicOption.isHDOcclusionAcitvated = false;
				}
				option = newGraphicOption;
				if (option.isHDOcclusionAcitvated)
					occBase = hdOccHelper.get();
				else if (option.isHZBOcclusionActivated)
					occBase = hzbOccHelper.get();
				else
					occBase = nullptr;

				hdOccHelper->SetUpdateFrequency(option.occUpdateFrequency);
				hzbOccHelper->SetUpdateFrequency(option.occUpdateFrequency);
			}
			void SetImGuiBackEnd()
			{
				ID3D12DescriptorHeap* srvHeap = graphicResource->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV);
				ImGui_ImplDX12_Init(d3dDevice.Get(), Constants::gNumFrameResources,
					graphicResource->GetBackBufferFormat(),
					srvHeap,
					srvHeap->GetCPUDescriptorHandleForHeapStart(),
					srvHeap->GetGPUDescriptorHandleForHeapStart());
			}
			void SetInfoCapacity(uint& capacity, const uint count, const J_UPLOAD_CAPACITY_CONDITION cond)
			{
				capacity = CalculateCapacity(cond, capacity, count);
			}
		public:
			J_UPLOAD_CAPACITY_CONDITION IsPassRedefineCapacity(const uint capacity, const uint nowCount)const noexcept
			{
				if (capacity <= nowCount)
					return J_UPLOAD_CAPACITY_CONDITION::UP_CAPACITY;
				else if (nowCount < capacity / 2)
				{
					if (nowCount <= info.minCapacity)
						return J_UPLOAD_CAPACITY_CONDITION::KEEP;
					else
						return J_UPLOAD_CAPACITY_CONDITION::DOWN_CAPACITY;
				}
				else
					return J_UPLOAD_CAPACITY_CONDITION::KEEP;
			}
		public:
			void StartCommand()
			{
				if (!stCommand)
				{
					ThrowIfFailedHr(commandList->Reset(directCmdListAlloc.Get(), nullptr));
					stCommand = true;
				}
			}
			void EndCommand()
			{
				if (stCommand)
				{
					ThrowIfFailedG(commandList->Close());
					ID3D12CommandList* cmdsLists[] = { commandList.Get() };
					commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
					stCommand = false;
				}
			}
			void FlushCommandQueue()
			{
				// Advance the fence value to mark commands up to this fence point.
				currentFence++;
				// Add an instruction to the command queue to set a new fence point.  Because we 
				// are on the GPU timeline, the new fence point won't be set until the GPU finishes
				// libcessing all the commands prior to this Signal().
				ThrowIfFailedHr(commandQueue->Signal(fence.Get(), currentFence));

				// Wait until the GPU has completed commands up to this fence point.
				if (fence->GetCompletedValue() < currentFence)
				{
					HANDLE eventHandle = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
					// Fire event when GPU hits current fence.  
					ThrowIfFailedHr(fence->SetEventOnCompletion(currentFence, eventHandle));
					// Wait until the GPU hits current fence event is fired.
					WaitForSingleObject(eventHandle, INFINITE);
					CloseHandle(eventHandle);
				}
			}
		public:
			JUserPtr<JGraphicResourceInfo> Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> info = graphicResource->Create2DTexture(uploadBuffer, path, oriFormat, d3dDevice.Get(), commandList.Get());
					EndCommand();
					FlushCommandQueue();
					return info;
				}
				else
					return graphicResource->Create2DTexture(uploadBuffer, path, oriFormat, d3dDevice.Get(), commandList.Get());
			}
			JUserPtr<JGraphicResourceInfo> CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> info = graphicResource->CreateCubeMap(uploadBuffer, path, oriFormat, d3dDevice.Get(), commandList.Get());
					EndCommand();
					FlushCommandQueue();
					return info;
				}
				else
					return graphicResource->CreateCubeMap(uploadBuffer, path, oriFormat, d3dDevice.Get(), commandList.Get());
			}
			JUserPtr<JGraphicResourceInfo> CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0 || textureHeight == 0)
				{
					textureWidth = info.width;
					textureHeight = info.height;
				}

				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> info = graphicResource->CreateRenderTargetTexture(d3dDevice.Get(), textureWidth, textureHeight);
					EndCommand();
					FlushCommandQueue();
					return info;
				}
				else
					return graphicResource->CreateRenderTargetTexture(d3dDevice.Get(), textureWidth, textureHeight);
			}
			JUserPtr<JGraphicResourceInfo> CreateShadowMapTexture(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0 || textureHeight == 0)
				{
					textureWidth = info.defaultShadowWidth;
					textureHeight = info.defaultShadowHeight;
				}

				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> info = graphicResource->CreateShadowMapTexture(d3dDevice.Get(), textureWidth, textureHeight);
					EndCommand();
					FlushCommandQueue();
					return info;
				}
				else
					return graphicResource->CreateShadowMapTexture(d3dDevice.Get(), textureWidth, textureHeight);
			}
			bool DestroyGraphicTextureResource(JGraphicResourceInfo* info)
			{
				if (info == nullptr)
					return false;

				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					bool res = graphicResource->DestroyGraphicTextureResource(d3dDevice.Get(), info);
					EndCommand();
					FlushCommandQueue();
					return res;
				}
				else
				{
					bool res = graphicResource->DestroyGraphicTextureResource(d3dDevice.Get(), info);
					return res;
				}
			}
		public:
			void StuffGraphicShaderPso(JGraphicShaderData* shaderData,
				const J_SHADER_VERTEX_LAYOUT vertexLayout,
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
				const JShaderGraphicPsoCondition& psoCondition,
				const J_GRAPHIC_EXTRA_PSO_TYPE extraType)
			{
				FlushCommandQueue();
				StartCommand();

				D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
				newShaderPso.InputLayout = { shaderData->inputLayout.data(), (uint)shaderData->inputLayout.size() };
				newShaderPso.pRootSignature = mRootSignature.Get();
				newShaderPso.VS =
				{
					reinterpret_cast<BYTE*>(shaderData->vs->GetBufferPointer()),
					shaderData->vs->GetBufferSize()
				};
				if ((gFunctionFlag & SHADER_FUNCTION_WRITE_SHADOWMAP) == 0 && (gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) == 0)
				{
					newShaderPso.PS =
					{
						reinterpret_cast<BYTE*>(shaderData->ps->GetBufferPointer()),
						shaderData->ps->GetBufferSize()
					};
				}
				if (shaderData->hs != nullptr)
				{
					newShaderPso.HS =
					{
						reinterpret_cast<BYTE*>(shaderData->hs->GetBufferPointer()),
						shaderData->hs->GetBufferSize()
					};
				}
				if (shaderData->ds != nullptr)
				{
					newShaderPso.DS =
					{
						reinterpret_cast<BYTE*>(shaderData->ds->GetBufferPointer()),
						shaderData->ds->GetBufferSize()
					};
				}
				if (shaderData->gs != nullptr)
				{
					newShaderPso.GS =
					{
						reinterpret_cast<BYTE*>(shaderData->gs->GetBufferPointer()),
						shaderData->gs->GetBufferSize()
					};
				}

				newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				if (extraType == J_GRAPHIC_EXTRA_PSO_TYPE::STENCIL_WRITE_ALWAYS)
				{
					newShaderPso.DepthStencilState.StencilEnable = true;
					newShaderPso.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
				}

				if ((gFunctionFlag & SHADER_FUNCTION_SKY) > 0)
				{
					newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
					//newShaderPso.DepthStencilState.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; 
					newShaderPso.DepthStencilState.StencilEnable = false;
				}

				newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				newShaderPso.SampleMask = UINT_MAX;
				newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				newShaderPso.NumRenderTargets = 1;
				newShaderPso.RTVFormats[0] = graphicResource->GetBackBufferFormat();
				newShaderPso.SampleDesc.Count = m4xMsaaState ? 4 : 1;
				newShaderPso.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
				newShaderPso.DSVFormat = graphicResource->GetDepthStencilFormat();

				if ((gFunctionFlag & SHADER_FUNCTION_WRITE_SHADOWMAP) > 0)
				{
					newShaderPso.DepthStencilState.StencilEnable = false;
					newShaderPso.RasterizerState.DepthBias = 100000;
					newShaderPso.RasterizerState.DepthBiasClamp = 0.0f;
					newShaderPso.RasterizerState.SlopeScaledDepthBias = 1.0f;
					newShaderPso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
					newShaderPso.NumRenderTargets = 0;
					newShaderPso.SampleDesc.Count = 1;
					newShaderPso.SampleDesc.Quality = 0;
				}
				if ((gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) > 0)
				{
					newShaderPso.DepthStencilState.StencilEnable = false;
					newShaderPso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
					newShaderPso.NumRenderTargets = 0;
					newShaderPso.SampleDesc.Count = 1;
					newShaderPso.SampleDesc.Quality = 0;
				}
				if ((gFunctionFlag & SHADER_FUNCTION_DEBUG) > 0)
				{
					//newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
					newShaderPso.DepthStencilState.StencilEnable = false;
				}

				if (psoCondition.primitiveCondition == J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO)
					newShaderPso.PrimitiveTopologyType = psoCondition.ConvertD3d12PrimitiveType();
				if (psoCondition.depthCompareCondition == J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO)
					newShaderPso.DepthStencilState.DepthFunc = psoCondition.ConvertD3d12Comparesion();
				if (psoCondition.cullModeCondition == J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO)
					newShaderPso.RasterizerState.CullMode = psoCondition.ConvertD3d12CullMode();

				if (extraType == J_GRAPHIC_EXTRA_PSO_TYPE::NONE)
				{
					ThrowIfFailedG(d3dDevice->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->pso.GetAddressOf())));
					shaderData->pso->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Pso") - 1, "Pso");
				}
				else if (extraType == J_GRAPHIC_EXTRA_PSO_TYPE::STENCIL_WRITE_ALWAYS)
				{
					ThrowIfFailedG(d3dDevice->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->extraPso[(int)extraType].GetAddressOf())));
					shaderData->extraPso[(int)extraType]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Extra Pso") - 1, "Extra Pso");
				}

				EndCommand();
				FlushCommandQueue();
			}
			void StuffComputeShaderPso(JComputeShaderData* shaderData, J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
			{
				FlushCommandQueue();
				StartCommand();

				D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

				if (cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_COPY ||
					cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING ||
					cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION)
					newShaderPso.pRootSignature = hzbOccHelper->GetRootSignature();

				shaderData->RootSignature = newShaderPso.pRootSignature;
				newShaderPso.CS =
				{
					reinterpret_cast<BYTE*>(shaderData->cs->GetBufferPointer()),
					shaderData->cs->GetBufferSize()
				};
				newShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
				ThrowIfFailedG(d3dDevice->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->pso.GetAddressOf())));

				EndCommand();
				FlushCommandQueue();
			}
		public:
			void ResourceTransition(_In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
			{
				CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, stateBefore, stateAfter);
				commandList->ResourceBarrier(1, &rsBarrier);
			}
		public:
			void ReBuildFrameResource(const J_UPLOAD_RESOURCE_TYPE type, const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowObjCount)
			{
				for (int i = 0; i < Constants::gNumFrameResources; ++i)
				{
					const uint newCapacity = CalculateCapacity(condition, frameResources[i]->GetElementCount(type), nowObjCount);
					frameResources[i]->BuildFrameResource(d3dDevice.Get(), type, newCapacity);
				}
			}
			void ReCompileGraphicShader()
			{
				mRootSignature.Reset();
				BuildRootSignature();

				auto shaderVec = JShader::StaticTypeInfo().GetInstanceRawPtrVec();
				for (auto& data : shaderVec)
				{
					JShader* shader = static_cast<JShader*>(data);
					if (!shader->IsComputeShader())
						JShaderPrivate::CompileInterface::RecompileGraphicShader(shader);
				}
			}
		public:
			uint CalculateCapacity(const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowCapacity, const uint nowCount)const noexcept
			{
				uint nextCapacity = nowCapacity;
				if (condition == J_UPLOAD_CAPACITY_CONDITION::UP_CAPACITY)
				{
					while (nextCapacity <= nowCount)
						nextCapacity *= 2;
				}
				else if (condition == J_UPLOAD_CAPACITY_CONDITION::DOWN_CAPACITY)
				{
					while ((nextCapacity / 2) > nowCount && (nextCapacity / 2) > info.minCapacity)
						nextCapacity /= 2;
				}
				return nextCapacity;
			}
			void OnResize()
			{
				info.width = JWindow::GetClientWidth();
				info.height = JWindow::GetClientHeight();

				assert(d3dDevice);
				assert(swapChain);
				assert(directCmdListAlloc);

				// Flush before changing any resources.
				FlushCommandQueue();
				StartCommand();

				currBackBuffer = 0;
				graphicResource->CreateSwapChainBuffer(d3dDevice.Get(), swapChain.Get(), info.width, info.height);
				graphicResource->CreateMainDepthStencilResource(d3dDevice.Get(), commandList.Get(), info.width, info.height, m4xMsaaState, m4xMsaaQuality);
				graphicResource->CreateDebugDepthStencilResource(d3dDevice.Get(), commandList.Get(), info.width, info.height);
				outlineHelper->UpdatePassBuf(info.width, info.height, Constants::commonStencilRef);

				// Wait until resize is complete.
				EndCommand();
				FlushCommandQueue();

				// Update the viewport transform to cover the client area.
				screenViewport.TopLeftX = 0;
				screenViewport.TopLeftY = 0;
				screenViewport.Width = static_cast<float>(info.width);
				screenViewport.Height = static_cast<float>(info.height);
				screenViewport.MinDepth = 0.0f;
				screenViewport.MaxDepth = 1.0f;

				scissorRect = { 0, 0,info.width, info.height };
			}
		public:
			ID3D12Resource* CurrentBackBuffer()const
			{
				return graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, currBackBuffer);
			}
			D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const
			{
				return graphicResource->GetCpuRtvDescriptorHandle(graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, currBackBuffer)->
					GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
			}
			const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()const noexcept
			{
				return std::vector<CD3DX12_STATIC_SAMPLER_DESC>
				{
					//PointWrap
					CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

						//pointClamp
						CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

							//linearWrap
						CD3DX12_STATIC_SAMPLER_DESC(2, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

							//linearClamp
						CD3DX12_STATIC_SAMPLER_DESC(3, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

							//anisotropicWrap
						CD3DX12_STATIC_SAMPLER_DESC(4, // shaderRegister
							D3D12_FILTER_ANISOTROPIC, // filter
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
							0.0f,                             // mipLODBias
							8),				                  // maxAnisotropy

							//anisotropicClamp
						CD3DX12_STATIC_SAMPLER_DESC(5, // shaderRegister
							D3D12_FILTER_ANISOTROPIC, // filter
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
							0.0f,                              // mipLODBias
							8),                                // maxAnisotropy

							//shadow
						CD3DX12_STATIC_SAMPLER_DESC(6, // shaderRegister
							D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
							D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
							0.0f,                               // mipLODBias
							16,                                 // maxAnisotropy
							D3D12_COMPARISON_FUNC_LESS_EQUAL,
							D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
				};
			}
		public:
			void StartFrame()
			{
				auto pMem = Core::JMemoryCapture::GetCurrentProcessMemory();
				//MessageBox(0, Core::JByteUnit::ByteToWString(pMem.privateUsage).c_str(), L"Usage1", 0);
				ImGui_ImplDX12_NewFrame();
				pMem = Core::JMemoryCapture::GetCurrentProcessMemory();
				//MessageBox(0, Core::JByteUnit::ByteToWString(pMem.privateUsage).c_str(), L"Usage2", 0);
				ImGui_ImplWin32_NewFrame();
				pMem = Core::JMemoryCapture::GetCurrentProcessMemory();
				//MessageBox(0, Core::JByteUnit::ByteToWString(pMem.privateUsage).c_str(), L"Usage3", 0);
				ImGui::NewFrame();
				pMem = Core::JMemoryCapture::GetCurrentProcessMemory();
				//MessageBox(0, Core::JByteUnit::ByteToWString(pMem.privateUsage).c_str(), L"Usage4", 0);
			}
			void EndFrame()
			{
				ImGui::Render();

				ID3D12Resource* mainDepthResource = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL, 0);
				JGraphicResourceInfo* mainDepthInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL, 0);;

				ResourceTransition(mainDepthResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				ResourceTransition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
				D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(mainDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV));

				commandList->ClearRenderTargetView(rtv, graphicResource->GetBackBufferClearColor(), 0, nullptr);
				commandList->OMSetRenderTargets(1, &rtv, true, nullptr);

				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

				ResourceTransition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
				ResourceTransition(mainDepthResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
				ThrowIfFailedG(commandList->Close());
				ID3D12CommandList* cmdsLists[] = { commandList.Get() };
				commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

				//ImGuiIO& io = ImGui::GetIO();
				//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				//{
				//	ImGui::UpdatePlatformWindows();
				//	ImGui::RenderPlatformWindowsDefault(NULL, (void*)cmdsLists);
				//}

				// Swap the back and front buffers
				ThrowIfFailedG(swapChain->Present(0, 0));
				currBackBuffer = (currBackBuffer + 1) % graphicResource->GetResourceCapacity(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN);
				// Advance the fence value to mark commands up to this fence point.
				currFrameResource->fence = ++currentFence;

				// Add an instruction to the command queue to set a new fence point. 
				// Because we are on the GPU timeline, the new fence point won't be 
				// set untwil the GPU finishes processing all the commands prior to this Signal().
				commandQueue->Signal(fence.Get(), currentFence);
			}
			void UpdateWait()
			{
				currFrameResourceIndex = (currFrameResourceIndex + 1) % Constants::gNumFrameResources;
				currFrameResource = frameResources[currFrameResourceIndex].get();
				if (currFrameResource->fence != 0 && fence->GetCompletedValue() < currFrameResource->fence)
				{
					//nullptr, FALSE, FALSE, EVENT_ALL_ACCESS
					HANDLE eventHandle = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
					ThrowIfFailedHr(fence->SetEventOnCompletion(currFrameResource->fence, eventHandle));
					WaitForSingleObject(eventHandle, INFINITE);
					CloseHandle(eventHandle);
				}
			}
			void UpdateEngine()
			{
				if (option.IsHZBOccActivated())
					hzbOccHelper->ReadCullingResult();

				updateHelper.Clear();
				for (uint i = 0; i < (uint)J_UPLOAD_RESOURCE_TYPE::COUNT; ++i)
				{
					updateHelper.uData[i].count = (*updateHelper.uData[i].getElementCountCallable)(nullptr);
					updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_RESOURCE_TYPE)i);
					updateHelper.uData[i].rebuildCondition = IsPassRedefineCapacity(updateHelper.uData[i].capacity, updateHelper.uData[i].count);
					updateHelper.hasRebuildCondition |= (bool)updateHelper.uData[i].rebuildCondition;
				}
				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					if (updateHelper.bData[i].HasCallable())
					{
						updateHelper.bData[i].count = (*updateHelper.bData[i].getTextureCountCallable)(nullptr, *thisGraphic);
						updateHelper.bData[i].capacity = (*updateHelper.bData[i].getTextureCapacityCallable)(nullptr, *thisGraphic);
						updateHelper.bData[i].recompileCondition = IsPassRedefineCapacity(updateHelper.bData[i].capacity, updateHelper.bData[i].count);
						updateHelper.hasRecompileShader |= (bool)updateHelper.bData[i].recompileCondition;

						if (updateHelper.bData[i].recompileCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
						{
							updateHelper.bData[i].capacity = CalculateCapacity(updateHelper.bData[i].recompileCondition,
								updateHelper.bData[i].capacity,
								updateHelper.bData[i].count);
						}
					}
				}

				updateHelper.WriteGraphicInfo(info);
				if (updateHelper.hasRebuildCondition)
				{
					FlushCommandQueue();
					StartCommand();
					for (uint i = 0; i < (uint)J_UPLOAD_RESOURCE_TYPE::COUNT; ++i)
					{
						if (updateHelper.uData[i].rebuildCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
						{
							ReBuildFrameResource((J_UPLOAD_RESOURCE_TYPE)i, updateHelper.uData[i].rebuildCondition, updateHelper.uData[i].count);
							updateHelper.uData[i].setFrameDirty = true;
							updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_RESOURCE_TYPE)i);
						}
					}
					//Has sequency dependency
					updateHelper.WriteGraphicInfo(info);
					updateHelper.NotifyUpdateFrameCapacity(*thisGraphic);	//use graphic info
					if (updateHelper.hasRecompileShader)
						ReCompileGraphicShader();		//use graphic info
					EndCommand();
					FlushCommandQueue();
				}
				else if (updateHelper.hasRecompileShader)
				{
					FlushCommandQueue();
					StartCommand();
					ReCompileGraphicShader();
					EndCommand();
					FlushCommandQueue();
				}

				const uint drawListCount = JGraphicDrawList::GetListCount();
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					const bool isAllowOcclusion = drawTarget->scene->IsMainScene() && option.isOcclusionQueryActivated;
					const uint sceneDrawReqCount = (uint)drawTarget->sceneRequestor.size();
					drawTarget->updateInfo->UpdateStart();
					UpdateSceneObjectCB(drawTarget->scene, drawTarget->updateInfo->objUpdateCount, drawTarget->updateInfo->hotObjUpdateCount);
					UpdateScenePassCB(drawTarget->scene);	//always update
					if (isAllowOcclusion && option.IsHZBOccActivated())
					{
						const uint queryCount = drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
						for (uint j = 0; j < sceneDrawReqCount; ++j)
						{
							//Has order dependency 
							//before camUpdate 
							JCamera* cam = static_cast<JCamera*>(drawTarget->sceneRequestor[j]->jCamera.Get());
							hzbOccHelper->UpdatePass(cam, info, option, queryCount, updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].offset + j);
						}
					}
					UpdateSceneAnimationCB(drawTarget->scene, drawTarget->updateInfo->aniUpdateCount, drawTarget->updateInfo->hotAniUpdateCount);
					UpdateSceneCameraCB(drawTarget->scene, drawTarget->updateInfo->camUpdateCount, drawTarget->updateInfo->hotCamUpdateCount);
					UpdateSceneLightCB(drawTarget->scene, drawTarget->updateInfo->lightUpdateCount, drawTarget->updateInfo->hotLitghtUpdateCount);	//always update
					drawTarget->updateInfo->UpdateEnd();
				}
				UpdateMaterialCB();
				if (occBase != nullptr)
					occBase->UpdateTimer();
			}
			void UpdateSceneObjectCB(_In_ const JUserPtr<JScene>& scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
			{
				const bool isUpdateBoundingObj = scene->IsMainScene() && option.isOcclusionQueryActivated;
				uint addOffset = 0;
				const std::vector<JUserPtr<JComponent>>& jRvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint renderItemCount = (uint)jRvec.size();

				JObjectConstants objectConstants;
				auto currObjectCB = currFrameResource->objectCB.get();

				JBoundingObjectConstants boundingConstants;
				auto currBoundingObjectCB = currFrameResource->bundingObjectCB.get();

				const uint meshOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].offset;
				const uint rItemOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT].offset;
				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].setFrameDirty
					|| updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT].setFrameDirty;

				using FrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
				for (uint i = 0; i < renderItemCount; ++i)
				{
					JRenderItem* renderItem = static_cast<JRenderItem*>(jRvec[i].Get());
					const uint submeshCount = renderItem->GetSubmeshCount();

					if (FrameUpdateInterface::UpdateStart(renderItem, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(renderItem, boundingConstants);
						currBoundingObjectCB->CopyData(rItemOffset + FrameUpdateInterface::GetBoundingCBOffset(renderItem), boundingConstants);
						hzbOccHelper->UpdateObject(renderItem, rItemOffset + i);

						for (uint j = 0; j < submeshCount; ++j)
						{
							FrameUpdateInterface::UpdateFrame(renderItem, objectConstants, j);
							currObjectCB->CopyData(meshOffset + FrameUpdateInterface::GetObjectCBBuffOffset(renderItem) + j, objectConstants);
							++updateCount;
						}
						if (FrameUpdateInterface::IsHotUpdated(renderItem))
							++hotUpdateCount;
						FrameUpdateInterface::UpdateEnd(renderItem);
					}
					addOffset += submeshCount;
				}

				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT].offset += renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].offset += addOffset;
			}
			void UpdateMaterialCB()
			{
				auto currMaterialBuffer = currFrameResource->materialBuffer.get();
				auto matVec = JMaterial::StaticTypeInfo().GetInstanceRawPtrVec();
				const uint matCount = matVec.size();
				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::MATERIAL].setFrameDirty;

				using FrameUpdateInterface = JMaterialPrivate::FrameUpdateInterface;
				for (uint i = 0; i < matCount; ++i)
				{
					JMaterialConstants materialConstant;
					JMaterial* material = static_cast<JMaterial*>(matVec[i]);

					if (FrameUpdateInterface::UpdateStart(material, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(material, materialConstant);
						currMaterialBuffer->CopyData(FrameUpdateInterface::GetCBOffset(material), materialConstant);
						FrameUpdateInterface::UpdateEnd(material);
					}
				};
			}
			void UpdateScenePassCB(_In_ const JUserPtr<JScene>& scene)
			{
				JPassConstants passContants;
				passContants.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
				passContants.totalTime = JEngineTimer::Data().TotalTime();
				passContants.deltaTime = JEngineTimer::Data().DeltaTime();

				auto currPassCB = currFrameResource->passCB.get();
				currPassCB->CopyData(updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::PASS].offset, passContants);
				++updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::PASS].offset;
			}
			void UpdateSceneAnimationCB(_In_ const JUserPtr<JScene>& scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
			{
				const std::vector<JUserPtr<JComponent>>& jAvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
				const uint animatorCount = (uint)jAvec.size();

				auto currSkinnedCB = currFrameResource->skinnedCB.get();
				JAnimationConstants animationConstatns;

				const uint offset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].offset;
				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].setFrameDirty;
				using FrameUpdateInterface = JAnimatorPrivate::FrameUpdateInterface;

				if (scene->IsActivatedSceneTime())
				{
					for (uint i = 0; i < animatorCount; ++i)
					{
						JAnimator* animator = static_cast<JAnimator*>(jAvec[i].Get());
						if (FrameUpdateInterface::UpdateStart(animator))
						{
							FrameUpdateInterface::UpdateFrame(animator, animationConstatns);
							currSkinnedCB->CopyData(offset + i, animationConstatns);
							FrameUpdateInterface::UpdateEnd(animator);
							++updateCount;
						}
					}
				}
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].offset += animatorCount;
			}
			void UpdateSceneCameraCB(_In_ const JUserPtr<JScene>& scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
			{
				const std::vector<JUserPtr<JComponent>>& jCvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
				const uint cameraCount = (uint)jCvec.size();

				auto currCameraCB = currFrameResource->cameraCB.get();
				JCameraConstants camContants;

				const uint offset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].offset;
				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].setFrameDirty;
				using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;

				for (uint i = 0; i < cameraCount; ++i)
				{
					JCamera* camera = static_cast<JCamera*>(jCvec[i].Get());
					if (FrameUpdateInterface::UpdateStart(camera, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(camera, camContants);
						currCameraCB->CopyData(offset + i, camContants);
						if (FrameUpdateInterface::IsHotUpdated(camera))
							++hotUpdateCount;
						FrameUpdateInterface::UpdateEnd(camera);
						++updateCount;
					}
				}
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].offset += cameraCount;
			}
			void UpdateSceneLightCB(_In_ const JUserPtr<JScene>& scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
			{
				const std::vector<JUserPtr<JComponent>>& jLvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
				const uint lightVecCount = (uint)jLvec.size();

				auto currLightCB = currFrameResource->lightBuffer.get();
				auto currLightIndexCB = currFrameResource->lightIndexCB.get();
				auto currSMLightCB = currFrameResource->smLightBuffer.get();
				auto currShadowCalCB = currFrameResource->shadowCalCB.get();

				JLightConstants lightConstants;
				JLightIndexConstants lightIndexConstants;
				JShadowMapLightConstants smLightConstants;
				JShadowMapConstants shadowCalConstant;

				uint lightOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset;
				uint lightIndexOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX].offset;
				uint shadowLitOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset;
				uint shadowMapOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP].offset;

				uint litCount = 0;
				uint smLitCount = 0;

				bool hasLitUpdate = false;
				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].setFrameDirty |
					updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX].setFrameDirty |
					updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].setFrameDirty |
					updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP].setFrameDirty;

				using FrameUpdateInterface = JLightPrivate::FrameUpdateInterface;
				for (uint i = 0; i < lightVecCount; ++i)
				{
					JLight* light = static_cast<JLight*>(jLvec[i].Get());
					const bool onShadow = light->IsShadowActivated();

					if (FrameUpdateInterface::UpdateStart(light, forcedSetFrameDirty))
					{
						if (onShadow)
						{
							FrameUpdateInterface::UpdateFrame(light, smLightConstants);
							FrameUpdateInterface::UpdateFrame(light, shadowCalConstant);
							currShadowCalCB->CopyData(shadowMapOffset + smLitCount, shadowCalConstant);
							currSMLightCB->CopyData(shadowMapOffset + smLitCount, smLightConstants);
						}
						else
						{
							FrameUpdateInterface::UpdateFrame(light, lightConstants);
							currLightCB->CopyData(lightOffset + litCount, lightConstants);
						}
						if (FrameUpdateInterface::IsHotUpdated(light))
							++hotUpdateCount;
						++updateCount;
						FrameUpdateInterface::UpdateEnd(light);

					}
					if (onShadow)
						++smLitCount;
					else
						++litCount;
				}

				lightIndexConstants.litStIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset;
				lightIndexConstants.litEdIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset + litCount;
				lightIndexConstants.shadwMapStIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset;
				lightIndexConstants.shadowMapEdIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset + smLitCount;
				currLightIndexCB->CopyData(lightIndexOffset, lightIndexConstants);

				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset += litCount;
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX].offset += 1;
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset += smLitCount;
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP].offset += smLitCount;
			}
			void DrawScene()
			{
				auto setGraphicResource = [](JGraphicImpl* g)
				{
					g->commandList->SetGraphicsRootSignature(g->mRootSignature.Get());

					g->commandList->SetGraphicsRootShaderResourceView(7, g->currFrameResource->lightBuffer->Resource()->GetGPUVirtualAddress());
					g->commandList->SetGraphicsRootShaderResourceView(8, g->currFrameResource->smLightBuffer->Resource()->GetGPUVirtualAddress());
					g->commandList->SetGraphicsRootShaderResourceView(9, g->currFrameResource->materialBuffer->Resource()->GetGPUVirtualAddress());

					g->commandList->SetGraphicsRootDescriptorTable(10, g->graphicResource->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE));
					g->commandList->SetGraphicsRootDescriptorTable(11, g->graphicResource->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D));
					g->commandList->SetGraphicsRootDescriptorTable(12, g->graphicResource->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP));
				};

				auto cmdListAlloc = currFrameResource->cmdListAlloc;
				ThrowIfFailedHr(cmdListAlloc->Reset());
				ThrowIfFailedHr(commandList->Reset(cmdListAlloc.Get(), nullptr));

				//Test Code
				commandList->OMSetStencilRef(Constants::commonStencilRef);
				ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResource->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
				commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
				commandList->RSSetViewports(1, &screenViewport);
				commandList->RSSetScissorRects(1, &scissorRect);

				//수정필요
				//Shadow Map Draw
				//Update가 있는 광원만 추적해서 Draw
				//광원내에 있는 오브젝트 검색을 위한 공간분할 및 검색 필요

				bool hasToSetGResource = true;
				const uint drawListCount = JGraphicDrawList::GetListCount();
				std::vector<DrawHelper> occlusionCash;
				DrawHelper helper;
				for (uint i = 0; i < drawListCount; ++i)
				{
					if (hasToSetGResource)
					{
						setGraphicResource(this);
						hasToSetGResource = false;
					}
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					if (drawTarget->updateInfo->hasShadowUpdate)
					{
						const uint shadowReqCount = (uint)drawTarget->shadowRequestor.size();
						for (uint j = 0; j < shadowReqCount; ++j)
						{
							DrawHelper copiedHelper = helper;
							copiedHelper.scene = drawTarget->scene;
							copiedHelper.lit.ConnnectChild(drawTarget->shadowRequestor[j]->jLight);
							copiedHelper.shadowOffset += j;
							//copiedHelper.isDrawShadowMap = true;
							DrawSceneShadowMap(copiedHelper);
						}
					}
					if (drawTarget->updateInfo->hasSceneUpdate)
					{
						const uint sceneReqCount = (uint)drawTarget->sceneRequestor.size();
						for (uint j = 0; j < sceneReqCount; ++j)
						{
							if (hasToSetGResource)
							{
								setGraphicResource(this);
								hasToSetGResource = false;
							}
							const J_SCENE_USE_CASE_TYPE useCaseType = drawTarget->scene->GetUseCaseType();
							DrawHelper copiedHelper = helper;
							copiedHelper.scene = drawTarget->scene;
							copiedHelper.cam.ConnnectChild(drawTarget->sceneRequestor[j]->jCamera);
							copiedHelper.camOffset += j;

							copiedHelper.allowDrawDebug = copiedHelper.cam->AllowDisplayDebug();
							copiedHelper.allowCulling = copiedHelper.cam->AllowCulling();
							 
							if (copiedHelper.cam->AllowCulling())
								JScenePrivate::CullingInterface::ViewCulling(drawTarget->scene, copiedHelper.cam);

							DrawSceneRenderTarget(copiedHelper);
							const bool isOcclusionActivated = option.IsHDOccActivated() || option.IsHZBOccActivated();
							const bool canCullingStart = (occBase != nullptr) && occBase->CanCullingStart();

							if (copiedHelper.allowCulling && isOcclusionActivated && canCullingStart)
								occlusionCash.push_back(copiedHelper);

							if (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN && option.allowDebugOutline && copiedHelper.allowDrawDebug)
							{
								JGraphicResourceInfo* editorDepthInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL, 0);
								outlineHelper->DrawOutline(commandList.Get(),
									graphicResource->GetGpuSrvDescriptorHandle(editorDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
									graphicResource->GetGpuSrvDescriptorHandle(editorDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + 1));
								hasToSetGResource = true;
							}
							if (useCaseType == J_SCENE_USE_CASE_TYPE::MAIN && option.allowDebugOutline && copiedHelper.allowDrawDebug)
							{
								JGraphicResourceInfo* mainDepthInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL, 0);
								JGraphicResourceInfo* mainDepthDebugInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG, 0);

								depthMapDebug->DrawNonLinearDepthDebug(commandList.Get(),
									graphicResource->GetGpuSrvDescriptorHandle(mainDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
									graphicResource->GetGpuSrvDescriptorHandle(mainDepthDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
									JVector2<uint>(info.width, info.height),
									copiedHelper.cam->GetNear(),
									copiedHelper.cam->GetFar());
								hasToSetGResource = true;
							}
						}
					}
					helper.objectMeshOffset += drawTarget->scene->GetMeshCount();
					helper.objectRitemOffset += drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
					helper.passOffset += 1;
					helper.aniOffset += drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
					helper.camOffset += drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
					helper.litIndexOffset += 1;
					helper.shadowOffset += (uint)drawTarget->shadowRequestor.size();
				}
				const uint occlusionCount = (uint)occlusionCash.size();
				if (occlusionCount > 0)
					setGraphicResource(this);
				for (uint i = 0; i < occlusionCount; ++i)
					DrawOcclusionDepthMap(occlusionCash[i]);
			}
			void DrawProjectSelector()
			{
				auto cmdListAlloc = currFrameResource->cmdListAlloc;
				ThrowIfFailedHr(cmdListAlloc->Reset());
				ThrowIfFailedHr(commandList->Reset(cmdListAlloc.Get(), nullptr));

				commandList->SetGraphicsRootSignature(mRootSignature.Get());

				ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResource->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
				commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
				commandList->RSSetViewports(1, &screenViewport);
				commandList->RSSetScissorRects(1, &scissorRect);
			}
			void DrawSceneRenderTarget(const DrawHelper helper)
			{
				auto gRInterface = helper.cam->GraphicResourceUserInterface();
				const uint rtvVecIndex = gRInterface.GetResourceArrayIndex();
				const uint rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV);

				commandList->RSSetViewports(1, &screenViewport);
				commandList->RSSetScissorRects(1, &scissorRect);

				ID3D12Resource* dsResource = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL, 0);
				JGraphicResourceInfo* dsInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL, 0);

				ID3D12Resource* rtResource = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);
				JGraphicResourceInfo* rtInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);

				ResourceTransition(dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				ResourceTransition(rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

				CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResource->GetCpuRtvDescriptorHandle(rtvHeapIndex);
				CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(dsInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV));

				commandList->ClearRenderTargetView(rtv, graphicResource->GetBackBufferClearColor(), 0, nullptr);
				commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
				commandList->OMSetRenderTargets(1, &rtv, true, &dsv);

				currFrameResource->passCB->SetGraphicCBBufferView(commandList.Get(), 2, helper.passOffset);
				currFrameResource->cameraCB->SetGraphicCBBufferView(commandList.Get(), 3, helper.camOffset);
				currFrameResource->lightIndexCB->SetGraphicCBBufferView(commandList.Get(), 4, helper.litIndexOffset);

				using GCash = JScenePrivate::CashInterface;
				const std::vector<JUserPtr<JGameObject>>& objVec00 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec01 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED);
				const std::vector< JUserPtr<JGameObject>>& objVec02 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::DEBUG_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec03 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::SKY, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec04 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::DEBUG_UI, J_MESHGEOMETRY_TYPE::STATIC);

				DrawGameObject(commandList.Get(), objVec00, helper, DrawCondition(option, helper, false, true, helper.allowDrawDebug));
				DrawGameObject(commandList.Get(), objVec01, helper, DrawCondition(option, helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebug));
				if (option.IsHDOccActivated())
					commandList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
				if (helper.allowDrawDebug)
					DrawGameObject(commandList.Get(), objVec02, helper, DrawCondition());
				DrawGameObject(commandList.Get(), objVec03, helper, DrawCondition());
				if (helper.allowDrawDebug)
				{
					ID3D12Resource* debugResource = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, 0);
					JGraphicResourceInfo* debugDepthInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, 0);

					CD3DX12_CPU_DESCRIPTOR_HANDLE editorDsv = graphicResource->GetCpuDsvDescriptorHandle(debugDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV));
					ResourceTransition(debugResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					commandList->ClearDepthStencilView(editorDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
					commandList->OMSetRenderTargets(1, &rtv, true, &editorDsv);
					DrawGameObject(commandList.Get(), objVec04, helper, DrawCondition());
					ResourceTransition(debugResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
				}

				ResourceTransition(rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
				ResourceTransition(dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
			}
			void DrawSceneShadowMap(const DrawHelper helper)
			{
				auto gRInterface = helper.lit->GraphicResourceUserInterface();
				const uint shadowWidth = gRInterface.GetResourceWidth();
				const uint shadowHeight = gRInterface.GetResourceHeight();
				const uint rVecIndex = gRInterface.GetResourceArrayIndex();
				const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV);

				D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)shadowWidth, (float)shadowHeight, 0.0f, 1.0f };
				D3D12_RECT mScissorRect = { 0, 0, shadowWidth, shadowHeight };

				commandList->RSSetViewports(1, &mViewport);
				commandList->RSSetScissorRects(1, &mScissorRect);

				ID3D12Resource* shdowMapResource = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, rVecIndex);
				ResourceTransition(shdowMapResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

				D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(dsvHeapIndex);
				commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
				commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

				currFrameResource->passCB->SetGraphicCBBufferView(commandList.Get(), 2, helper.passOffset);
				currFrameResource->shadowCalCB->SetGraphicCBBufferView(commandList.Get(), 5, helper.shadowOffset);

				using GCash = JScenePrivate::CashInterface;
				const std::vector<JUserPtr<JGameObject>>& objVec00 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec01 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED);

				DrawShadowMapGameObject(commandList.Get(), objVec00, helper, DrawCondition(option, helper, false, false, false));
				DrawShadowMapGameObject(commandList.Get(), objVec01, helper, DrawCondition(option, helper, helper.scene->IsActivatedSceneTime(), false, false));

				ResourceTransition(shdowMapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
			}
			void DrawOcclusionDepthMap(const DrawHelper helper)
			{
				//commandList->SetGraphicsRootSignature(mRootSignature.Get());

				D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)info.occlusionWidth, (float)info.occlusionHeight, 0.0f, 1.0f };
				D3D12_RECT mScissorRect = { 0, 0, info.occlusionWidth, info.occlusionHeight };

				commandList->RSSetViewports(1, &mViewport);
				commandList->RSSetScissorRects(1, &mScissorRect);

				ID3D12Resource* occDepthMap = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, 0);
				ResourceTransition(occDepthMap, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

				JGraphicResourceInfo* occDsInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, 0);
				D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(occDsInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV));
				commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
				commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

				currFrameResource->cameraCB->SetGraphicCBBufferView(commandList.Get(), 3, helper.camOffset);
				DrawSceneBoundingBox(commandList.Get(),
					helper.scene->GetAlignedObject(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, helper.cam->GetBoundingFrustum()),
					helper, DrawCondition(option, helper, false, true, false));

				ResourceTransition(occDepthMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);

				if (option.IsHZBOccActivated())
				{
					JGraphicResourceInfo* occMipMapInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, 0);

					hzbOccHelper->DepthMapDownSampling(commandList.Get(),
						graphicResource->GetGpuSrvDescriptorHandle(occDsInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						graphicResource->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						graphicResource->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
						info.occlusionMapCount,
						graphicResource->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::SRV),
						helper.camOffset);
					hzbOccHelper->OcclusuinCulling(commandList.Get(),
						graphicResource->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						helper.camOffset);

					if (option.allowHZBDepthMapDebug)
					{
						JVector2<uint> occlusionSize = JVector2<uint>(info.occlusionWidth, info.occlusionHeight);
						const float camNear = helper.cam->GetNear();
						const float camFar = helper.cam->GetFar();

						//Debug and mipmap viwe count is same
						JGraphicResourceInfo* occDebugInfo = graphicResource->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP, 0);
						const uint viewCount = occDebugInfo->GetViewCount(J_GRAPHIC_BIND_TYPE::SRV);
						for (uint i = 0; i < viewCount; ++i)
						{
							depthMapDebug->DrawLinearDepthDebug(commandList.Get(),
								graphicResource->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + i),
								graphicResource->GetGpuSrvDescriptorHandle(occDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV) + i),
								occlusionSize,
								camNear,
								camFar);
							occlusionSize /= 2;
						}
					}
				}
				else if (option.IsHDOccActivated())
				{
					CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->GetOcclusionQueryResult(), D3D12_RESOURCE_STATE_PREDICATION, D3D12_RESOURCE_STATE_COPY_DEST);
					commandList->ResourceBarrier(1, &rsBarrier);
					commandList->ResolveQueryData(graphicResource->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, 0, graphicResource->GetOcclusionQueryCapacity(), graphicResource->GetOcclusionQueryResult(), 0);
					rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->GetOcclusionQueryResult(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PREDICATION);
					commandList->ResourceBarrier(1, &rsBarrier);
				}
			}
			void DrawGameObject(ID3D12GraphicsCommandList* commandList,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const DrawHelper helper,
				const DrawCondition& condition)
			{
				uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
				uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

				auto objectCB = currFrameResource->objectCB->Resource();
				auto skinCB = currFrameResource->skinnedCB->Resource();
				const uint gameObjCount = (uint)gameObject.size();

				using RFrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
				using BufferViewInterface = JMeshGeometryPrivate::BufferViewInterface;

				for (uint i = 0; i < gameObjCount; ++i)
				{
					JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
					const uint finalRitemOffset = helper.objectRitemOffset + RFrameUpdateInterface::GetBoundingCBOffset(renderItem);
					if (condition.allowFrustumCulling && !renderItem->IsVisible())
					{
						if (helper.allowRecordCullingResult)
							cullResult[finalRitemOffset] = false;
						continue;
					}

					if (condition.allowHZBOcclusionCulling && hzbOccHelper->IsCulled(finalRitemOffset))
					{
						if (helper.allowRecordCullingResult)
							cullResult[finalRitemOffset] = false;
						continue;
					}
					if (condition.allowAllCullingResult && !cullResult[finalRitemOffset])
						continue;

					if (helper.allowRecordCullingResult)
						cullResult[finalRitemOffset] = true;

					JMeshGeometry* mesh = renderItem->GetMesh().Get();
					const D3D12_VERTEX_BUFFER_VIEW vertexPtr = BufferViewInterface::VertexBufferView(mesh);
					const D3D12_INDEX_BUFFER_VIEW indexPtr = BufferViewInterface::IndexBufferView(mesh);

					commandList->IASetVertexBuffers(0, 1, &vertexPtr);
					commandList->IASetIndexBuffer(&indexPtr);
					commandList->IASetPrimitiveTopology(renderItem->GetPrimitiveType());

					const JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>().Get();
					const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

					if (condition.allowDebugOutline && gameObject[i]->IsSelected())
						commandList->OMSetStencilRef(2);

					for (uint j = 0; j < submeshCount; ++j)
					{
						const JShader* shader = renderItem->GetValidMaterial(j)->GetShader().Get();
						const bool onSkinned = animator != nullptr && condition.allowAnimation;
						const J_MESHGEOMETRY_TYPE meshType = onSkinned ? J_MESHGEOMETRY_TYPE::SKINNED : J_MESHGEOMETRY_TYPE::STATIC;
						const J_SHADER_VERTEX_LAYOUT shaderLayout = JShaderType::ConvertToVertexLayout(meshType);

						if (condition.allowDebugOutline && gameObject[i]->IsSelected())
							commandList->SetPipelineState(shader->GetGraphicExtraPso(shaderLayout, J_GRAPHIC_EXTRA_PSO_TYPE::STENCIL_WRITE_ALWAYS));
						else
							commandList->SetPipelineState(shader->GetGraphicPso(shaderLayout));

						const uint finalObjMeshOffset = (helper.objectMeshOffset +
							RFrameUpdateInterface::GetObjectCBBuffOffset(renderItem) + j);
						D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + finalObjMeshOffset * objectCBByteSize;
						commandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
						if (onSkinned)
						{
							D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + (helper.aniOffset + i) * skinCBByteSize;
							commandList->SetGraphicsRootConstantBufferView(1, skinObjCBAddress);
						}
						if (condition.allowHDOcclusionCulling)
							commandList->SetPredication(graphicResource->GetOcclusionQueryResult(), finalObjMeshOffset * 8, D3D12_PREDICATION_OP_EQUAL_ZERO);

						commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
					}
					if (condition.allowDebugOutline && gameObject[i]->IsSelected())
						commandList->OMSetStencilRef(Constants::commonStencilRef);
				}
			}
			void DrawShadowMapGameObject(ID3D12GraphicsCommandList* cmdList,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const DrawHelper helper,
				const DrawCondition& condition)
			{
				JShader* shadowShader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SHADOW_MAP_SHADER).Get();

				uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
				uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

				auto objectCB = currFrameResource->objectCB->Resource();
				auto skinCB = currFrameResource->skinnedCB->Resource();
				const uint gameObjCount = (uint)gameObject.size();

				using RFrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
				using BufferViewInterface = JMeshGeometryPrivate::BufferViewInterface;

				for (uint i = 0; i < gameObjCount; ++i)
				{
					JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
					if (condition.allowFrustumCulling && !renderItem->IsVisible())
						continue;

					const uint finalRitemOffset = helper.objectRitemOffset + RFrameUpdateInterface::GetBoundingCBOffset(renderItem);
					if (condition.allowHZBOcclusionCulling && hzbOccHelper->IsCulled(finalRitemOffset))
						continue;

					JMeshGeometry* mesh = renderItem->GetMesh().Get();
					const D3D12_VERTEX_BUFFER_VIEW vertexPtr = BufferViewInterface::VertexBufferView(mesh);
					const D3D12_INDEX_BUFFER_VIEW indexPtr = BufferViewInterface::IndexBufferView(mesh);

					commandList->IASetVertexBuffers(0, 1, &vertexPtr);
					commandList->IASetIndexBuffer(&indexPtr);
					commandList->IASetPrimitiveTopology(renderItem->GetPrimitiveType());

					commandList->SetPipelineState(shadowShader->GetGraphicPso(JShaderType::ConvertToVertexLayout(mesh->GetMeshGeometryType())));

					const JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>().Get();
					const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

					for (uint j = 0; j < submeshCount; ++j)
					{
						const bool onSkinned = animator != nullptr && condition.allowAnimation;
						const uint finalObjMeshOffset = (helper.objectMeshOffset +
							RFrameUpdateInterface::GetObjectCBBuffOffset(renderItem) + j);
						D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + finalObjMeshOffset * objectCBByteSize;
						commandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
						if (onSkinned)
						{
							D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + (helper.aniOffset + i) * skinCBByteSize;
							commandList->SetGraphicsRootConstantBufferView(1, skinObjCBAddress);
						}
						if (condition.allowHDOcclusionCulling)
							commandList->SetPredication(graphicResource->GetOcclusionQueryResult(), finalObjMeshOffset * 8, D3D12_PREDICATION_OP_EQUAL_ZERO);

						commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
					}
				}
			}
			void DrawSceneBoundingBox(ID3D12GraphicsCommandList* commandList,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const DrawHelper helper,
				const DrawCondition& condition)
			{
				//JMeshGeometry* mesh = _JResourceManager::Instance().Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE);
				JMeshGeometry* mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE).Get();
				JMaterial* mat = _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST).Get();
				JShader* shader = mat->GetShader().Get();

				using RFrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
				using BufferViewInterface = JMeshGeometryPrivate::BufferViewInterface;

				const D3D12_VERTEX_BUFFER_VIEW vertexPtr = BufferViewInterface::VertexBufferView(mesh);
				const D3D12_INDEX_BUFFER_VIEW indexPtr = BufferViewInterface::IndexBufferView(mesh);

				commandList->IASetVertexBuffers(0, 1, &vertexPtr);
				commandList->IASetIndexBuffer(&indexPtr);
				commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				uint boundingObjectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JBoundingObjectConstants));
				auto boundingObjectCB = currFrameResource->bundingObjectCB->Resource();

				uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));
				auto skinCB = currFrameResource->skinnedCB->Resource();

				const uint gameObjCount = (uint)gameObject.size();
				for (uint i = 0; i < gameObjCount; ++i)
				{
					JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
					if (condition.allowFrustumCulling && !renderItem->IsVisible())
						continue;

					//Test Code
					//수정필요
					//if (JMathHelper::Vector3Length(gameObject[i]->GetTransform()->GetScale()) < 5)
					//	continue;

					commandList->SetPipelineState(shader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::STATIC)));
					const uint index = helper.objectRitemOffset + RFrameUpdateInterface::GetBoundingCBOffset(renderItem);
					D3D12_GPU_VIRTUAL_ADDRESS boundingObjectCBAddress = boundingObjectCB->GetGPUVirtualAddress() + index * boundingObjectCBByteSize;
					commandList->SetGraphicsRootConstantBufferView(6, boundingObjectCBAddress);

					if (condition.allowHZBOcclusionCulling)
						commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
					else if (condition.allowHDOcclusionCulling)
					{
						commandList->BeginQuery(graphicResource->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
						commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
						commandList->EndQuery(graphicResource->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
					}
				}
			}
		public:
			void Initialize()
			{
				AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);

				info.width = JWindow::GetClientWidth();
				info.height = JWindow::GetClientHeight();

				graphicResource = std::make_unique<JGraphicResourceManager>();
				hdOccHelper = std::make_unique<JHardwareOccCulling>();
				hzbOccHelper = std::make_unique<JHZBOccCulling>();
				depthMapDebug = std::make_unique<JDepthMapDebug>();
				outlineHelper = std::make_unique<JOutline>();

				LoadData();
				InitializeD3D();
				InitializeResource();
				OnResize();
				if (cullResult == nullptr)
					cullResult = (bool*)calloc(info.minCapacity, sizeof(bool) * info.minCapacity);
			}
			void Clear()
			{
				if (JApplicationEngine::GetApplicationState() != J_APPLICATION_STATE::PROJECT_SELECT)
					StoreData();

				updateHelper.Clear();
				FlushCommandQueue();
				StartCommand();
				mRootSignature.Reset();

				outlineHelper->Clear();
				outlineHelper.reset();

				depthMapDebug->Clear();
				depthMapDebug.reset();

				hdOccHelper->Clear();
				hdOccHelper.reset();

				hzbOccHelper->Clear();
				hzbOccHelper.reset();
				occBase = nullptr;

				frameResources.clear();
				currFrameResource = nullptr;
				currFrameResourceIndex = 0;

				swapChain.Reset();

				graphicResource->Clear();
				graphicResource.reset();

				EndCommand();
				FlushCommandQueue();

				fence.Reset();
				currentFence = 0;

				commandQueue.Reset();
				directCmdListAlloc.Reset();
				commandList.Reset();

				m4xMsaaState = false;
				m4xMsaaQuality = 0;

				d3dDevice.Reset();
				dxgiFactory.Reset();

				screenViewport = D3D12_VIEWPORT();
				scissorRect = D3D12_RECT();

				currBackBuffer = 0;
				info.width = 0;
				info.height = 0;
				RemoveListener(*JWindow::EvInterface(), guid);
				if (cullResult != nullptr)
				{
					free(cullResult);
					cullResult = nullptr;
				}

#ifdef DEBUG
				HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
				decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));
				IDXGIDebug* debug;
				GetDebugInterface(IID_PPV_ARGS(&debug));
				OutputDebugStringW(L"Starting Live Direct3D Object Dump:\r\n");
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
				OutputDebugStringW(L"Completed Live Direct3D Object Dump.\r\n");
				debug->Release();
#endif
			}
			bool InitializeD3D()
			{
#if defined(DEBUG) || defined(_DEBUG) 
				// Enable the D3D12 DEBUG layer.
				{
					ComPtr<ID3D12Debug> debugController;
					ThrowIfFailedHr(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
					debugController->EnableDebugLayer();
				}
#endif 
				ThrowIfFailedHr(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
				//ThrowIfFailedHr(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,  IID_PPV_ARGS(&dxgiFactory)));

				HRESULT hardwareResult = D3D12CreateDevice(
					nullptr,
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&d3dDevice));

				if (FAILED(hardwareResult))
				{
					ComPtr<IDXGIAdapter> pWrapAdapter;
					ThrowIfFailedHr(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWrapAdapter)));

					ThrowIfFailedHr(D3D12CreateDevice(
						pWrapAdapter.Get(),
						D3D_FEATURE_LEVEL_11_0,
						IID_PPV_ARGS(&d3dDevice)));
				}

				ThrowIfFailedHr(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
				d3dDevice->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Main Device") - 1, "Main Device");
				// Check 4X MSAA quality support for our back buffer format.
				// All Direct3D 11 capable devices support 4X MSAA for all render 
				// target formats, so we only need to check quality support.

				D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
				msQualityLevels.Format = graphicResource->GetBackBufferFormat();
				msQualityLevels.SampleCount = 4;
				msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
				msQualityLevels.NumQualityLevels = 0;
				ThrowIfFailedHr(d3dDevice->CheckFeatureSupport(
					D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
					&msQualityLevels,
					sizeof(msQualityLevels)));

				m4xMsaaQuality = msQualityLevels.NumQualityLevels;
				assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
				LogAdapters();
#endif  
				CreateCommandObjects();
				CreateSwapChain();
				return true;
			}
			bool InitializeResource()
			{
				FlushCommandQueue();
				StartCommand();
				BuildRootSignature();
				graphicResource->Initialize(d3dDevice.Get());
				graphicResource->CreateOcclusionQueryResource(d3dDevice.Get());
				graphicResource->CreateOcclusionHZBResource(d3dDevice.Get(), commandList.Get(), info.occlusionWidth, info.occlusionHeight);
				BuildFrameResources();
				hzbOccHelper->Initialize(d3dDevice.Get(), info);
				depthMapDebug->Initialize(d3dDevice.Get(), graphicResource->GetBackBufferFormat(), graphicResource->GetDepthStencilFormat());
				outlineHelper->Initialize(d3dDevice.Get(), graphicResource->GetBackBufferFormat(), graphicResource->GetDepthStencilFormat(), info.width, info.height, Constants::commonStencilRef);
				EndCommand();
				FlushCommandQueue();
				return true;
			}
			void LogAdapters()
			{
				uint i = 0;
				IDXGIAdapter* adapter = nullptr;
				std::vector<IDXGIAdapter*> adapterList;
				while (dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_ADAPTER_DESC desc;
					adapter->GetDesc(&desc);

					std::wstring text = L"***Adapter: ";
					text += desc.Description;
					text += L"\n";

					OutputDebugString(text.c_str());
					adapterList.push_back(adapter);
					++i;
				}
				for (size_t i = 0; i < adapterList.size(); ++i)
				{
					LogAdapterOutputs(adapterList[i]);
					ReleaseCom(adapterList[i]);
				}
			}
			void LogAdapterOutputs(IDXGIAdapter* adapter)
			{
				uint i = 0;
				IDXGIOutput* output = nullptr;
				while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_OUTPUT_DESC desc;
					output->GetDesc(&desc);

					std::wstring text = L"***Output: ";
					text += desc.DeviceName;
					text += L"\n";
					OutputDebugString(text.c_str());

					LogOutputDisplayModes(output, graphicResource->GetBackBufferFormat());

					ReleaseCom(output);

					++i;
				}
			}
			void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
			{
				uint count = 0;
				uint flags = 0;

				// Call with nullptr to get list count.
				output->GetDisplayModeList(format, flags, &count, nullptr);

				std::vector<DXGI_MODE_DESC> modeList(count);
				output->GetDisplayModeList(format, flags, &count, &modeList[0]);

				for (auto& x : modeList)
				{
					uint n = x.RefreshRate.Numerator;
					uint d = x.RefreshRate.Denominator;
					std::wstring text =
						L"Width = " + std::to_wstring(x.Width) + L" " +
						L"Height = " + std::to_wstring(x.Height) + L" " +
						L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
						L"\n";
					::OutputDebugString(text.c_str());
				}
			}
			void CreateCommandObjects()
			{
				D3D12_COMMAND_QUEUE_DESC queueDesc = {};
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				ThrowIfFailedHr(d3dDevice->CreateCommandQueue(
					&queueDesc,
					IID_PPV_ARGS(&commandQueue)));

				ThrowIfFailedHr(d3dDevice->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(directCmdListAlloc.GetAddressOf())));


				ThrowIfFailedHr(d3dDevice->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					directCmdListAlloc.Get(),
					nullptr,
					IID_PPV_ARGS(commandList.GetAddressOf())));

				commandList->Close();
			}
			void CreateSwapChain()
			{
				swapChain.Reset();
				DXGI_SWAP_CHAIN_DESC sd;
				sd.BufferDesc.Width = info.width;
				sd.BufferDesc.Height = info.height;
				sd.BufferDesc.RefreshRate.Numerator = 60;
				sd.BufferDesc.RefreshRate.Denominator = 1;
				sd.BufferDesc.Format = graphicResource->GetBackBufferFormat();
				sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
				sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
				sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				sd.BufferCount = graphicResource->GetSwapChainBufferCount();
				sd.OutputWindow = Window::JWindowPrivate::HandleInterface::GetHandle();
				sd.Windowed = true;
				sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				// Note: Swap chain uses queue to perform flush.

				ThrowIfFailedHr(dxgiFactory->CreateSwapChain(
					commandQueue.Get(),
					&sd,
					swapChain.GetAddressOf()));
			}
			void BuildRootSignature()
			{
				// Root parameter can be a table, root descriptor or root constants.
				CD3DX12_DESCRIPTOR_RANGE cubeMapTable;
				cubeMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingCubeMapCapacity, 2, 1);

				CD3DX12_DESCRIPTOR_RANGE tex2DTable;
				tex2DTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.binding2DTextureCapacity, 2, 2);

				CD3DX12_DESCRIPTOR_RANGE shadowMapTable;
				shadowMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCapacity, 2, 3);

				static constexpr int slotCount = 13;
				CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

				// Create root CBV.
				slotRootParameter[0].InitAsConstantBufferView(0);
				slotRootParameter[1].InitAsConstantBufferView(1);
				slotRootParameter[2].InitAsConstantBufferView(2);
				slotRootParameter[3].InitAsConstantBufferView(3);
				slotRootParameter[4].InitAsConstantBufferView(4);
				slotRootParameter[5].InitAsConstantBufferView(5);
				slotRootParameter[6].InitAsConstantBufferView(6);

				//Light Buff
				slotRootParameter[7].InitAsShaderResourceView(0);
				slotRootParameter[8].InitAsShaderResourceView(0, 1);
				//Material Buff
				slotRootParameter[9].InitAsShaderResourceView(1);
				//Texture Buff
				slotRootParameter[10].InitAsDescriptorTable(1, &cubeMapTable, D3D12_SHADER_VISIBILITY_ALL);
				slotRootParameter[11].InitAsDescriptorTable(1, &tex2DTable, D3D12_SHADER_VISIBILITY_ALL);
				slotRootParameter[12].InitAsDescriptorTable(1, &shadowMapTable, D3D12_SHADER_VISIBILITY_ALL);

				const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Sampler();

				// A root signature is an array of root parameters.
				CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter,
					(uint)sam.size(), sam.data(),
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

				// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
				ComPtr<ID3DBlob> serializedRootSig = nullptr;
				ComPtr<ID3DBlob> errorBlob = nullptr;
				HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
					serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

				if (errorBlob != nullptr)
				{
					::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				}
				ThrowIfFailedHr(hr);
				ThrowIfFailedHr(d3dDevice->CreateRootSignature(
					0,
					serializedRootSig->GetBufferPointer(),
					serializedRootSig->GetBufferSize(),
					IID_PPV_ARGS(mRootSignature.GetAddressOf())));

				mRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Main RootSignature") - 1, "Main RootSignature");
			}
			void BuildFrameResources()
			{
				for (int i = 0; i < Constants::gNumFrameResources; ++i)
					frameResources.push_back(std::make_unique<JFrameResource>(d3dDevice.Get(), info));
				currFrameResource = frameResources[currFrameResourceIndex].get();
			}
		public:
			void StoreData()
			{
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::SettingPath(), L"GraphicOption.txt");
				std::wofstream stream;
				stream.open(path, std::ios::binary | std::ios::out);
				if (!stream.is_open())
					return;

				JFileIOHelper::StoreJString(stream, L"--Info--", L"");
				JFileIOHelper::StoreAtomicData(stream, L"UploadObjCount:", info.upObjCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadAniCount:", info.upAniCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadPassCount:", info.upPassCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadCameraCount:", info.upCameraCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadLightCount:", info.upLightCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadShadowMapLightCount:", info.upSmLightCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadMaterialCount:", info.upMaterialCount);
				JFileIOHelper::StoreAtomicData(stream, L"Bind2DTextureCount:", info.binding2DTextureCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindCubeMapCount:", info.bindingCubeMapCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindShadowTextureCount:", info.bindingShadowTextureCapacity);

				JFileIOHelper::StoreJString(stream, L"--Option--", L"");
				JFileIOHelper::StoreAtomicData(stream, L"AllowOcclusionQuery:", option.isOcclusionQueryActivated);
				JFileIOHelper::StoreAtomicData(stream, L"HardwareOcclusionAcitvated:", option.isHDOcclusionAcitvated);
				JFileIOHelper::StoreAtomicData(stream, L"HZBOcclusionAcitvated:", option.isHZBOcclusionActivated);
				JFileIOHelper::StoreAtomicData(stream, L"AllowHZBCorrectFail:", option.allowHZBCorrectFail);
				JFileIOHelper::StoreAtomicData(stream, L"AllowHZBDepthMapDebug:", option.allowHZBDepthMapDebug);
				JFileIOHelper::StoreAtomicData(stream, L"AllowOutline:", option.allowDebugOutline);
				stream.close();
			}
			void LoadData()
			{
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::SettingPath(), L"GraphicOption.txt");
				std::wifstream stream;
				stream.open(path, std::ios::binary | std::ios::in);
				if (!stream.is_open())
					return;

				std::wstring guide;
				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, info.upObjCount);
				JFileIOHelper::LoadAtomicData(stream, info.upAniCount);
				JFileIOHelper::LoadAtomicData(stream, info.upPassCount);
				JFileIOHelper::LoadAtomicData(stream, info.upCameraCount);
				JFileIOHelper::LoadAtomicData(stream, info.upLightCount);
				JFileIOHelper::LoadAtomicData(stream, info.upSmLightCount);
				JFileIOHelper::LoadAtomicData(stream, info.upMaterialCount);
				JFileIOHelper::LoadAtomicData(stream, info.binding2DTextureCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingCubeMapCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingShadowTextureCapacity);

				JGraphicOption newOption;
				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, newOption.isOcclusionQueryActivated);
				JFileIOHelper::LoadAtomicData(stream, newOption.isHDOcclusionAcitvated);
				JFileIOHelper::LoadAtomicData(stream, newOption.isHZBOcclusionActivated);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowHZBCorrectFail);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowHZBDepthMapDebug);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowDebugOutline);
				stream.close();
				SetGraphicOption(newOption);
			}
			void WriteLastRsTexture()
			{
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				{
					// 0 is main camera

					auto mainCamRs = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
					JApplicationProjectInfo* opendInfo = JApplicationProject::GetOpenProjectInfo();
					if (opendInfo == nullptr)
						MessageBox(0, L"get open proejct info error", 0, 0);

					SaveDDSTextureToFile(commandQueue.Get(), mainCamRs, opendInfo->lastRsPath().c_str());
				}
			}
		public:
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
			{
				if (senderGuid == guid)
					return;

				if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
					OnResize();
			}
		};
#pragma endregion


#pragma region Interface

		JGraphicInfo JGraphic::GetGraphicInfo()const noexcept
		{
			return impl->GetGraphicInfo();
		}
		JGraphicOption JGraphic::GetGraphicOption()const noexcept
		{
			return impl->GetGraphicOption();
		}
		void JGraphic::SetGraphicOption(JGraphicOption newGraphicOption)noexcept
		{
			impl->SetGraphicOption(newGraphicOption);
		}
		JGraphic::JGraphic()
			:impl(std::make_unique<JGraphicImpl>(Core::MakeGuid(), this))
		{}
		JGraphic::~JGraphic()
		{
			impl.reset();
		}

		using DeviceInterface = JGraphicPrivate::DeviceInterface;
		using ResourceInterface = JGraphicPrivate::ResourceInterface;
		using EditorInterface = JGraphicPrivate::EditorInterface;
		using DebugInterface = JGraphicPrivate::DebugInterface;
		using CommandInterface = JGraphicPrivate::CommandInterface;
		using AppAccess = JGraphicPrivate::AppAccess; 

		ID3D12Device* DeviceInterface::GetDevice() noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetDevice();
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceInterface::GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCpuDescriptorHandle(bType, index);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceInterface::GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetGpuDescriptorHandle(bType, index);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceInterface::GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
			const J_GRAPHIC_BIND_TYPE bType,
			int rIndex,
			int bIndex)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCpuDescriptorHandle(rType, bType, rIndex, bIndex);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceInterface::GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
			const J_GRAPHIC_BIND_TYPE bType,
			int rIndex,
			int bIndex)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetGpuDescriptorHandle(rType, bType, rIndex, bIndex);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)
		{
			return JinEngine::JGraphic::Instance().impl->Create2DTexture(uploadHeap, path, oriFormat);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)
		{
			return JinEngine::JGraphic::Instance().impl->CreateCubeMap(uploadHeap, path, oriFormat);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateRenderTargetTexture(textureWidth, textureHeight);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateShadowMapTexture(textureWidth, textureHeight);
		}
		bool ResourceInterface::DestroyGraphicTextureResource(JGraphicResourceInfo* info)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyGraphicTextureResource(info);
		}
		void ResourceInterface::StuffGraphicShaderPso(JGraphicShaderData* shaderData,
			const J_SHADER_VERTEX_LAYOUT vertexLayout,
			const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JShaderGraphicPsoCondition& psoCondition,
			const J_GRAPHIC_EXTRA_PSO_TYPE extraType)
		{
			JinEngine::JGraphic::Instance().impl->StuffGraphicShaderPso(shaderData, vertexLayout, gFunctionFlag, psoCondition, extraType);
		}
		void ResourceInterface::StuffComputeShaderPso(JComputeShaderData* shaderData, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		{
			JinEngine::JGraphic::Instance().impl->StuffComputeShaderPso(shaderData, cFunctionFlag);
		}

		void EditorInterface::SetImGuiBackEnd()
		{
			JinEngine::JGraphic::Instance().impl->SetImGuiBackEnd();
		}

		JGraphicResourceManager* DebugInterface::GetGraphicResourceManager()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->graphicResource.get();
		}

		ID3D12CommandQueue* CommandInterface::GetCommandQueue()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCommandQueue();
		}
		ID3D12CommandAllocator* CommandInterface::GetCommandAllocator()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCommandAllocator();
		}
		ID3D12GraphicsCommandList* CommandInterface::GetCommandList()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCommandList();
		}
		void CommandInterface::StartCommand()
		{
			JinEngine::JGraphic::Instance().impl->StartCommand();
		}
		void CommandInterface::EndCommand()
		{
			JinEngine::JGraphic::Instance().impl->EndCommand();
		}
		void CommandInterface::FlushCommandQueue()
		{
			JinEngine::JGraphic::Instance().impl->FlushCommandQueue();
		}

		void AppAccess::Initialize()
		{
			JinEngine::JGraphic::Instance().impl->Initialize();
		}
		void AppAccess::Clear()
		{
			JinEngine::JGraphic::Instance().impl->Clear();
		}
		void AppAccess::StartFrame()
		{
			JinEngine::JGraphic::Instance().impl->StartFrame();
		}
		void AppAccess::EndFrame()
		{
			JinEngine::JGraphic::Instance().impl->EndFrame();
		}
		void AppAccess::UpdateWait()
		{
			JinEngine::JGraphic::Instance().impl->UpdateWait();
		}
		void AppAccess::UpdateEngine()
		{
			JinEngine::JGraphic::Instance().impl->UpdateEngine();
		}
		void AppAccess::DrawScene()
		{
			JinEngine::JGraphic::Instance().impl->DrawScene();
		}
		void AppAccess::DrawProjectSelector()
		{
			JinEngine::JGraphic::Instance().impl->DrawProjectSelector();
		}
		void AppAccess::WriteLastRsTexture()
		{
			JinEngine::JGraphic::Instance().impl->WriteLastRsTexture();
		}
#pragma endregion
	}
}