#pragma once   
#include"../JResourceObject.h" 
#include"JSceneType.h"
#include"../../Component/JComponentType.h"
#include"../../Component/RenderItem/JRenderLayer.h"
#include"../../../Core/SpaceSpatial/JSpaceSpatialType.h"
#include"../../../Core/SpaceSpatial/Bvh/JBvhOption.h"
#include"../../../Core/SpaceSpatial/Octree/JOctreeOption.h"
#include"../../../Core/SpaceSpatial/Kd-tree/JKdTreeOption.h"
#include<memory>
#include<vector>  
#include<DirectXCollision.h>

namespace JinEngine
{
	class JComponent;
	class JCamera;
	class JGameObject;
	class JScenePrivate;
	namespace Core
	{
		class JRay;  
	}
	class JScene : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JScene)
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			const J_SCENE_USE_CASE_TYPE useCaseType;
		public:
			InitData(const uint8 formatIndex,
				JDirectory* directory,
				const J_SCENE_USE_CASE_TYPE useCaseType);
			InitData(const size_t guid,
				const uint8 formatIndex,
				JDirectory* directory,
				const J_SCENE_USE_CASE_TYPE useCaseType);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				JDirectory* directory,
				const J_SCENE_USE_CASE_TYPE useCaseType); 
		};
	protected: 
		class LoadMetaData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			J_SCENE_USE_CASE_TYPE useCaseType;
		public:
			bool isOpen = false;
			bool isMainScene = false;
			//space spatial trigger
			bool isActivatedSpaceSpatial;
			bool hasInnerRoot[(uint)Core::J_SPACE_SPATIAL_TYPE::COUNT][(uint)Core::J_SPACE_SPATIAL_LAYER::COUNT];
			size_t innerRootGuid[(uint)Core::J_SPACE_SPATIAL_TYPE::COUNT][(uint)Core::J_SPACE_SPATIAL_LAYER::COUNT];
			Core::JOctreeOption octreeOption[(uint)Core::J_SPACE_SPATIAL_LAYER::COUNT];
			Core::JBvhOption bvhOption[(uint)Core::J_SPACE_SPATIAL_LAYER::COUNT];
			Core::JKdTreeOption kdTreeOption[(uint)Core::J_SPACE_SPATIAL_LAYER::COUNT];
		public:
			LoadMetaData(JDirectory* directory);
		};
	private:
		friend class JScenePrivate;
		class JSceneImpl;
	private:
		std::unique_ptr<JSceneImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SCENE;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JGameObject* GetRootGameObject()noexcept;
		JGameObject* GetDebugRootGameObject()noexcept;
		JGameObject* GetGameObject(const uint index)noexcept;
		JCamera* GetMainCamera()noexcept;
		uint GetGameObjectCount()const noexcept;
		uint GetComponetCount(const J_COMPONENT_TYPE cType)const noexcept;
		uint GetMeshCount()const noexcept;
		J_SCENE_USE_CASE_TYPE GetUseCaseType()const noexcept;
		std::vector<JGameObject*> GetAlignedObject(const Core::J_SPACE_SPATIAL_LAYER layer, const DirectX::BoundingFrustum& frustum)const noexcept;
		Core::JOctreeOption GetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept;
		Core::JBvhOption GetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept;
		Core::JKdTreeOption GetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer)const noexcept;
	public:
		void SetOctreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JOctreeOption& newOption)noexcept;
		void SetBvhOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JBvhOption& newOption)noexcept;
		void SetKdTreeOption(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JKdTreeOption& newOption)noexcept;
	public:
		bool IsActivatedSceneTime()const noexcept;
		bool IsPauseSceneTime()const noexcept;
		bool IsMainScene()const noexcept;
		bool IsSpaceSpatialActivated()const noexcept;
		bool HasComponent(const J_COMPONENT_TYPE cType)const noexcept;
	public:
		JGameObject* FindGameObject(const size_t guid)noexcept;
		//Intersect by scene space spatial
		JGameObject* IntersectFirst(const Core::J_SPACE_SPATIAL_LAYER layer, const Core::JRay& ray)const noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JScene(const InitData& initData);
		~JScene();
	};
}

  

 