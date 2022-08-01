#pragma once 
#include<memory>
#include"../JResourceObject.h" 
#include"../../../Core/FSM/JFSMconditionValueType.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationShareData.h" 

namespace JinEngine
{
	struct EditorDiagram;
	class JSkeletonAsset;
	class JAnimationClip;

	namespace Core
	{
		__interface IJFSMconditionStorageManager;
		class JFSMcondition;
		class JAnimationFSMdiagram;
		class JAnimationFSMstate;
		struct JAnimationTime;
	}
	namespace Graphic
	{
		struct JAnimationConstants;
	}

	class JAnimationController : public JResourceObject
	{
		REGISTER_CLASS(JAnimationController)
	public:
		constexpr static uint diagramMaxCount = 8;
	private:
		std::unique_ptr<Core::IJFSMconditionStorageManager> conditionStorage;
		std::vector<std::unique_ptr<Core::JAnimationFSMdiagram>> stateDiagram;
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
		uint GetAnimationDiagramCount()const noexcept;
		std::string GetAnimationDiagramName(const uint diagramIndex)noexcept;
		EditorDiagram* GetEditorDiagram(const uint diagramIndex)noexcept;

		//passing
		bool GetAnimationFSMstateVectorHandle(const uint diagramIndex, _Out_ uint& stateCount, _Out_ std::vector<Core::JAnimationFSMstate*>::const_iterator& iter)noexcept;
		uint GetConditionCount()const noexcept;
		Core::JFSMcondition* GetCondition(const uint index)noexcept;

		static std::vector<std::string> GetAvailableFormat()noexcept;

		void SetConditionName(const std::string& oldName, const std::string& newName)noexcept;
		void SetConditionValueType(const std::string& name, const Core::J_FSMCONDITION_VALUE_TYPE type)noexcept;

		bool CreateAnimationDiagram()noexcept;
		bool CreateAnimationClip(const uint diagramIndex, const float initPosX, const float initPosY)noexcept;
		bool CreateTransition(const uint diagramIndex, const std::string& stateName, const std::string& outputStateName);
		bool CreateParameter()noexcept;

		bool EraseAnimationDiagram(const uint diagramIndex)noexcept;
		bool EraseAnimationState(const uint diagramIndex, const std::string& stateName)noexcept;
		bool EraseParameter(const std::string& paraName)noexcept;

	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JAnimationController* animationCont);
		static JAnimationController* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static void RegisterFunc();
	private:
		JAnimationController(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JAnimationController();
	};
}