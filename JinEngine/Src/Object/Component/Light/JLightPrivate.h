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
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		};
		class CreateInstanceInterface final : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final; 
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		private:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept;
		};
		class DestroyInstanceInterface final : public JComponentPrivate::DestroyInstanceInterface
		{
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced)final;
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
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}