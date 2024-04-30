#include"JDx12GraphicResourceManager.h"  
#include"JDx12GraphicResourceInfo.h"
#include"JDx12GraphicResourceCreation.h" 
#include"JLoadTextureFromFile.h"
#include"../Dx/JDx12GraphicResourceHolder.h"
#include"../JGraphicResourceInterface.h"
#include"../../Device/JGraphicDevice.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Utility/Dx/JDx12Utility.h" 
#include"../../JGraphic.h"

#include"../../../Core/Utility/JCommonUtility.h"  
#include"../../../Core/Exception/JExceptionMacro.h"  
#include"../../../Core/File/JFileConstant.h"  
#include"../../../Core/Log/JLogMacro.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"   
#include"../../../Window/JWindow.h"
#include"../../../../ThirdParty/DirectX/DDSTextureLoader11.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/ScreenGrab.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/WICTextureLoader.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/ResourceUploadBatch.h" 
#include"../../../../ThirdParty/Tif/tiffio.h"
#include<DirectXColors.h> 
 
#define GetNextViewIndexLam [](JDx12GraphicResourceManager* gm, J_GRAPHIC_RESOURCE_TYPE rType, J_GRAPHIC_BIND_TYPE bindType)	\
	->uint																					\
{																							\
	return gm->typeDesc[(uint)rType].viewInfo[(uint)bindType].GetNextViewIndex();			\
};																							\

#define AddViewIndexLam [](JDx12GraphicResourceManager* gm, J_GRAPHIC_RESOURCE_TYPE rType, J_GRAPHIC_BIND_TYPE bindType)	\
{																							\
	++gm->typeDesc[(uint)rType].viewInfo[(uint)bindType].count;							\
};				

