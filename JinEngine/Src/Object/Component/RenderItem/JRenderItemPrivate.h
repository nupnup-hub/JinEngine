#pragma once
#include"../JComponentPrivate.h"

namespace JinEngine
{
	class JRenderItem; 
	namespace Graphic
	{
		class JGraphic; 
		struct JObjectConstants;
		struct JBoundingObjectConstants;
	}
	class JRenderItemPrivate final : public JComponentPrivate
	{
	public:
		class AssetDataIOInterface final : public JComponentPrivate::AssetDataIOInterface
		{
		private:
			Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		};
		class CreateInstanceInterface final : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		private:
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		private:
			bool Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept;
		};
		class FrameUpdateInterface final
		{
		private:
			friend class Graphic::JGraphic;
		private:   
			static bool UpdateStart(JRenderItem* rItem, const bool isUpdateForced)noexcept;
			static void UpdateFrame(JRenderItem* rItem, Graphic::JObjectConstants& constant, const uint submeshIndex)noexcept;
			static void UpdateFrame(JRenderItem* rItem, Graphic::JBoundingObjectConstants& constant)noexcept;
			static void UpdateEnd(JRenderItem* rItem)noexcept;
			static bool IsHotUpdated(JRenderItem* rItem)noexcept;
		private:
			static uint GetBoundingCBOffset(JRenderItem* rItem)noexcept;		//Count ritem
			static uint GetObjectCBBuffOffset(JRenderItem* rItem)noexcept;		//Count submesh
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}