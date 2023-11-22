#pragma once
#include"../JComponentPrivate.h"

namespace JinEngine
{ 
	class JLight; 
	class JDirectionalLight;
	class JPointLight;
	class JSpotLight;
	class JRectLight;

	namespace Graphic
	{
		class JGraphic;  
		class JFrameDirty;
		struct JDrawHelper;
	}
	class JLightPrivate : public JComponentPrivate
	{
	public:
		class AssetDataIOInterface : public JComponentPrivate::AssetDataIOInterface
		{
		protected:
			Core::J_FILE_IO_RESULT LoadLightData(JFileIOTool& tool, JUserPtr<JLight> user);
			Core::J_FILE_IO_RESULT StoreLightData(JFileIOTool& tool, const JUserPtr<JLight>& user);
		};
		class CreateInstanceInterface : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
		protected:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept override;
		};
		class DestroyInstanceInterface : public JComponentPrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced)override;
		};
		class FrameUpdateInterface
		{
		private:
			friend class Graphic::JGraphic; 
		private:
			virtual bool UpdateStart(JLight* lit, const bool isUpdateForced)noexcept = 0;
			virtual void UpdateEnd(JLight* lit)noexcept = 0;
		private:
			virtual int GetLitFrameIndex(JLight* lit)noexcept = 0;
			virtual int GetShadowMapFrameIndex(JLight* lit)noexcept = 0; 
			virtual int GetDepthTestPassFrameIndex(JLight* lit)noexcept = 0;
			virtual int GetHzbOccComputeFrameIndex(JLight* lit)noexcept = 0;
		private:
			//valid updating
			virtual bool IsHotUpdate(JLight* lit)noexcept = 0;
			//valid after update end
			virtual bool IsLastFrameHotUpdated(JLight* lit)noexcept = 0;
			virtual bool IsLastUpdated(JLight* lit)noexcept = 0;
			virtual bool HasLitRecopyRequest(JLight* lit)noexcept = 0;
			virtual bool HasShadowMapRecopyRequest(JLight* lit)noexcept = 0;
			virtual bool HasDepthTestPassRecopyRequest(JLight* lit)noexcept = 0;
			virtual bool HasHzbOccComputeRecopyRequest(JLight* lit)noexcept = 0;
		};
		class FrameIndexInterface
		{
		private:
			friend struct Graphic::JDrawHelper;
		private:
			virtual int GetLitFrameIndex(JLight* lit)noexcept = 0;
			virtual int GetShadowMapFrameIndex(JLight* lit)noexcept = 0;
			virtual int GetDepthTestPassFrameIndex(JLight* lit)noexcept = 0;
			virtual int GetHzbOccComputeFrameIndex(JLight* lit)noexcept = 0;
		};
		class FrameDirtyInterface final
		{
		private:    
			friend class JDirectionalLight;
			friend class JPointLight;
			friend class JSpotLight;
			friend class JRectLight;
		private:
			//almost JFrameDirtyListener is impl class
			static void RegisterFrameDirtyListener(JLight* lit, Graphic::JFrameDirty* listener, const size_t guid)noexcept;
			static void DeRegisterFrameDirtyListener(JLight* lit, const size_t guid)noexcept;
		};
	public:  
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		virtual JLightPrivate::FrameUpdateInterface& GetFrameUpdateInterface()const noexcept = 0;
		virtual JLightPrivate::FrameIndexInterface& GetFrameIndexInterface()const noexcept = 0; 
	};
}