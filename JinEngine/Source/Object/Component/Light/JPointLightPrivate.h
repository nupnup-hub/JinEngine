#pragma once
#include"JLightPrivate.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JPointLightConstants;
		struct JShadowMapCubeDrawConstants; 
	}
	class JPointLight;
	class JPointLightPrivate : public JLightPrivate
	{
	public:
		class AssetDataIOInterface final : public JLightPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		};
		class CreateInstanceInterface final : public JLightPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		private:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept final;
		};
		class DestroyInstanceInterface final : public JLightPrivate::DestroyInstanceInterface
		{
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced)final;
		};
		class FrameUpdateInterface final : public JLightPrivate::FrameUpdateInterface
		{
		private:
			friend class Graphic::JGraphic; 
		private:
			bool UpdateStart(JLight* lit, const bool isUpdateForced)noexcept final;
			void UpdateFrame(JPointLight* lit, Graphic::JPointLightConstants& constant)noexcept;
			void UpdateFrame(JPointLight* lit, Graphic::JShadowMapCubeDrawConstants& constant)noexcept; 
			void UpdateEnd(JLight* lit)noexcept final;
		private:
			int GetLitFrameIndex(JLight* lit)noexcept final;
			int GetShadowMapFrameIndex(JLight* lit)noexcept final;  
			int GetDepthTestPassFrameIndex(JLight* lit)noexcept final;
			int GetHzbOccComputeFrameIndex(JLight* lit)noexcept final;
		private:
			//valid updating
			bool IsHotUpdate(JLight* lit)noexcept final;
			//valid after update end
			bool IsLastFrameHotUpdated(JLight* lit)noexcept final;
			bool IsLastUpdated(JLight* lit)noexcept final;
			bool HasLitRecopyRequest(JLight* lit)noexcept final;
			bool HasShadowMapRecopyRequest(JLight* lit)noexcept final; 
			bool HasDepthTestPassRecopyRequest(JLight* lit)noexcept final;
			bool HasHzbOccComputeRecopyRequest(JLight* lit)noexcept final;
		};
		class FrameIndexInterface final : public JLightPrivate::FrameIndexInterface
		{ 
		private:
			int GetLitFrameIndex(JLight* lit)noexcept final;
			int GetShadowMapFrameIndex(JLight* lit)noexcept final; 
			int GetDepthTestPassFrameIndex(JLight* lit)noexcept final;
			int GetHzbOccComputeFrameIndex(JLight* lit)noexcept final;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
		JLightPrivate::FrameUpdateInterface& GetFrameUpdateInterface()const noexcept final;
		JLightPrivate::FrameIndexInterface& GetFrameIndexInterface()const noexcept final;
	};
}