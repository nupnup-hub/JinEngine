#pragma once
#include"JAcceleratorType.h" 
#include"JAcceleratorOption.h" 
#include"../../../../Core/Geometry/JRay.h"
#include"../../../../Core/Geometry/JBBox.h" 
#include"../../../../Graphic/Accelerator/JGpuAcceleratorInterface.h"
#include<DirectXCollision.h>
#include<vector>

namespace JinEngine
{
	class JGameObject;
	class JComponent;
	struct JAcceleratorCullingInfo;
	namespace Graphic
	{
		class JGpuAcceleratorInfo;
	}

	class JAccelerator
	{
	public:
		virtual ~JAccelerator() = default;
	protected:
		virtual void Build()noexcept = 0;
		virtual void UnBuild()noexcept = 0;
	public:
		virtual void Clear()noexcept = 0;
	};
	class JCpuAccelerator : public JAccelerator
	{
	private:
		JUserPtr<JGameObject>innerRoot = nullptr;
		JUserPtr<JGameObject> debugRoot = nullptr;
	private:
		bool isAcceleratorActivated = false;
		bool isDebugActivated = false;
		bool isDebugLeafOnly = true;
		bool isCullingActivated = false;
	private:
		const J_ACCELERATOR_LAYER layer;
	public:
		JCpuAccelerator(const J_ACCELERATOR_LAYER layer);
		~JCpuAccelerator() = default;
	protected: 
		void Clear()noexcept override;
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
	class JGpuAccelerator : public JAccelerator, public Graphic::JGpuAcceleratorInterface
	{
	private:
		JGpuAcceleratorOption option;
	public:
		JGpuAccelerator();
	protected:
		void Build()noexcept final;
		void UnBuild()noexcept final;
	public:
		void Clear()noexcept final;
	public:
		void UpdateTransform(const JUserPtr<JComponent>& comp)noexcept;
	public:
		void AddComponent(const JUserPtr<JComponent>& newComp)noexcept;
		void RemoveComponent(const JUserPtr<JComponent>& comp)noexcept;
	public:
		JGpuAcceleratorOption GetOption()const noexcept;
	public:
		void SetOption(const JGpuAcceleratorOption& newOption);
	public:
		static bool CanBuild()noexcept;
	public:
		void RegisterInterfacePointer();
	};
}