#include"JGraphicUpdateHelper.h"

namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicUpdateHelper::BindingTextureData::HasCallable()const noexcept
		{
			return getTextureCountCallable && getTextureCapacityCallable && setCapacityCallable;
		}
		void JGraphicUpdateHelper::Clear()
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
		void JGraphicUpdateHelper::RegisterCallable(J_UPLOAD_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr)
		{
			if (getCountPtr == nullptr)
				return;

			uData[(int)type].getElementCountCallable = std::make_unique<GetElementCountT::Callable>(*getCountPtr);
			//uData[(int)type].getElementCapacityCallable = std::make_unique<GetElementCapacityT::Callable>(*getCapaPtr);
			//uData[(int)type].rebuildCallable = std::make_unique<RebuildT::Callable>(*rPtr);
		}
		void JGraphicUpdateHelper::RegisterCallable(J_GRAPHIC_TEXTURE_TYPE type, GetTextureCountT::Ptr* getCountPtr, GetTextureCapacityT::Ptr* getCapaPtr, SetCapacityT::Ptr* sPtr)
		{
			if (getCountPtr == nullptr || getCapaPtr == nullptr || sPtr == nullptr)
				return;

			bData[(int)type].getTextureCountCallable = std::make_unique<GetTextureCountT::Callable>(*getCountPtr);
			bData[(int)type].getTextureCapacityCallable = std::make_unique< GetTextureCapacityT::Callable>(*getCapaPtr);
			bData[(int)type].setCapacityCallable = std::make_unique<SetCapacityT::Callable>(*sPtr);
		}
		void JGraphicUpdateHelper::RegisterListener(J_UPLOAD_RESOURCE_TYPE type, std::unique_ptr<NotifyUpdateCapacityT::Callable>&& listner)
		{
			uData[(int)type].notifyUpdateCapacityCallable.push_back(std::move(listner));
		}
		void JGraphicUpdateHelper::WriteGraphicInfo(JGraphicInfo& info)const noexcept
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

			info.binding2DTextureCount = bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D].count;
			info.bindingCubeMapCount = bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE].count;
			info.bindingShadowTextureCount = bData[(int)J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP].count;

			info.binding2DTextureCapacity = bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D].capacity;
			info.bindingCubeMapCapacity = bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE].capacity;
			info.bindingShadowTextureCapacity = bData[(int)J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP].capacity;
		}
		void JGraphicUpdateHelper::NotifyUpdateFrameCapacity(JGraphicImpl& grpahic)
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
	}
}