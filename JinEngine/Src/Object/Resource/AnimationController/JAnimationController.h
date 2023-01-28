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
		class JAnimationFSMdiagram; 
		class JAnimationFSMstate;

		class JFSMconditionStorage; 
		class IJFSMconditionOwnerInterface;
		class JFSMcondition;
		struct JAnimationTime;
	}
	namespace Graphic
	{
		struct JAnimationConstants;
	}

	//수정필요 & 추가필요
	//Load & store
	//Clear& stuff
	class JAnimationController final : public JAnimationControllerInterface
	{
		REGISTER_CLASS(JAnimationController)
	public:
		struct JAnimationControllerInitData : JResourceInitData
		{
		public:
			JAnimationControllerInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JAnimationController>(GetAvailableFormat()[0]));
			JAnimationControllerInitData(const std::wstring& name,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JAnimationController>(GetAvailableFormat()[0]));
			JAnimationControllerInitData(JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JAnimationController>(GetAvailableFormat()[0]));
		public:
			J_RESOURCE_TYPE GetResourceType() const noexcept;
		};
		using InitData = JAnimationControllerInitData;
	public:
		static constexpr uint diagramMaxCount = 8; 
	private:
		std::unique_ptr<Core::JFSMconditionStorage> conditionStorage;
		std::vector<Core::JAnimationFSMdiagram*> fsmDiagram;
		Core::JAnimationShareData animationShaderData;
	public:
		void Initialize(std::vector<Core::JAnimationTime>& animationtimes, JSkeletonAsset* modelSkeleton)noexcept;
		void Update(std::vector<Core::JAnimationTime>& animationtimes, JSkeletonAsset* modelSkeleton, Graphic::JAnimationConstants& animationConstatns)noexcept;
	public:
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::ANIMATION_CONTROLLER;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		uint GetDiagramCount()const noexcept; 
		uint GetConditionCount()const noexcept;   

		Core::JAnimationFSMdiagram* GetDiagram(const size_t guid)noexcept;
		Core::JAnimationFSMdiagram* GetDiagramByIndex(const uint index)noexcept;
		const std::vector<Core::JAnimationFSMdiagram*>& GetDiagramVec()noexcept;
		Core::JFSMcondition* GetCondition(const size_t guid)noexcept;
		Core::JFSMcondition* GetConditionByIndex(const uint index)noexcept;

		bool CanCreateDiagram()const noexcept;
		bool CanCreateCondition()const noexcept;
		bool CanCreateState(Core::JAnimationFSMdiagram* diagram)const noexcept;
	public:
		Core::JAnimationFSMdiagram* CreateFSMDiagram(const size_t guid)noexcept;
		Core::JFSMcondition* CreateFSMCondition(const size_t guid)noexcept;
		Core::JAnimationFSMstate* CreateFSMClip(const size_t guid, Core::JAnimationFSMdiagram* diagram)noexcept;
	private: 
		Core::JFSMconditionStorageUserAccess* GetConditionStorageUser() noexcept final;
		bool AddDiagram(Core::JFSMdiagram* diagram) noexcept final;
		bool RemoveDiagram(Core::JFSMdiagram* diagram) noexcept final;
	private:
		Core::JAnimationFSMdiagram* FindDiagram(const size_t guid) noexcept;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool WriteAnimationControllerData();
		bool ReadAnimationControllerData();
		bool IsValid()const noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JAnimationController* animationCont);
		static JAnimationController* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);
		static void RegisterJFunc();
	private:
		JAnimationController(const JAnimationControllerInitData& initdata);
		~JAnimationController();
	};
}