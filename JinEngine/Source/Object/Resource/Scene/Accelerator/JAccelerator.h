#pragma once
#include"JAcceleratorType.h" 
#include"JAcceleratorOption.h"
#include"../../../../Core/Geometry/JCullingFrustum.h" 
#include"../../../../Core/Geometry/JRay.h"
#include"../../../../Core/Geometry/JBBox.h"
#include"../../../../Core/JCoreEssential.h"
#include<DirectXCollision.h>
#include<vector>

namespace JinEngine
{
	class JGameObject; 
	struct JAcceleratorCullingInfo;
	class JAccelerator
	{
	private:
		JUserPtr<JGameObject>innerRoot = nullptr;
		JUserPtr<JGameObject> debugRoot = nullptr;

		bool isAcceleratorActivated = false;
		bool isDebugActivated = false;
		bool isDebugLeafOnly = true;
		bool isCullingActivated = false;
	private:
		const J_ACCELERATOR_LAYER layer;
	public:
		JAccelerator(const J_ACCELERATOR_LAYER layer);
		virtual ~JAccelerator() = default;
	protected:
		virtual void Build()noexcept = 0;
		virtual void UnBuild()noexcept = 0;
		virtual void Clear()noexcept;
		virtual void OnDebugGameObject()noexcept = 0;
		virtual void OffDebugGameObject()noexcept = 0;
	public:
		J_ACCELERATOR_LAYER GetLayer()const noexcept;
		virtual J_ACCELERATOR_TYPE GetType()const noexcept = 0;
		virtual Core::JBBox GetSceneBBox()const noexcept = 0;
	protected:
		std::vector<JUserPtr<JGameObject>> GetInnerObject()const noexcept;
		JUserPtr<JGameObject> GetInnerRoot()const noexcept;
		JUserPtr<JGameObject> GetDebugRoot()const noexcept;
		JAcceleratorOption GetCommonOption()const noexcept;
	private:
		void SetInnerRoot(const JUserPtr<JGameObject>& newInnerRoot)noexcept;
		void SetDebugRoot(const JUserPtr<JGameObject>& newDebugRoot)noexcept;
		void SetAcceleratorActivate(bool value)noexcept;
		void SetDebugActivated(bool value)noexcept;
		void SetDebugLeafOnly(bool value)noexcept;
		virtual void SetCullingActivate(bool value)noexcept;
	protected:
		void SetCommonOption(const JAcceleratorOption& newOption)noexcept;
	public:
		bool IsInnerRoot(const JUserPtr<JGameObject>& gameObj)const noexcept;
		bool IsDebugRoot(const JUserPtr<JGameObject>& gameObj)const noexcept;
		bool IsAcceleratorActivated()const noexcept;
		bool IsDebugActivated()const noexcept;
		bool IsDebugLeafOnly()const noexcept;
		bool IsCullingActivated()const noexcept;
		bool IsValidLayer(const J_RENDER_LAYER objLayer)const noexcept;
		bool HasInnerRoot()const noexcept;
		bool HasDebugRoot()const noexcept;
		bool CanAddGameObject(const JUserPtr<JGameObject>& gameObj)const noexcept;
	private:
		void FindInnerObject(std::vector<JUserPtr<JGameObject>>& vec, const JUserPtr<JGameObject>& parent)const noexcept;
	public:
		virtual void Culling(JAcceleratorCullingInfo& info)noexcept = 0;
		virtual void Intersect(JAcceleratorIntersectInfo& info)const noexcept = 0;
		virtual void Contain(JAcceleratorContainInfo& info)const noexcept = 0;
		virtual void AlignedObject(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& validCount)const noexcept = 0;
	public:
		virtual void UpdateGameObject(const JUserPtr<JGameObject>& gameObject)noexcept = 0;
	public:
		virtual void AddGameObject(const JUserPtr<JGameObject>& newGameObject)noexcept = 0;
		virtual void RemoveGameObject(const JUserPtr<JGameObject>& gameObj)noexcept = 0;
	};
}