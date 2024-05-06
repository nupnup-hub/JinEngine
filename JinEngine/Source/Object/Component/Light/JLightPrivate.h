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
		class JFrameIndexAccess;
		struct JLightConstantsSet;
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
			virtual void UpdateFrame(JLight* lit, Graphic::JLightConstantsSet& set)noexcept = 0;
			virtual void UpdateEnd(JLight* lit)noexcept = 0;
		private:
			virtual int GetFrameIndex(JLight* lit, const uint layerIndex)noexcept = 0;
			virtual int GetFrameIndexSize(JLight* lit, const uint layerIndex)noexcept = 0;
			virtual int GetShadowFrameLayerIndex(JLight* lit)noexcept = 0;
		private:
			//valid updating
			virtual bool IsFrameHotDirted(JLight* lit)noexcept = 0;
			//valid after update end
			virtual bool IsLastFrameHotUpdated(JLight* lit)noexcept = 0;
			virtual bool IsLastUpdated(JLight* lit)noexcept = 0; 
		};
		class FrameIndexInterface
		{
		private:
			friend class Graphic::JFrameIndexAccess;
		private:
			virtual int GetFrameIndex(JLight* lit, const uint layerIndex)noexcept = 0;
			virtual int GetShadowFrameLayerIndex(JLight* lit)noexcept = 0;
		};
		class ChildInterface final
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
			static void UpdateLightShape(const JUserPtr<JLight>& lit)noexcept;
		};
	public:  
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		virtual JLightPrivate::FrameUpdateInterface& GetFrameUpdateInterface()const noexcept = 0;
		virtual JLightPrivate::FrameIndexInterface& GetFrameIndexInterface()const noexcept = 0; 
	};
}