#pragma once
#include"JSpaceSpatialType.h"
#include"JSpaceSpatialSortType.h"
#include"JSpaceSpatialOption.h"
#include"../Geometry/JCullingFrustum.h" 
#include"../Geometry/JRay.h"
#include<DirectXCollision.h>
#include<vector>
namespace JinEngine
{
	class JGameObject;
	namespace Core
	{ 
		class JSpaceSpatial
		{
		private:
			JGameObject* innerRoot = nullptr;
			JGameObject* debugRoot = nullptr; 

			bool isSpaceSpatialActivated = false;
			bool isDebugActivated = false;
			bool isDebugLeafOnly = true;
			bool isCullingActivated = false;
		private:
			const J_SPACE_SPATIAL_LAYER layer;
		public: 
			JSpaceSpatial(const J_SPACE_SPATIAL_LAYER layer);
			virtual ~JSpaceSpatial() = default;
		protected:
			virtual void Build()noexcept = 0;
			virtual void UnBuild()noexcept = 0;
			virtual void Clear()noexcept;
			virtual void OnDebugGameObject()noexcept = 0;
			virtual void OffDebugGameObject()noexcept = 0;
			virtual void OffCulling()noexcept = 0;
		public:
			bool IsInnerRoot(JGameObject* gameObj)const noexcept;
			bool IsDebugRoot(JGameObject* gameObj)const noexcept;
			bool IsSpaceSpatialActivated()const noexcept;
			bool IsDebugActivated()const noexcept;
			bool IsDebugLeafOnly()const noexcept; 
			bool IsCullingActivated()const noexcept;
			bool IsValidLayer(const J_RENDER_LAYER objLayer)const noexcept;

			bool HasInnerRoot()const noexcept;
			bool HasDebugRoot()const noexcept;

			bool CanAddGameObject(JGameObject* gameObj)const noexcept; 
		public:
			J_SPACE_SPATIAL_LAYER GetLayer()const noexcept;
		protected:
			std::vector<JGameObject*> GetInnerObject()const noexcept;
			JGameObject* GetInnerRoot()const noexcept;
			JGameObject* GetDebugRoot()const noexcept;
			JSpaceSpatialOption GetCommonOption()const noexcept;
		private:
			void SetInnerRoot(JGameObject* newInnerRoot)noexcept;
			void SetDebugRoot(JGameObject* newDebugRoot)noexcept;
			void SetSpaceSpatialActivate(bool value)noexcept;
			void SetDebugActivated(bool value)noexcept;
			void SetDebugLeafOnly(bool value)noexcept;
			virtual void SetCullingActivate(bool value)noexcept;
		protected:
			void SetCommonOption(const JSpaceSpatialOption& newOption)noexcept;
		private:
			void FindInnerObject(std::vector<JGameObject*>& vec, JGameObject* parent)const noexcept;
		public:
			virtual void Culling(const JCullingFrustum& camFrustum)noexcept = 0;
			virtual void Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::FXMVECTOR camPos)noexcept = 0;
			virtual JGameObject* IntersectFirst(const JRay& ray)const noexcept = 0;
			virtual void Intersect(const JRay& ray, const J_SPACE_SPATIAL_SORT_TYPE sortType, std::vector<JGameObject*>& res)const noexcept = 0;
			virtual void UpdateGameObject(JGameObject* gameObject)noexcept = 0;
		public:
			virtual void AddGameObject(JGameObject* newGameObject)noexcept = 0;
			virtual void RemoveGameObject(JGameObject* gameObj)noexcept = 0;
		public:
			virtual J_SPACE_SPATIAL_TYPE GetType()const noexcept = 0;
		};
	}
}