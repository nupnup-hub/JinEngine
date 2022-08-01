#include"JFrameResource.h"
#include"../../Core/Exception/JExceptionMacro.h"
 
#include"JObjectConstants.h" 
#include"JAnimationConstants.h"
#include"JMaterialConstants.h"
#include"JPassConstants.h" 
#include"JCameraConstants.h"
#include"JLightConstants.h"
#include"JShadowMapCalConstants.h"

namespace JinEngine
{
    namespace Graphic
    {
        JFrameResource::JFrameResource(ID3D12Device* device,
            const uint initMateiralCount,
            const uint initRenderItemCount,
            const uint initAnimatorCount,
            const uint initSceneCount,
            const uint initCameraCount,
            const uint initSceneShadowCount)
        {
            ThrowIfFailedHr(device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

            /*
            std::unique_ptr<JUploadBuffer<JMaterialConstants>> materialBuffer = nullptr;
            std::unique_ptr<JUploadBuffer<TransformConstants>> transformCB = nullptr;
            std::unique_ptr<JUploadBuffer<RenderItemConstants>> renderItemCB = nullptr;
            std::unique_ptr<JUploadBuffer<JAnimationConstants>> skinnedCB = nullptr;
            std::unique_ptr<JUploadBuffer<JPassConstants>> passCB = nullptr;
            std::unique_ptr<JUploadBuffer<RenderTargetConstants>> renderTargetCB = nullptr;
            std::unique_ptr<JUploadBuffer<JCameraConstants>> cameraCB = nullptr;
            */
            materialBuffer = std::make_unique<JUploadBuffer<JMaterialConstants>>(device, initMateiralCount, false);
            objectCB = std::make_unique<JUploadBuffer<JObjectConstants>>(device, initRenderItemCount, true);
            skinnedCB = std::make_unique<JUploadBuffer<JAnimationConstants>>(device, initAnimatorCount, true);
            passCB = std::make_unique<JUploadBuffer<JPassConstants>>(device, initSceneCount, true);
            cameraCB = std::make_unique<JUploadBuffer<JCameraConstants>>(device, initCameraCount, true);
            lightCB = std::make_unique<JUploadBuffer<JLightConstants>>(device, initSceneCount, true);
            shadowCalCB = std::make_unique<JUploadBuffer<JShadowMapCalConstants>>(device, initSceneCount * initSceneShadowCount, true);
        }

        JFrameResource::~JFrameResource()
        {

        }
    }
}