//Debug
//#include<fstream>
//#include"../../Core/File/JFileIOHelper.h"  
namespace JinEngine::Graphic
{
	using CreationClass = JDx12GraphicResourceCreation;
	/**
	* 새로운 type추가시 반드시 여기에 해당하는 Attribute사항을 추가해야한다.
	* Attribute는 type별 상숫값에 집합이다.
	*/
	class JDx12GraphicResourceTypeAttribute : public JGraphicResourceTypeAttribute
	{
	public:
		static constexpr uint innerResourceInitCapacity = 8;
		static constexpr float exposureInitData[Constants::exposureBufferCount]
		{
			Constants::exposureDefaultValue,
			1.0f / Constants::exposureDefaultValue,
			Constants::exposureDefaultValue,
			0.0f,
			Constants::exposureDefaultMinLog,
			Constants::exposureDefaultMaxLog,
			 Constants::exposureDefaultMaxLog - Constants::exposureDefaultMinLog,
			1.0f / (Constants::exposureDefaultMaxLog - Constants::exposureDefaultMinLog)
		};
	public:
		static uint InitViewFixedCount(const J_GRAPHIC_BIND_TYPE bType)
		{
			switch (bType)
			{
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
				return 1;	//Imgui reserved
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
				return 0;
			default:
				return 0;
			}
		}
		static uint InitCapacity(const J_GRAPHIC_RESOURCE_TYPE type)
		{
			switch (type)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
				return Constants::swapChainBufferCount;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
				return 512;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
				return 16;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
				return 512;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
				return 16 + 16; //(hzb + hd)
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				return 16;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
				return 16;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				return 1024;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
				return 128;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
				return 256;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
				return 512;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
				return 512 * 3;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
				return 256;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
				return 16;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
				return 128;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
				return 16;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
				return 16;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
				return 16;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP:
				return innerResourceInitCapacity;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING:
				return 32;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE:
				return innerResourceInitCapacity;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
				return 4;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
				return 4 * 2;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON:
				return innerResourceInitCapacity * 4;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON:
				return innerResourceInitCapacity;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
				return Constants::vertexBufferCapacity;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
				return Constants::indexBufferCapacity;
			default:
				return 0;
			}
		}
		static uint InitViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		{
			const uint initResourceCapacity = InitCapacity(rType);
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity + initResourceCapacity;	//detph & stencil
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return initResourceCapacity;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity * Constants::occlusionMipmapViewCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return initResourceCapacity * Constants::occlusionMipmapViewCapacity;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return initResourceCapacity;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return initResourceCapacity;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return initResourceCapacity;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return initResourceCapacity;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return initResourceCapacity;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return initResourceCapacity;
				default:
					break;
				}
				break;
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					return 0;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return InitCapacity(rType) * InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType) * InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					return 0;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return InitCapacity(rType);
				default:
					return 0;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return InitCapacity(rType);
				default:
					return 0;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					break;
				}
				break;
			}
			default:
			{
				break;
			}
			}
			return 0;
		}
		static uint InitViewCapacity(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		{
			if (!CanUseOption(opType, rType))
				return 0;

			switch (opType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP: 
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					return 0;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return InitCapacity(rType);
				default:
					return 0;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return InitCapacity(rType);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return InitCapacity(rType);
				default:
					return 0;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return 0;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return 0;
				default:
					return 0;
				}
			}
			default:
				return 0;
			}
		}
		static uint InitAdditionalViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		{
			uint sum = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT; ++i)
			{
				J_GRAPHIC_RESOURCE_OPTION_TYPE opType = (J_GRAPHIC_RESOURCE_OPTION_TYPE)i;
				if (!CanUseOption(opType, rType))
					continue;

				sum += InitViewCapacity(opType, rType, bType);
			}
			return sum;
		}
		static uint InitTotalViewCapaicty(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		{
			return InitViewCapacity(rType, bType) + InitAdditionalViewCapacity(rType, bType);
		}
	public:
		static uint IsBuffer(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_FXAA_COLOR_QUEUE:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::VERTEX:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::INDEX:
				return true;
			default:
				return false;
			}
		}
		static uint IsBuffer(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			switch (opType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
				return true;
			default:
				return false;
			}
		}
		static bool IsFixedBindType(const J_GRAPHIC_RESOURCE_TYPE type)
		{
			switch (type)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:					//has optional uav for IBL...
				return false;
			default:
				return true;
			}
		} 
		static bool IsFixedBindType(const J_GRAPHIC_RESOURCE_TYPE type, const J_GRAPHIC_BIND_TYPE bType)
		{
			switch (type)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return false;
				default:
					return true;
				}
			}
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:					 
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return true;
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return false;
				default:
					return true;
				}
			}
			default:
				return true;
			}
		}
		static bool IsFixedFormat(const J_GRAPHIC_RESOURCE_TYPE type)
		{
			switch (type)
			{
			case J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
			case J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
			case J_GRAPHIC_RESOURCE_TYPE::BYTE_BUFFER_COMMON:
			case J_GRAPHIC_RESOURCE_TYPE::STRUCTURE_BUFFER_COMMON:
				return false;
			default:
				return true;
			}
		}
		static bool TrySetInitBufferPointer(const J_GRAPHIC_RESOURCE_TYPE rType, JUploadBufferCreationDesc* desc)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
			{
				desc->data = exposureInitData;
				desc->bufferSize = Constants::exposureBufferCount * sizeof(float);
				return true;
			};
			default:
				return false;
			}
		}
	public:	
		static bool GraphicOptionNeedRebind(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)noexcept
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
			{
				switch (opType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
					return false;
				default:
					break;
				}
			}
			}
			return true;
		}
		static bool CanCreateMipmap(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
				return true;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
				return true;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
				return true;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
				return true;
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON:
				return true;
			default:
				return false;
			}
		}
		static uint MPBCount(const J_GRAPHIC_BIND_TYPE bType)
		{
			switch (bType)
			{
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
				return 0;
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
				return JDx12GraphicResourceManager::MPBCapactiy();
			case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
				return JDx12GraphicResourceManager::MPBCapactiy();
			default:
				return 0;
			}
		} 
	};
	using JDx12TypeAttribute = JDx12GraphicResourceTypeAttribute;
 
	JDx12GraphicResourceManager::~JDx12GraphicResourceManager()
	{
		ClearResource();
	}
	/*
	* create resource senario
	1. Create resource and info(Owner Pointer)
	2. caller(impl) is derived interface(has ginfo and access gmanager public method)
	3. impl interface class can derived user access class(restrict version impl derived interface)
	*/
	void JDx12GraphicResourceManager::Initialize(JGraphicDevice* device)
	{
		if (!IsSameDevice(device))
			return;

		BuildResource(device);
		//Debug
		/*
		std::wofstream stream;
		stream.open(L"D:\\JinWooJung\\gDebug.txt", std::ios::app | std::ios::out);

		Core::JEnumInfo* rInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
		Core::JEnumInfo* bInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
		auto rNameVec = rInfo->GetEnumNameVec();
		auto bNameVec = bInfo->GetEnumNameVec();

		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		{
			JFileIOHelper::StoreJString(stream, L"RType:", JCUtil::StrToWstr(rNameVec[i]));
			for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
			{
				JFileIOHelper::StoreJString(stream, L"BType:", JCUtil::StrToWstr(bNameVec[j]));
				JFileIOHelper::StoreAtomicData(stream, L"Count:", typeDesc[i].viewInfo[j].count);
				JFileIOHelper::StoreAtomicData(stream, L"Capacity:", typeDesc[i].viewInfo[j].capacity);
				JFileIOHelper::StoreAtomicData(stream, L"Offset:", typeDesc[i].viewInfo[j].offset);
			}
			JFileIOHelper::InputSpace(stream, 1);
		}
		JFileIOHelper::InputSpace(stream, 2);
		stream.close();
		*/
	}
	void JDx12GraphicResourceManager::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicResourceManager::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuRtvDescriptorHandle(int index)const noexcept
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), index, rtvDescriptorSize);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuRtvDescriptorHandle(int index)const noexcept
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(rtvHeap->GetGPUDescriptorHandleForHeapStart(), index, rtvDescriptorSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuDsvDescriptorHandle(int index)const noexcept
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart(), index, dsvDescriptorSize);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuDsvDescriptorHandle(int index)const noexcept
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(dsvHeap->GetGPUDescriptorHandleForHeapStart(), index, dsvDescriptorSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuSrvDescriptorHandle(int index)const noexcept
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart(), index, cbvSrvUavDescriptorSize);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuSrvDescriptorHandle(int index)const noexcept
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(srvHeap->GetGPUDescriptorHandleForHeapStart(), index, cbvSrvUavDescriptorSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
	{
		switch (bType)
		{
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
			return GetCpuRtvDescriptorHandle(index);
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
			return GetCpuDsvDescriptorHandle(index);
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
			return GetCpuSrvDescriptorHandle(index);
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
			return GetCpuSrvDescriptorHandle(index);
		default:
			return CD3DX12_CPU_DESCRIPTOR_HANDLE();
		}
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
	{
		switch (bType)
		{
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
			return GetGpuRtvDescriptorHandle(index);
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
			return GetGpuDsvDescriptorHandle(index);
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
			return GetGpuSrvDescriptorHandle(index);
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
			return GetGpuSrvDescriptorHandle(index);
		default:
			return CD3DX12_GPU_DESCRIPTOR_HANDLE();
		}
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
		const J_GRAPHIC_BIND_TYPE bType,
		const int rIndex,
		const int bIndex)
	{
		return GetCpuDescriptorHandle(bType, resource[(int)rType][rIndex]->GetHeapIndexStart(bType) + bIndex);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
		const J_GRAPHIC_BIND_TYPE bType,
		const int rIndex,
		const int bIndex)
	{
		return GetGpuDescriptorHandle(bType, resource[(int)rType][rIndex]->GetHeapIndexStart(bType) + bIndex);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetFirstGpuSrvDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
	{
		return GetGpuSrvDescriptorHandle(typeDesc[(int)rType].viewInfo[(int)J_GRAPHIC_BIND_TYPE::SRV].offset);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetMPBCpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		auto info = mpb.Get(handle);
		return info != nullptr ? GetCpuDescriptorHandle(bType, info->index[(uint)bType]) : CD3DX12_CPU_DESCRIPTOR_HANDLE();
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceManager::GetMPBGpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		auto info = mpb.Get(handle);
		return info != nullptr ? GetGpuDescriptorHandle(bType, info->index[(uint)bType]) : CD3DX12_GPU_DESCRIPTOR_HANDLE();
	}
	ID3D12DescriptorHeap* JDx12GraphicResourceManager::GetDescriptorHeap(const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		switch (bType)
		{
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
			return rtvHeap.Get();
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
			return dsvHeap.Get();
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
			return srvHeap.Get();
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
			return srvHeap.Get();
		default:
			return nullptr;
		}
	}
	uint JDx12GraphicResourceManager::GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		switch (bType)
		{
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
			return rtvDescriptorSize;
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
			return dsvDescriptorSize;
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
			return cbvSrvUavDescriptorSize;
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
			return cbvSrvUavDescriptorSize;
		default:
			return 0;
		}
	}
	D3D12_VERTEX_BUFFER_VIEW JDx12GraphicResourceManager::VertexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept
	{
		const int arrayIndex = mesh->GraphicResourceUserInterface().GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::VERTEX, 0);
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = GetResource(J_GRAPHIC_RESOURCE_TYPE::VERTEX, arrayIndex)->GetGPUVirtualAddress();
		vbv.StrideInBytes = mesh->GetVertexByteSize();
		vbv.SizeInBytes = mesh->GetVertexBufferByteSize();
		return vbv;
	}
	D3D12_INDEX_BUFFER_VIEW JDx12GraphicResourceManager::IndexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept
	{
		const int arrayIndex = mesh->GraphicResourceUserInterface().GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::INDEX, 0);
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = GetResource(J_GRAPHIC_RESOURCE_TYPE::INDEX, arrayIndex)->GetGPUVirtualAddress();
		ibv.Format = mesh->GetIndexByteSize() == sizeof(uint32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		ibv.SizeInBytes = mesh->GetIndexBufferByteSize();
		return ibv;
	}
	uint JDx12GraphicResourceManager::GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
	{
		return typeDesc[(int)rType].count;
	}
	uint JDx12GraphicResourceManager::GetResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
	{
		return typeDesc[(int)rType].capacity;
	}
	uint JDx12GraphicResourceManager::GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		return typeDesc[(int)rType].viewInfo[(int)bType].count;
	}
	uint JDx12GraphicResourceManager::GetViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		return typeDesc[(int)rType].viewInfo[(int)bType].capacity;
	}
	uint JDx12GraphicResourceManager::GetViewOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		return typeDesc[(int)rType].viewInfo[(int)bType].offset;
	}
	uint JDx12GraphicResourceManager::GetTotalViewCount(const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		uint total = 0;
		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			total += typeDesc[i].viewInfo[(int)bType].count;
		return total;
	}
	uint JDx12GraphicResourceManager::GetTotalViewCapacity(const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		uint total = 0;
		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			total += typeDesc[i].viewInfo[(int)bType].capacity;
		return total;
	}
	ResourceHandle JDx12GraphicResourceManager::GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
	{
		return (ResourceHandle)GetGpuDescriptorHandle(bType, index).ptr;
	}
	ResourceHandle JDx12GraphicResourceManager::GetMPBResourceCpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		return (ResourceHandle)GetMPBCpuDescriptorHandle(handle, bType).ptr;
	}
	ResourceHandle JDx12GraphicResourceManager::GetMPBResourceGpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		return (ResourceHandle)GetMPBGpuDescriptorHandle(handle, bType).ptr;
	}
	ID3D12Resource* JDx12GraphicResourceManager::GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
	{
		return resource[(int)rType][index]->resourceHolder->GetResource();
	}
	ID3D12Resource* JDx12GraphicResourceManager::GetOptionResource(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, int index)const noexcept
	{
		return resource[(int)rType][index]->optionHolderSet->holder[(uint)opType]->GetResource();
	}
	JGraphicResourceInfo* JDx12GraphicResourceManager::GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
	{
		return index != invalidIndex ? resource[(int)rType][index].Get() : nullptr;
	}
	JGraphicResourceInfo* JDx12GraphicResourceManager::GetMPBInfo(const Core::JDataHandle& handle)const noexcept
	{
		auto info = mpb.Get(handle);
		return info != nullptr ? (info->info.Get()) : nullptr;
	}
	JDx12GraphicResourceInfo* JDx12GraphicResourceManager::GetDxInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
	{
		return index != invalidIndex ? resource[(int)rType][index].Get() : nullptr;
	}
	JDx12GraphicResourceInfo* JDx12GraphicResourceManager::GetDefaultSceneDsInfo()const noexcept
	{
		return static_cast<JDx12GraphicResourceInfo*>(defaultSceneDsInfo.Get());
	}
	JDx12GraphicResourceHolder* JDx12GraphicResourceManager::GetDxHolder(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept
	{
		return resource[(int)rType][index]->resourceHolder.get();
	}
	JDx12GraphicResourceHolder* JDx12GraphicResourceManager::GetOptionHolder(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, int index)const noexcept
	{
		return resource[(int)rType][index]->optionHolderSet->holder[(uint)opType].get();
	}
	uint JDx12GraphicResourceManager::GetHeapIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
	{
		return typeDesc[(int)rType].viewInfo[(int)bType].count + typeDesc[(int)rType].viewInfo[(int)bType].offset;
	}
	void JDx12GraphicResourceManager::SetViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const int nextViewIndex)
	{
		if (handlePtr->GetViewCount(bType) == 0)
		{
			handlePtr->SetHeapIndexStart(bType, nextViewIndex);
			handlePtr->SetViewCount(bType, 1);
		}
		else
		{
			const int viewCount = handlePtr->GetViewCount(bType) + 1;
			handlePtr->SetViewCount(bType, viewCount);
		}
	}
	void JDx12GraphicResourceManager::SetOptionViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const int nextViewIndex)
	{
		if (handlePtr->GetOptionViewCount(bType, opType) == 0)
		{
			handlePtr->SetHeapOptionIndexStart(bType, opType, nextViewIndex);
			handlePtr->SetOptionViewCount(bType, opType, 1);
		}
		else
		{
			const int viewCount = handlePtr->GetOptionViewCount(bType, opType) + 1;
			handlePtr->SetOptionViewCount(bType, opType, viewCount);
		}
	}
	bool JDx12GraphicResourceManager::CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
	{
		auto& desc = typeDesc[(uint)rType];
		bool hasSpace = true;
		for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
			hasSpace &= JDx12TypeAttribute::InitViewCapacity(rType, (J_GRAPHIC_BIND_TYPE)i) > 0 ? desc.viewInfo[i].HasSpace() : true;
		return desc.HasSpace() & hasSpace;
	}
	bool JDx12GraphicResourceManager::CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType, JGraphicDevice* device)noexcept
	{
		if (!IsSameDevice(device))
			return false;

		return CanCreateResource(rType);
	}
	bool JDx12GraphicResourceManager::CanCreateOptionResource(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
	{
		auto& desc = typeDesc[(uint)rType];
		bool hasSpace = true;
		if (JDx12TypeAttribute::IsBuffer(opType))
			hasSpace = true;
		else
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				hasSpace &= JDx12TypeAttribute::InitViewCapacity(opType, rType, (J_GRAPHIC_BIND_TYPE)i) > 0 ? desc.viewInfo[i].HasSpace() : true;
		}
		return desc.HasSpace() & JDx12TypeAttribute::CanUseOption(opType, rType) & hasSpace;
	}
	bool JDx12GraphicResourceManager::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		if (type == JGraphicInfo::TYPE::FRAME)
			return true;
		else
			return false;
	}
	bool JDx12GraphicResourceManager::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::CULLING || type == JGraphicOption::TYPE::RENDERING)
			return true;
		else
			return false;
	}
	void JDx12GraphicResourceManager::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicInfoChangedSet&>(set);
		if (dx12Set.preInfo.frame.upPLightCapacity != dx12Set.newInfo.frame.upPLightCapacity ||
			dx12Set.preInfo.frame.upSLightCapacity != dx12Set.newInfo.frame.upSLightCapacity ||
			dx12Set.preInfo.frame.upRLightCapacity != dx12Set.newInfo.frame.upRLightCapacity)
		{
			const JGraphicOption& gOption = GetGraphicOption();
			JGraphicResourceCreationDesc lightRtDesc;
			lightRtDesc.width = gOption.GetClusterXCount();
			lightRtDesc.height = gOption.GetClusterYCount();
			lightRtDesc.arraySize = max(max(dx12Set.newInfo.frame.upPLightCapacity, dx12Set.newInfo.frame.upSLightCapacity), dx12Set.newInfo.frame.upRLightCapacity);

			ReAllocTypePerAllResource(dx12Set.device, lightRtDesc, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING);
		}
	}
	void JDx12GraphicResourceManager::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.changedPart == JGraphicOption::TYPE::CULLING &&
			set.preOption.culling.clusterXIndex != set.newOption.culling.clusterXIndex &&
			set.preOption.culling.clusterYIndex != set.newOption.culling.clusterYIndex &&
			set.preOption.culling.clusterZIndex != set.newOption.culling.clusterZIndex &&
			set.preOption.culling.lightPerClusterIndex != set.newOption.culling.lightPerClusterIndex &&
			set.preOption.culling.clusterNear != set.newOption.culling.clusterNear)
		{ 
			const JGraphicInfo& gInfo = GetGraphicInfo();
			JGraphicResourceCreationDesc lightRtDesc;
			lightRtDesc.width = set.newOption.GetClusterXCount();
			lightRtDesc.height = set.newOption.GetClusterYCount();
			lightRtDesc.arraySize = max(max(gInfo.frame.upPLightCapacity, gInfo.frame.upSLightCapacity), gInfo.frame.upRLightCapacity);

			ReAllocTypePerAllResource(dx12Set.device, lightRtDesc, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING);

			JGraphicResourceCreationDesc linkedDesc;
			linkedDesc.width = set.newOption.GetClusterIndexCount();

			JGraphicResourceCreationDesc offsetDesc;
			offsetDesc.width = set.newOption.GetClusterTotalCount();

			ReAllocTypePerAllResource(dx12Set.device, linkedDesc, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST);
			ReAllocTypePerAllResource(dx12Set.device, offsetDesc, J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET);
		}
		else if (set.changedPart == JGraphicOption::TYPE::RENDERING && 
			set.preOption.rendering.renderTargetFormat != set.newOption.rendering.renderTargetFormat)
		{
			ReAllocTypePerAllResource(dx12Set.device, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
		}
	}
	void JDx12GraphicResourceManager::CreateSwapChainBuffer(JDx12SwapChainCreationData& creationData)
	{
		static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN;
		JDx12GraphicResourceManager::ResourceTypeDesc& swapChainDesc = typeDesc[(int)rType];
		resource[(int)rType].clear();
		swapChainDesc.ClearCount();

		CreationClass::CreateSwapChainBuffer(GetWindowHandle(), creationData, GetGraphicOption()); 
		//resource[(int)rType].resize(JDx12TypeAttribute::swapChainBufferCount);
		for (uint i = 0; i < Constants::swapChainBufferCount; ++i)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
			ThrowIfFailedHr((*creationData.swapChain)->GetBuffer(i, IID_PPV_ARGS(&newResource)));
			//일반적으로 swap chain state을 수동으로 관리하지않는다.
			auto info = CreateResourceInfo(rType, JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_COMMON));

			auto getNextViewIndexLam = GetNextViewIndexLam;
			auto addViewIndexLam = AddViewIndexLam;
			JBindDesc bindDesc(this, creationData.device, getNextViewIndexLam, addViewIndexLam, info.Get());
			CreationClass::Bind(bindDesc);
		}
	}
	JUserPtr<JGraphicResourceInfo> JDx12GraphicResourceManager::CreateResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
	{ 
		if (device == nullptr || !creationDesc.IsValid())
			return nullptr;

		switch (rType)
		{
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:
			return nullptr;			//invalid call  
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
			return CreateTexture2D(device, creationDesc);
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
			return CreateCubeMap(device, creationDesc); 
		case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
			return CreateLightLinkedList(device, creationDesc);
		default:
			return CommonCreationProcess(device, creationDesc, rType);
		}
	}
	bool JDx12GraphicResourceManager::CreateOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo> info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
	{
		if (info == nullptr || !IsSameDevice(info.Get()))
			return false;

		const J_GRAPHIC_RESOURCE_TYPE rType = info->GetGraphicResourceType();
		if (!CanCreateOptionResource(opType, rType))
			return false;

		JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info.Get());
		if (dxInfo->HasOption(opType))
			return false;
		 
		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		CreateOption(opType, dxInfo, CreationClass::Create(data, dxInfo->resourceHolder->GetResource(), opType));
		if (JDx12TypeAttribute::GraphicOptionNeedRebind(rType, opType))
		{
			ClearDescViewCountUntil(dxInfo);
			ReBind(data.device, rType, dxInfo->GetArrayIndex());
		}
		device->EndPublicCommandSet(data.startCommandThisCreation);
		return true;
	}
	bool JDx12GraphicResourceManager::DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info)
	{
		if (info == nullptr || !IsSameDevice(device))
			return false;
		 
		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info);
		const uint rIndex = dxInfo->GetArrayIndex();
		const J_GRAPHIC_RESOURCE_TYPE rType = dxInfo->GetGraphicResourceType();
		JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
		const bool canBind = CreationClass::CanBind(rType);

		//ptr can nullptr ex) occlusion query(resource view binding을 사용하지 않는 타입)
		if (canBind)
		{
			ClearDescViewCountUntil(dxInfo);
			ReBind(data.device, rType, rIndex + 1, -1);
		}
		else
		{
			for (uint i = rIndex + 1; i < desc.count; ++i)
			{
				//reset array
				const int newIndex = i - 1;
				resource[(int)rType][i]->SetArrayIndex(newIndex);
			}
		}

		--desc.count;
		//resource[(int)rType].begin() + rIndex 
		//JOwnerPtr<JGraphicResourceInfo> target = std::move(resource[(int)rType][rIndex]);
		resource[(int)rType].erase(resource[(int)rType].begin() + rIndex);
		//target.Clear();
		device->EndPublicCommandSet(data.startCommandThisCreation);

		return true;
	}
	bool JDx12GraphicResourceManager::DestroyGraphicOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)
	{
		if (info == nullptr || !IsSameDevice(device))
			return false;

		if (!info->HasOption(optype))
			return false;
		 
		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);
		JDx12GraphicResourceInfo* dxInfo = static_cast<JDx12GraphicResourceInfo*>(info.Get());
		const J_GRAPHIC_RESOURCE_TYPE rType = dxInfo->GetGraphicResourceType();

		dxInfo->optionHolderSet->holder[(uint)optype] = nullptr;
		bool isEmpty = true;
		for (const auto& data : dxInfo->optionHolderSet->holder)
			isEmpty &= data == nullptr;
		if (isEmpty)
			dxInfo->optionHolderSet = nullptr;

		ClearDescViewCountUntil(dxInfo);
		ReBind(data.device, rType, dxInfo->GetArrayIndex());
		device->EndPublicCommandSet(data.startCommandThisCreation);
		return true;
	}
	JUserPtr<JDx12GraphicResourceInfo> JDx12GraphicResourceManager::CommonCreationProcess(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType, const ExtraOption& extraOption)
	{
		if (!CanCreateResource(rType, device))
		{
			J_LOG_PRINT_OUT("Fail create graphic resouce", Core::GetName(rType));
			return nullptr;
		}
		 
		JUserPtr<JDx12GraphicResourceInfo> userPtr = nullptr;
		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
		JDx12GraphicResourceHolderDesc result = CreationClass::Create(data, creationDesc, rType);
		if (result.IsValid())
		{
			if (JDx12TypeAttribute::IsBuffer(rType) && creationDesc.uploadBufferDesc != nullptr)
			{
				bool canUpload = true;
				if (creationDesc.uploadBufferDesc->useEngineDefine)
					canUpload = JDx12TypeAttribute::TrySetInitBufferPointer(rType, creationDesc.uploadBufferDesc.get());

				canUpload &= creationDesc.uploadBufferDesc->data != nullptr;
				if (canUpload)
				{
					CreationClass::CreateUploadBuffer(data.device, result.resource.Get(), uploadBuffer.GetAddressOf());
					CreationClass::UploadData(data.commandList,
						result.resource.Get(),
						uploadBuffer.Get(),
						D3D12_RESOURCE_STATE_COMMON,
						D3D12_RESOURCE_STATE_COMMON,
						creationDesc.uploadBufferDesc->data,
						creationDesc.uploadBufferDesc->bufferSize,
						1);
				}
			}
			userPtr = CreateResourceInfo(rType, std::move(result));
			if (CreationClass::CanBind(rType) && !extraOption.bindResourceManually)
			{
				auto getNextViewIndexLam = GetNextViewIndexLam;
				auto addViewIndexLam = AddViewIndexLam;
				JBindDesc bindDesc(this, data.device, getNextViewIndexLam, addViewIndexLam, userPtr.Get(), creationDesc);
				CreationClass::Bind(bindDesc);
			}
		}
		device->EndPublicCommandSet(data.startCommandThisCreation);
		if (uploadBuffer != nullptr && !data.startCommandThisCreation)
		{
			data.dxDevice->ReStartPublicCommandSet();
		}
		return userPtr;
	}
	JUserPtr<JDx12GraphicResourceInfo> JDx12GraphicResourceManager::CreateTexture2D(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc)
	{
		static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
		if (!CanCreateResource(rType, device))
			return nullptr;

		JUserPtr<JDx12GraphicResourceInfo> userPtr = nullptr;
		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
		std::unique_ptr<DirectX::ResourceUploadBatch> uploadBatch = nullptr;
		bool res = false;

		if (creationDesc.textureDesc->creationType == JTextureCreationDesc::CREATION_TYPE::LOAD)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> newResource = nullptr;
			res = CreationClass::Load(data, creationDesc, rType, newResource, uploadBuffer, uploadBatch);
			if(res)
			{
				userPtr = CreateResourceInfo(rType, JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
				auto getNextViewIndexLam = GetNextViewIndexLam;
				auto addViewIndexLam = AddViewIndexLam;
				JBindDesc bindDesc(this, data.device, getNextViewIndexLam, addViewIndexLam, userPtr.Get(), creationDesc);
				CreationClass::Bind(bindDesc);
			}
		}
		else
		{ 
			JDx12GraphicResourceHolderDesc holder = CreationClass::Create(data, creationDesc, rType);
			auto desc = holder.resource->GetDesc();
			const size_t resourceSize = holder.elementSize * desc.Width * desc.Height;

			if (creationDesc.uploadBufferDesc != nullptr && resourceSize == creationDesc.uploadBufferDesc->bufferSize)
			{
				CreationClass::CreateUploadBuffer(data.device, holder.resource.Get(), uploadBuffer.GetAddressOf());
				CreationClass::UploadData(data.commandList,
					holder.resource.Get(),
					uploadBuffer.Get(),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					creationDesc.uploadBufferDesc->data,
					desc.Width,
					desc.Height,
					holder.elementSize,
					1);
			}
			res = true;
		}
		if (!res)
			J_LOG_PRINT_OUT("Create texture2D fail", "");
		device->EndPublicCommandSet(data.startCommandThisCreation);
		if (res && !data.startCommandThisCreation)
		{
			//함수 호출전부터 publicCommand가 작동중일시
			//uploadBuffer에 삭제를 위해 여기서 command를 제출하고 다시 작동시킨다.
			data.dxDevice->ReStartPublicCommandSet();
			//data.dxDevice->EndPublicCommand();
			//data.dxDevice->FlushCommandQueue();
			//data.dxDevice->StartPublicCommand();
		}
		uploadBuffer.Reset();
		uploadBatch = nullptr;
		return userPtr;
	}
	JUserPtr<JDx12GraphicResourceInfo> JDx12GraphicResourceManager::CreateCubeMap(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc)
	{
		static constexpr J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
		if (!CanCreateResource(rType, device))
			return nullptr;

		JUserPtr<JDx12GraphicResourceInfo> userPtr = nullptr;
		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
		std::unique_ptr<DirectX::ResourceUploadBatch> uploadBatch;

		uint heapIndex = GetHeapIndex(rType, J_GRAPHIC_BIND_TYPE::SRV); 
		bool res = CreationClass::Load(data, creationDesc, rType, newResource, uploadBuffer, uploadBatch, false);
 
		if (res)
		{
			userPtr = CreateResourceInfo(rType, JDx12GraphicResourceHolderDesc(std::move(newResource), D3D12_RESOURCE_STATE_COMMON));
			auto getNextViewIndexLam = GetNextViewIndexLam;
			auto addViewIndexLam = AddViewIndexLam;
			JBindDesc bindDesc(this, data.device, getNextViewIndexLam, addViewIndexLam, userPtr.Get(), creationDesc);
			CreationClass::Bind(bindDesc);
		}
		device->EndPublicCommandSet(data.startCommandThisCreation);
		if (res && !data.startCommandThisCreation)
		{
			//함수 호출전부터 publicCommand가 작동중일시
			//uploadBuffer에 삭제를 위해 여기서 command를 제출하고 다시 작동시킨다.
			data.dxDevice->ReStartPublicCommandSet();
			//data.dxDevice->EndPublicCommand();
			//data.dxDevice->FlushCommandQueue();
			//data.dxDevice->StartPublicCommand();
		}
		uploadBuffer.Reset();
		return userPtr;
	}
	JUserPtr<JDx12GraphicResourceInfo> JDx12GraphicResourceManager::CreateLightLinkedList(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc)
	{
		ExtraOption extraOption;
		extraOption.bindResourceManually = true;

		auto user = CommonCreationProcess(device, creationDesc, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, extraOption);
		if (user != nullptr)
		{
			if (CreateOption(device, user, J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER))
			{
				JDeviceData data(device, GetGraphicOption());
				device->StartPublicCommandSet(data.startCommandThisCreation);

				auto getNextViewIndexLam = GetNextViewIndexLam;
				auto addViewIndexLam = AddViewIndexLam;
				JBindDesc bindDesc(this, data.device, getNextViewIndexLam, addViewIndexLam, user.Get(), creationDesc);
				CreationClass::Bind(bindDesc);

				device->EndPublicCommandSet(data.startCommandThisCreation);
			}
			else
				DestroyGraphicTextureResource(device, user.Release());
		}
		return user;
	}
	JUserPtr<JDx12GraphicResourceInfo> JDx12GraphicResourceManager::CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE rType, JDx12GraphicResourceHolderDesc&& result)
	{
		JDx12GraphicResourceInfo::GetCpuHandlePtr getCpuHandleLam = [](JDx12GraphicResourceManager* gm, const J_GRAPHIC_BIND_TYPE btype, const uint index)
		{
			return gm->GetCpuDescriptorHandle(btype, index);
		};
		JDx12GraphicResourceInfo::GetGpuHandlePtr getGpuHandleLam = [](JDx12GraphicResourceManager* gm, const J_GRAPHIC_BIND_TYPE btype, const uint index)
		{
			return gm->GetGpuDescriptorHandle(btype, index);
		};

		auto dx12Holder = std::make_unique<JDx12GraphicResourceHolder>(std::move(result));
		auto newInfo = JOwnerPtr<JDx12GraphicResourceInfo>(new JDx12GraphicResourceInfo(rType, this, std::move(dx12Holder), getGpuHandleLam));

		newInfo->SetArrayIndex(resource[(int)rType].size());
		newInfo->SetPrivateName();
		resource[(int)rType].push_back(std::move(newInfo));
		++typeDesc[(int)rType].count;

		JUserPtr<JDx12GraphicResourceInfo> user = resource[(int)rType][resource[(int)rType].size() - 1];
		return user;
	}
	bool JDx12GraphicResourceManager::CreateOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, JDx12GraphicResourceInfo* dxInfo, JDx12GraphicResourceHolderDesc&& result)
	{
		if (dxInfo == nullptr || !result.IsValid())
			return false;

		dxInfo->SetOption(opType, std::make_unique<JDx12GraphicResourceHolder>(std::move(result)));
		dxInfo->SetPrivateOptionName(opType, Core::GetWName(opType));
		return true;
	}
	void JDx12GraphicResourceManager::ClearDescViewCountUntil(JDx12GraphicResourceInfo* dxInfo)
	{
		if (dxInfo == nullptr)
			return;

		const J_GRAPHIC_RESOURCE_TYPE rType = dxInfo->GetGraphicResourceType();
		JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
		const uint arrayIndex = dxInfo->GetArrayIndex(); 
	 
		for (uint i = 0; i < (int)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
		{
			J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)i;
			if (desc.viewInfo[i].isFixedBind)
				desc.viewInfo[i].count = dxInfo->GetHeapIndexStart((J_GRAPHIC_BIND_TYPE)i) - desc.viewInfo[i].offset;
			else if (desc.viewInfo[i].count > 0)
			{
				//option혹은 선택적 사용중인 view에 대한 제어
				//typeDesc에는 Resource info에 fixed bind type뿐만 아니라 optional bind type, 그리고 option에 bind type에 대한
				//count도 포함한다.
				for (uint j = arrayIndex; j < desc.count; ++j)
				{
					int validHeapSt = resource[(uint)rType][j]->GetMinValidHeapIndexStart(bType);
					if (validHeapSt != invalidIndex)
					{
						desc.viewInfo[i].count = validHeapSt - desc.viewInfo[i].offset;
						break;
					}
				}
			} 
			//heapIndexStart는 바인딩되지 않을 경우 기본값인 -1을 가지고있는 상태이다.  
			if (desc.viewInfo[i].count < 0)
				desc.viewInfo[i].count = 0;
		}
		 
	}
	void JDx12GraphicResourceManager::ReBind(ID3D12Device* device, const J_GRAPHIC_RESOURCE_TYPE rType, const uint index, const int additionalArrayIndex)
	{
		if (!CreationClass::CanBind(rType))
			return;

		JDx12GraphicResourceManager::ResourceTypeDesc& desc = typeDesc[(int)rType];
		const bool fixedBind = JDx12TypeAttribute::IsFixedBindType(rType);
		//type per resource view controll
		for (uint i = index; i < desc.count; ++i)
		{ 
			//init heap st, bind value
			auto info = GetDxInfo(rType, i);
			auto getNextViewIndexLam = GetNextViewIndexLam;
			auto addViewIndexLam = AddViewIndexLam;
			JBindDesc bindDesc(this, device, getNextViewIndexLam, addViewIndexLam, info);

			for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
			{
				const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)j;
				if (!fixedBind && info->HasView(bType))
					bindDesc.cDesc.requestAdditionalBind[j] = true;
				else
					bindDesc.cDesc.requestAdditionalBind[j] = false;

				info->SetHeapIndexStart(bType, invalidIndex);
				info->SetViewCount(bType, 0);
				for (uint k = 0; k < (uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT; ++k)
				{
					const J_GRAPHIC_RESOURCE_OPTION_TYPE opType = (J_GRAPHIC_RESOURCE_OPTION_TYPE)k;
					info->SetHeapOptionIndexStart(bType, opType, invalidIndex);
					info->SetOptionViewCount(bType, opType, 0);
				}
			}

			if (!fixedBind)
				bindDesc.cDesc.useEngineDefinedBindType = false;
			CreationClass::Bind(bindDesc);

			//reset array, heap st, bind value
			if (additionalArrayIndex != 0)
			{
				const int newIndex = i + additionalArrayIndex;
				info->SetArrayIndex(newIndex);
			}
		}
	}
	bool JDx12GraphicResourceManager::ReAllocTypePerAllResource(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType)
	{
		if (!IsSameDevice(device) || !JDx12TypeAttribute::CanReAlloc(rType))
			return false;

		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		auto& vec = resource[(uint)rType];
		for (const auto& gInfo : vec)
		{
			auto existDesc = gInfo->resourceHolder->GetResource()->GetDesc();
			JGraphicResourceCreationDesc creationDesc;
			creationDesc.width = existDesc.Width;
			creationDesc.height = existDesc.Height;
			creationDesc.arraySize = existDesc.DepthOrArraySize;
			if (!JDx12TypeAttribute::IsFixedBindType(rType))
			{
				creationDesc.bindDesc.useEngineDefinedBindType = false;
				for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				{
					const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)i;
					if (gInfo->HasView(bType))
						creationDesc.bindDesc.requestAdditionalBind[i] = true;
					else
						creationDesc.bindDesc.requestAdditionalBind[i] = false;
				}
			}

			auto dx12Holder = std::make_unique<JDx12GraphicResourceHolder>(CreationClass::Create(data, creationDesc, rType));
			gInfo->resourceHolder = std::move(dx12Holder);
			gInfo->SetPrivateName();
		}

		ReBind(data.device, rType, 0);
		device->EndPublicCommandSet(data.startCommandThisCreation);
		return true;
	}
	bool JDx12GraphicResourceManager::ReAllocTypePerAllResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
	{
		if (!IsSameDevice(device) || !JDx12TypeAttribute::CanReAlloc(rType))
			return false;

		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		auto& vec = resource[(uint)rType];
		for (const auto& gInfo : vec)
		{
			auto dx12Holder = std::make_unique<JDx12GraphicResourceHolder>(CreationClass::Create(data, creationDesc, rType));
			gInfo->resourceHolder = std::move(dx12Holder);
			gInfo->SetPrivateName();
		}

		ReBind(data.device, rType, 0);
		device->EndPublicCommandSet(data.startCommandThisCreation);
		return true;
	}
	bool JDx12GraphicResourceManager::CopyResource(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& from, const JUserPtr<JGraphicResourceInfo>& to)
	{
		if (!IsSameDevice(device) || from == nullptr || to == nullptr)
			return false;

		JDeviceData data(device, GetGraphicOption());
		device->StartPublicCommandSet(data.startCommandThisCreation);

		auto fromHolder = GetDxHolder(from->GetGraphicResourceType(), from->GetArrayIndex());
		auto toHolder = GetDxHolder(to->GetGraphicResourceType(), to->GetArrayIndex());
		auto fromR = fromHolder->GetResource();
		auto toR = toHolder->GetResource();
		auto fromBefore = fromHolder->GetState();
		auto toBefore = toHolder->GetState();

		JDx12Utility::ResourceTransition(data.commandList, fromR, fromBefore, D3D12_RESOURCE_STATE_COPY_SOURCE);
		JDx12Utility::ResourceTransition(data.commandList, toR, toBefore, D3D12_RESOURCE_STATE_COPY_DEST);
		data.commandList->CopyResource(toR, fromR);
		JDx12Utility::ResourceTransition(data.commandList, toR, D3D12_RESOURCE_STATE_COPY_DEST, toBefore);
		JDx12Utility::ResourceTransition(data.commandList, fromR, D3D12_RESOURCE_STATE_COPY_SOURCE, fromBefore);

		device->EndPublicCommandSet(data.startCommandThisCreation);
		return true;
	}
	bool JDx12GraphicResourceManager::SettingMipmapBind(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& info, const bool isReadOnly, _Out_ std::vector<Core::JDataHandle>& handle)
	{
		handle.clear();
		if (!IsSameDevice(device) || info == nullptr || !JDx12TypeAttribute::CanCreateMipmap(info->GetGraphicResourceType()) || !mpb.CanAdd())
			return false;

		auto resource = static_cast<JDx12GraphicResourceInfo*>(info.Get())->resourceHolder->GetResource();
		auto desc = resource->GetDesc();
		if (mpb.GetEmptyCount() < desc.MipLevels || desc.MipLevels < 2)
			return false;

		JDeviceData data(device, GetGraphicOption());
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = desc.Format;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		if (info->GetGraphicResourceType() == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		}

		const uint mipLevelCount = desc.MipLevels;
		for (uint i = 0; i < mipLevelCount; ++i)
		{
			srvDesc.Texture2D.MostDetailedMip = i;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.ResourceMinLODClamp = i;
			uavDesc.Texture2D.MipSlice = i;

			auto mpbInfo = JPtrUtil::MakeOwnerPtr<MPBInfo>();
			mpbInfo->info = info;
			mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::SRV] = GetMPBOffset(J_GRAPHIC_BIND_TYPE::SRV) + mpb.GetValidIndex();
			data.device->CreateShaderResourceView(resource, &srvDesc, GetCpuSrvDescriptorHandle(mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::SRV]));

			if (!isReadOnly)
			{
				mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::UAV] = GetMPBOffset(J_GRAPHIC_BIND_TYPE::UAV) + mpb.GetValidIndex();
				data.device->CreateUnorderedAccessView(resource, nullptr, &uavDesc, GetCpuSrvDescriptorHandle(mpbInfo->index[(uint)J_GRAPHIC_BIND_TYPE::UAV]));
			}
			handle.push_back(mpb.Add(std::move(mpbInfo)));
		}
		return true;
	}
	void JDx12GraphicResourceManager::DestroyMPB(JGraphicDevice* device, Core::JDataHandle& handle)
	{
		if (device == nullptr || !IsSameDevice(device))
			return;

		mpb.Remove(handle);
	}
	uint JDx12GraphicResourceManager::GetMPBOffset(const J_GRAPHIC_BIND_TYPE bType)const noexcept
	{
		switch (bType)
		{
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
			return 0;
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
			return 0;
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
		{
			return GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::SRV) + GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::UAV) +
				JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) + JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV);
		}
		case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
		{
			return GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::SRV) + GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::UAV) +
				JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) + JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) +
				JDx12TypeAttribute::MPBCount(J_GRAPHIC_BIND_TYPE::SRV);
		}
		default:
			return 0;
		}
	}
	void JDx12GraphicResourceManager::BuildResource(JGraphicDevice* device)
	{
		ID3D12Device* d3dDevice = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		BuildTypeDesc(d3dDevice);
		BuildRtvDescriptorHeaps(d3dDevice);
		BuildDsvDescriptorHeaps(d3dDevice);
		BuildSrvDescriptorHeaps(d3dDevice);
	}
	void JDx12GraphicResourceManager::BuildTypeDesc(ID3D12Device* device)
	{
		rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		cbvSrvUavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			typeDesc[i].ClearAllData();

		int totalView[(uint)J_GRAPHIC_BIND_TYPE::COUNT];
		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
			typeDesc[i].capacity = JDx12TypeAttribute::InitCapacity(rType);

			for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
			{
				if (j == (uint)J_GRAPHIC_BIND_TYPE::UAV)
					continue;

				const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)j;
				typeDesc[i].viewInfo[j].capacity = JDx12TypeAttribute::InitTotalViewCapaicty(rType, bType);
				if (i > 0)
				{
					typeDesc[i].viewInfo[j].offset = totalView[j];
					totalView[j] += typeDesc[i].viewInfo[j].capacity;
				}
				else
				{
					typeDesc[i].viewInfo[j].offset = JDx12TypeAttribute::InitViewFixedCount(bType);
					totalView[j] = typeDesc[i].viewInfo[j].offset + typeDesc[i].viewInfo[j].capacity;
				}
			}
		}

		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
			const uint uavIndex = (uint)J_GRAPHIC_BIND_TYPE::UAV;
			typeDesc[i].viewInfo[uavIndex].capacity = JDx12TypeAttribute::InitTotalViewCapaicty(rType, J_GRAPHIC_BIND_TYPE::UAV);
			if (i > 0)
			{
				typeDesc[i].viewInfo[uavIndex].offset = totalView[uavIndex];
				totalView[uavIndex] += typeDesc[i].viewInfo[uavIndex].capacity;
			}
			else
			{
				const uint srvIndex = (uint)J_GRAPHIC_BIND_TYPE::SRV;
				const uint lastResourceIndex = (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT - 1;

				typeDesc[i].viewInfo[uavIndex].offset = JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) +
					typeDesc[lastResourceIndex].viewInfo[srvIndex].offset +
					typeDesc[lastResourceIndex].viewInfo[srvIndex].capacity;
				totalView[uavIndex] = typeDesc[i].viewInfo[uavIndex].offset + typeDesc[i].viewInfo[uavIndex].capacity;
			}
		}

		//빠른 계산을 위한 fixed data 전처리
		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		{
			const J_GRAPHIC_RESOURCE_TYPE rType = (J_GRAPHIC_RESOURCE_TYPE)i;
			const bool isFixed = JDx12TypeAttribute::IsFixedBindType(rType);

			for (uint j = 0; j < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++j)
			{
				const J_GRAPHIC_BIND_TYPE bType = (J_GRAPHIC_BIND_TYPE)j;
				const uint initCapacity = JDx12TypeAttribute::InitViewCapacity(rType, bType);
			 
				if (initCapacity > 0)
				{
					typeDesc[(uint)rType].viewInfo[j].canBind = true;
					typeDesc[(uint)rType].viewInfo[j].isFixedBind = JDx12TypeAttribute::IsFixedBindType(rType, bType);
				}
			}
		}
	}
	void JDx12GraphicResourceManager::BuildRtvDescriptorHeaps(ID3D12Device* device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::RTV) +
			JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::RTV) +
			JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::RTV) +
			JDx12TypeAttribute::MPBCount(J_GRAPHIC_BIND_TYPE::RTV);
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailedHr(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));
		rtvHeap->SetName(L"RtvHeap");
	}
	void JDx12GraphicResourceManager::BuildDsvDescriptorHeaps(ID3D12Device* device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::DSV) +
			JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::DSV) +
			JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::DSV) +
			JDx12TypeAttribute::MPBCount(J_GRAPHIC_BIND_TYPE::DSV);
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailedHr(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf())));
		dsvHeap->SetName(L"DsvHeap");
	}
	void JDx12GraphicResourceManager::BuildSrvDescriptorHeaps(ID3D12Device* device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::SRV) +
			GetTotalViewCapacity(J_GRAPHIC_BIND_TYPE::UAV) +
			JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::SRV) +
			JDx12TypeAttribute::InitViewFixedCount(J_GRAPHIC_BIND_TYPE::UAV) +
			JDx12TypeAttribute::MPBCount(J_GRAPHIC_BIND_TYPE::SRV) +
			JDx12TypeAttribute::MPBCount(J_GRAPHIC_BIND_TYPE::UAV);

		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailedHr(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(srvHeap.GetAddressOf())));
		srvHeap->SetName(L"SrvHeap");
	}
	void JDx12GraphicResourceManager::ClearResource()
	{
		mpb.Clear();
		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		{
			resource[i].clear();
			typeDesc[i].ClearAllData();
		}

		rtvHeap.Reset();
		dsvHeap.Reset();
		srvHeap.Reset();
		defaultSceneDsInfo = nullptr;
	}
	void JDx12GraphicResourceManager::ResizeWindow(JGraphicDevice* device)
	{
		if (!IsSameDevice(device))
			return;

		if (defaultSceneDsInfo != nullptr)
			DestroyGraphicTextureResource(device, defaultSceneDsInfo.Release());

		auto graphicInfo = GetGraphicInfo();
		JGraphicResourceCreationDesc desc(graphicInfo.width, graphicInfo.height);
		defaultSceneDsInfo = CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL);
		defaultSceneDsInfo->SetPrivateName(L"EndFrameDs");
	}
	void JDx12GraphicResourceManager::StoreTexture(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType, const int index, const std::wstring& path)
	{
		if (!IsSameDevice(device))
			return;

		auto holder = GetDxHolder(rType, index);
		if (holder == nullptr)
			return;

		if (holder->GetResource()->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			device->FlushCommandQueue();
			device->StartPublicCommand();

			JDx12GraphicDevice* dxDevice = static_cast<JDx12GraphicDevice*>(device);
			DirectX::SaveDDSTextureToFile(dxDevice->GetCommandQueue(),
				holder->GetResource(),
				path.c_str(),
				holder->GetState(),
				holder->GetState());

			device->EndPublicCommand();
			device->FlushCommandQueue();
		}
	}
	void JDx12GraphicResourceManager::RegisterTypeData()
	{
		using JAllocationDesc = JinEngine::Core::JAllocationDesc;
		using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;
		using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;
		using ReceiverPtr = JAllocationDesc::ReceiverPtr;
		using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;
		using MemIndex = JAllocationDesc::MemIndex;

		NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
		{
			JDx12GraphicResourceInfo* movedInfo = static_cast<JDx12GraphicResourceInfo*>(movedPtr);
			JDx12GraphicResourceManager* manager = movedInfo->manager;

			//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
			//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
			//Release() 한다음 Reset()을 호출해야한다.
			manager->resource[(int)movedInfo->GetGraphicResourceType()][movedInfo->GetArrayIndex()].Release();
			manager->resource[(int)movedInfo->GetGraphicResourceType()][movedInfo->GetArrayIndex()].Reset(movedInfo);
		};
		auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
		std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

		desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
		JDx12GraphicResourceInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
	}

	JDx12GraphicResourceComputeSet::JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, JGraphicResourceInfo* gInfo)
		:gm(gm),
		info(static_cast<JDx12GraphicResourceInfo*>(gInfo)),
		holder(info != nullptr ? gm->GetDxHolder(info->GetGraphicResourceType(), info->GetArrayIndex()) : nullptr),
		resource(holder != nullptr ? holder->GetResource() : nullptr)
	{
	}
	JDx12GraphicResourceComputeSet::JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JUserPtr<JGraphicResourceInfo>& gInfo)
		: gm(gm),
		info(static_cast<JDx12GraphicResourceInfo*>(gInfo.Get())),
		holder(info != nullptr ? gm->GetDxHolder(info->GetGraphicResourceType(), info->GetArrayIndex()) : nullptr),
		resource(holder != nullptr ? holder->GetResource() : nullptr)
	{
	}
	JDx12GraphicResourceComputeSet::JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)
		: gm(gm),
		info(gm->GetDxInfo(rType, gInterface.GetResourceArrayIndex(rType, gInterface.GetResourceDataIndex(rType, taskType)))),
		holder(info != nullptr ? gm->GetDxHolder(info->GetGraphicResourceType(), info->GetArrayIndex()) : nullptr),
		resource(holder != nullptr ? holder->GetResource() : nullptr)
	{
	}
	JDx12GraphicResourceComputeSet::JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JGraphicResourceUserInterface& gInterface, const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)
		: gm(gm),
		info(gm->GetDxInfo(rType, gInterface.GetResourceArrayIndex(rType, dataIndex))),
		holder(info != nullptr ? gm->GetDxHolder(info->GetGraphicResourceType(), info->GetArrayIndex()) : nullptr),
		resource(holder != nullptr ? holder->GetResource() : nullptr)
	{
	}
	JDx12GraphicResourceComputeSet::JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, JGraphicResourceInfo* gInfo, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		: gm(gm),
		info(static_cast<JDx12GraphicResourceInfo*>(gInfo)),
		holder(info != nullptr ? gm->GetOptionHolder(info->GetGraphicResourceType(), opType, info->GetArrayIndex()) : nullptr),
		resource(holder != nullptr ? holder->GetResource() : nullptr),
		opType(opType)
	{
	}
	JDx12GraphicResourceComputeSet::JDx12GraphicResourceComputeSet(JDx12GraphicResourceManager* gm, const JUserPtr<JGraphicResourceInfo>& gInfo, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		: gm(gm),
		info(static_cast<JDx12GraphicResourceInfo*>(gInfo.Get())),
		holder(info != nullptr ? gm->GetOptionHolder(info->GetGraphicResourceType(), opType, info->GetArrayIndex()) : nullptr),
		resource(holder != nullptr ? holder->GetResource() : nullptr),
		opType(opType)
	{
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetCpuDsvHandle()const noexcept
	{
		return opType == invalidType ? gm->GetCpuDsvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV) + viewOffset) :
			gm->GetCpuDsvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV, opType) + viewOffset);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetCpuRtvHandle()const noexcept
	{
		return opType == invalidType ? gm->GetCpuRtvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV) + viewOffset) :
			gm->GetCpuRtvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV, opType) + viewOffset);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetCpuSrvHandle()const noexcept
	{
		return opType == invalidType ? gm->GetCpuSrvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + viewOffset) :
			gm->GetCpuSrvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV, opType) + viewOffset);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetCpuUavHandle()const noexcept
	{
		return opType == invalidType ? gm->GetCpuSrvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV) + viewOffset) :
			gm->GetCpuSrvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV, opType) + viewOffset);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetGpuDsvHandle()const noexcept
	{
		return opType == invalidType ? gm->GetGpuDsvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV) + viewOffset) :
			gm->GetGpuDsvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV, opType) + viewOffset);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetGpuRtvHandle()const noexcept
	{
		return opType == invalidType ? gm->GetGpuRtvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV) + viewOffset) :
			gm->GetGpuRtvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV, opType) + viewOffset);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetGpuSrvHandle()const noexcept
	{
		return opType == invalidType ? gm->GetGpuSrvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + viewOffset) :
			gm->GetGpuSrvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV, opType) + viewOffset);
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE JDx12GraphicResourceComputeSet::GetGpuUavHandle()const noexcept
	{
		return opType == invalidType ? gm->GetGpuSrvDescriptorHandle(info->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV) + viewOffset) :
			gm->GetGpuSrvDescriptorHandle(info->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV, opType) + viewOffset);
	}
	bool JDx12GraphicResourceComputeSet::IsValid()const noexcept
	{
		return info != nullptr;
	}
}

/*
//Debug
 /*
 #include<fstream>
#include"../../Core/File/JFileIOHelper.h"

		std::wofstream stream;
		stream.open(L"D:\\JinWooJung\\gDebug.txt", std::ios::app | std::ios::out);

		Core::JEnumInfo* rInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
		Core::JEnumInfo* bInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
		auto rNameVec = rInfo->GetEnumNameVec();
		auto bNameVec = bInfo->GetEnumNameVec();
		JFileIOHelper::InputSpace(stream, 1);
 */