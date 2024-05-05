#pragma once   
#include"../JResourceObject.h" 
#include"JSceneType.h"
#include"Accelerator/JAcceleratorType.h" 
#include"Accelerator/Bvh/JBvhOption.h"
#include"Accelerator/Octree/JOctreeOption.h"
#include"Accelerator/Kd-tree/JKdTreeOption.h"
#include"../../Component/JComponentType.h"
#include"../../Component/RenderItem/JRenderLayer.h" 
#include"../../../Graphic/Accelerator/JGpuAcceleratorInterface.h"
#include"../../../Core/Geometry/Mesh/JMeshType.h" 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JComponent;
	class JCamera;
	class JLight;
	class JGameObject;
	class JScenePrivate;
	namespace Core
	{
		class JRay;  
	}
	class JScene : public JResourceObject, public Graphic::JGpuAcceleratorUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JScene) 
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			const J_SCENE_USE_CASE_TYPE useCaseType;
		public:
			InitData(const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const J_SCENE_USE_CASE_TYPE useCaseType);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				const J_SCENE_USE_CASE_TYPE useCaseType);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
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
			bool isActivatedAccelerator;
			bool hasInnerRoot[(uint)J_ACCELERATOR_TYPE::COUNT][(uint)J_ACCELERATOR_LAYER::COUNT];
			size_t innerRootGuid[(uint)J_ACCELERATOR_TYPE::COUNT][(uint)J_ACCELERATOR_LAYER::COUNT];
			JOctreeOption octreeOption[(uint)J_ACCELERATOR_LAYER::COUNT];
			JBvhOption bvhOption[(uint)J_ACCELERATOR_LAYER::COUNT];
		JKdTreeOption kdTreeOption[(uint)J_ACCELERATOR_LAYER::COUNT];
		public:
			LoadMetaData(const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JScenePrivate;
		class JSceneImpl;
	private:
		std::unique_ptr<JSceneImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final; 
		const Graphic::JGpuAcceleratorUserInterface GpuAcceleratorUserInterface()const noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SCENE;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JUserPtr<JGameObject> GetRootGameObject()noexcept;
		JUserPtr<JGameObject> GetDebugRootGameObject()noexcept;
		JUserPtr<JGameObject> GetGameObject(const uint index)noexcept;
		JUserPtr<JGameObject> GetGameObject(const std::wstring& name)noexcept;
		uint GetGameObjectCount()const noexcept;
		uint GetGameObjectCount(const J_RENDER_LAYER layer)const noexcept;
		uint GetComponetCount(const J_COMPONENT_TYPE cType)const noexcept;
		uint GetMeshCount()const noexcept;
		J_SCENE_USE_CASE_TYPE GetUseCaseType()const noexcept; 
		std::vector<JUserPtr<JGameObject>> GetGameObjectVec()const noexcept;
		std::vector<JUserPtr<JGameObject>> GetGameObjectVec(const J_RENDER_LAYER layer, const Core::J_MESHGEOMETRY_TYPE mesh)const noexcept;
		std::vector<JUserPtr<JComponent>> GetComponentVec(const J_COMPONENT_TYPE cType)const noexcept;
		JUserPtr<JLight> GetFirstDirectionalLight()const noexcept;
		JOctreeOption GetOctreeOption(const J_ACCELERATOR_LAYER layer)const noexcept;
		JBvhOption GetBvhOption(const J_ACCELERATOR_LAYER layer)const noexcept;
		JKdTreeOption GetKdTreeOption(const J_ACCELERATOR_LAYER layer)const noexcept;		
		DirectX::BoundingBox GetSceneBBox(const J_ACCELERATOR_LAYER layer = J_ACCELERATOR_LAYER::COMMON_OBJECT)const noexcept;
		/**
		* @return return 0 if scene time deactivated 
		*/
		float GetTotalTime()const noexcept;
		/**
		* @return return 0 if scene time deactivated
		*/
		float GetDeltaTime()const noexcept;
	public:
		void SetOctreeOption(const J_ACCELERATOR_LAYER layer, const JOctreeOption& newOption)noexcept;
		void SetBvhOption(const J_ACCELERATOR_LAYER layer, const JBvhOption& newOption)noexcept;
		void SetKdTreeOption(const J_ACCELERATOR_LAYER layer, const JKdTreeOption& newOption)noexcept;
	public:
		bool IsActivatedSceneTime()const noexcept;
		bool IsPauseSceneTime()const noexcept;
		bool IsMainScene()const noexcept;
		bool IsAcceleratorActivated()const noexcept;
		bool HasComponent(const J_COMPONENT_TYPE cType)const noexcept;
		bool HasCanCullingAccelerator(const J_ACCELERATOR_LAYER layer)const noexcept;
		bool AllowLightCulling()const noexcept;
		bool CanUseAcceleratorUtility(const J_ACCELERATOR_LAYER layer, const J_ACCELERATOR_TYPE type)const noexcept;
	public:
		JUserPtr<JGameObject> FindGameObject(const size_t guid)noexcept;
		JUserPtr<JCamera> FindFirstSelectedCamera(const bool allowEditorCam)const noexcept;
		//Intersect by scene space spatial 
		JUserPtr<JGameObject> IntersectFirst(const Core::JRay& ray, const J_ACCELERATOR_LAYER layer, const bool findOtherAcceleratorIfTypeNull = false)const noexcept;
		JUserPtr<JGameObject> IntersectFirst(JAcceleratorIntersectInfo& info)const noexcept;
		void Intersect(JAcceleratorIntersectInfo& info)const noexcept;
		void Contain(JAcceleratorContainInfo& info);
		std::vector<JUserPtr<JGameObject>> AlignedObject(JAcceleratorAlignInfo& info)const noexcept;
		/**
		* fast version
		* aligned vector는 algined할 layer에 render item만큼 크기를 갖고있어야한다.
		* 크기가 부족할시 함수내부에서 재할당한다.
		*/
		void AlignedObjectF(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& count)const noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JScene(const InitData& initData);
		~JScene();
	};
}

  

 