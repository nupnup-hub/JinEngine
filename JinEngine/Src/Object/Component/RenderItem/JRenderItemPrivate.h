#pragma once
#include"../JComponentPrivate.h"

namespace JinEngine
{
	class JRenderItem; 
	namespace Graphic
	{
		class JGraphic; 
		class JShadowMap;
		class JHZBOccCulling;
		struct JObjectConstants;
		struct JBoundingObjectConstants;
		struct JHzbOccObjectConstants;
	}
	namespace Core
	{
		class JSpaceSpatialNode; 
		class JBvh;
	}
	namespace Editor
	{
		class JSceneObserver;
	}
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
			friend class Graphic::JShadowMap;
			friend class Graphic::JHZBOccCulling;
		private:   
			static bool UpdateStart(JRenderItem* rItem, const bool isUpdateForced)noexcept;
			static void UpdateFrame(JRenderItem* rItem, Graphic::JObjectConstants& constant, const uint submeshIndex)noexcept;
			static void UpdateFrame(JRenderItem* rItem, Graphic::JBoundingObjectConstants& constant)noexcept;
			static void UpdateFrame(JRenderItem* rItem, Graphic::JHzbOccObjectConstants& constant)noexcept;
			static void UpdateEnd(JRenderItem* rItem)noexcept;
		private:
			//Count submesh
			static int GetObjectFrameIndex(JRenderItem* rItem)noexcept;		
			//Count ritem	 
			static int GetBoundingFrameIndex(JRenderItem* rItem)noexcept;
			static int GetOccObjectFrameIndex(JRenderItem* rItem)noexcept;
		private:
			static bool IsHotUpdated(JRenderItem* rItem)noexcept;
			static bool IsLastUpdated(JRenderItem* rItem)noexcept;
			static bool HasObjectRecopyRequest(JRenderItem* rItem)noexcept;
			static bool HasBoundingRecopyRequest(JRenderItem* rItem)noexcept;
			static bool HasOccObjectRecopyRequest(JRenderItem* rItem)noexcept;
		};
		class FrameIndexInterface
		{
		private: 
			friend class Core::JSpaceSpatialNode;
			friend class Core::JBvh;
			friend class Graphic::JGraphic;
			friend class Graphic::JShadowMap;
			friend class Editor::JSceneObserver;
		private:
			//Count submesh
			static int GetObjectFrameIndex(JRenderItem* rItem)noexcept;
			//Count ritem	 
			static int GetBoundingFrameIndex(JRenderItem* rItem)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}