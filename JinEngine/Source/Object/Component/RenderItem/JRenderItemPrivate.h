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
	class JRenderItem; 
	class JAcceleratorNode;
	class JBvh;
	namespace Graphic
	{
		class JGraphic;   
		class JFrameIndexAccess; 
		struct JObjectConstantsSet;
	}
	//Debug
	/*
	namespace Editor
	{
		class JSceneObserver;
	}
	*/
	class JRenderItemPrivate final : public JComponentPrivate
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
			static bool UpdateStart(JRenderItem* rItem, const bool isUpdateForced)noexcept;
			static void UpdateFrame(JRenderItem* rItem, Graphic::JObjectConstantsSet& set)noexcept;
			static void UpdateEnd(JRenderItem* rItem)noexcept;
		private:
			//Count submesh
			static int GetObjectFrameIndex(JRenderItem* rItem)noexcept;		
			//Count ritem	 
			static int GetBoundingFrameIndex(JRenderItem* rItem)noexcept;
			static int GetOccObjectFrameIndex(JRenderItem* rItem)noexcept;
			static int GetRefInfoFrameIndex(JRenderItem* rItem)noexcept;
		private:
			static bool IsLastFrameHotUpdated(JRenderItem* rItem)noexcept;
			static bool IsLastUpdated(JRenderItem* rItem)noexcept; 
		};
		class FrameIndexInterface
		{
		private: 
			friend class JAcceleratorNode;
			friend class JBvh;  
			friend class Graphic::JFrameIndexAccess;
		private:
			//per submesh
			static int GetObjectFrameIndex(JRenderItem* rItem)noexcept;
			//per ritem	 
			static int GetBoundingFrameIndex(JRenderItem* rItem)noexcept;
			static int GetRefInfoFrameIndex(JRenderItem* rItem)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}