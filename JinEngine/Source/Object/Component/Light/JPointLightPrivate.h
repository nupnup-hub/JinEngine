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
#include"JLightPrivate.h"

namespace JinEngine
{ 
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
			void UpdateFrame(JLight* lit, Graphic::JLightConstantsSet& set)noexcept final;
			void UpdateEnd(JLight* lit)noexcept final;
		private:
			int GetFrameIndex(JLight* lit, const uint layerIndex)noexcept final;
			int GetFrameIndexSize(JLight* lit, const uint layerIndex)noexcept final;
			int GetShadowFrameLayerIndex(JLight* lit)noexcept final;
		private:
			//valid updating
			bool IsFrameHotDirted(JLight* lit)noexcept final;
			//valid after update end
			bool IsLastFrameHotUpdated(JLight* lit)noexcept final;
			bool IsLastUpdated(JLight* lit)noexcept final; 
		};
		class FrameIndexInterface final : public JLightPrivate::FrameIndexInterface
		{ 
		private:
			int GetFrameIndex(JLight* lit, const uint layerIndex)noexcept final;
			int GetShadowFrameLayerIndex(JLight* lit)noexcept final;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
		JLightPrivate::FrameUpdateInterface& GetFrameUpdateInterface()const noexcept final;
		JLightPrivate::FrameIndexInterface& GetFrameIndexInterface()const noexcept final;
	};
}