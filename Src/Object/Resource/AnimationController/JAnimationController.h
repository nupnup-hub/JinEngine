#pragma once 
#include<memory>
#include"JAnimationControllerInterface.h" 
#include"../../../Core/FSM/JFSMconditionValueType.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationShareData.h" 

namespace JinEngine
{
	struct JEditorDiagram;
	class JSkeletonAsset;
	class JAnimationClip;

	namespace Core
	{
		__interface IJFSMconditionStorageManager;
		struct JAnimationTime;
	}
	namespace Graphic
	{
		struct JAnimationConstants;
	}

	//수정필요 & 추가필요
	//Load & store
	//Clear& stuff
	class JAnimationController : public JAnimationControllerInterface
	{
		REGISTER_CLASS(JAnimationController)
	public:
		constexpr static uint diagramMaxCount = 8;
	private:
		std::unique_ptr<Core::IJFSMconditionStorageManager> conditionStorage;
		std::vector<std::unique_ptr<Core::JAnimationFSMdiagram>> fsmDiagram;
		Core::JAnimationShareData animationShaderData;
	public:
		void Initialize(std::vector<Core::JAnimationTime>& animationtimes, JSkeletonAsset* modelSkeleton)noexcept;
		void Update(std::vector<Core::JAnimationTime>& animationtimes, JSkeletonAsset* modelSkeleton, Graphic::JAnimationConstants& animationConstatns)noexcept;
		
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::ANIMATION_CONTROLLER;
		}
		std::string GetFormat()const noexcept final;
		static std::vector<std::string> GetAvailableFormat()noexcept;
	public:
		uint GetAnimationDiagramCount()const noexcept;
		std::string GetAnimationDiagramName(const size_t diagramGuid)noexcept; 
		std::string GetUniqueDiagramName(const std::string& initName)noexcept;
		std::string GetUniqueStateName(const size_t diagramGuid, const std::string& initName)noexcept;
		std::string GetUniqueConditionName(const std::string& initName)noexcept;
		 
		uint GetConditionCount()const noexcept; 

		void SetConditionName(const size_t guid, const std::string& newName)noexcept;
		void SetConditionValueType(const size_t guid, const Core::J_FSMCONDITION_VALUE_TYPE type)noexcept;

		bool CreateAnimationDiagram(const std::string& name, const size_t guid)noexcept;
		bool CreateAnimationClipState(const std::string& name, const size_t clipGuid, const size_t diagramGuid)noexcept;
		bool CreateTransition(const size_t intputStateGuid, const size_t outputStateGuid, const size_t diagramGuid);
		bool CreateCondition(const std::string& name, const size_t guid)noexcept;

		bool DestroyAnimationDiagram(const size_t diagramGuid)noexcept;
		bool DestroyAnimationState(const size_t statGuid, const size_t diagramGuid)noexcept;
		bool DestroyCondition(const size_t conditionGuid)noexcept;
	public:
		JAnimationControllerEditInterface* EditorInterface() final;
	private:
		Core::JAnimationFSMdiagram* GetDiagram(const size_t guid)noexcept final;
		std::vector<Core::JAnimationFSMdiagram*> GetDiagramVec()noexcept final;
		Core::JAnimationFSMstate* GetState(const size_t diagramGuid, const size_t stateGuid) noexcept final;
		std::vector<Core::JAnimationFSMstate*>& GetStateVec(const size_t diagramGuid)noexcept final;
		Core::JFSMcondition* GetCondition(const size_t guid) noexcept final; 
	private:
		Core::JAnimationFSMdiagram* FindDiagram(const size_t guid) noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool IsValid()const noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JAnimationController* animationCont);
		static JAnimationController* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static void RegisterJFunc();
	private:
		JAnimationController(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JAnimationController();
	};
}