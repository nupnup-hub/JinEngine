#pragma once
#include"../JComponentPrivate.h"

namespace JinEngine
{ 
	class JLight; 
	namespace Graphic
	{
		class JGraphic;
		struct JLightConstants;
		struct JShadowMapLightConstants;
		struct JShadowMapConstants;
	}
	class JLightPrivate final : public JComponentPrivate
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
			static bool UpdateStart(JLight* lit, const bool isUpdateForced)noexcept;
			static void UpdateFrame(JLight* lit, Graphic::JLightConstants& constant)noexcept;
			static void UpdateFrame(JLight* lit, Graphic::JShadowMapLightConstants& constant)noexcept;
			static void UpdateFrame(JLight* lit, Graphic::JShadowMapConstants& constant)noexcept;
			static void UpdateEnd(JLight* lit)noexcept;
			static bool IsHotUpdated(JLight* lit)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}