#pragma once 
#include<memory>
#include"JAnimationControllerInterface.h" 
#include"../../../Core/FSM/JFSMparameterValueType.h"  
#include"../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	struct JEditorDiagram;
	class JSkeletonAsset;
	class JAnimationClip;

	namespace Core
	{
		class JAnimationFSMdiagram; 
		class JAnimationFSMstate;
		class JAnimationFSMtransition;

		class JFSMparameterStorage; 
		class IJFSMparameterStorageInterface;
		class JFSMparameter;
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
		};
		using InitData = JAnimationControllerInitData;
	private:
		std::unique_ptr<Core::JFSMparameterStorage> paramStorage;
		std::vector<Core::JAnimationFSMdiagram*> fsmDiagram;
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
		uint GetParameterCount()const noexcept;   

		Core::JAnimationFSMdiagram* GetDiagram(const size_t guid)noexcept;
		Core::JAnimationFSMdiagram* GetDiagramByIndex(const uint index)noexcept;
		const std::vector<Core::JAnimationFSMdiagram*>& GetDiagramVec()noexcept;
		Core::JFSMparameter* GetParameter(const size_t guid)noexcept;
		Core::JFSMparameter* GetParameterByIndex(const uint index)noexcept;

		bool CanCreateDiagram()const noexcept;
		bool CanCreateParameter()const noexcept;
		bool CanCreateState(Core::JAnimationFSMdiagram* diagram)const noexcept;
	public:
		Core::JAnimationFSMdiagram* CreateFSMdiagram(const size_t guid = Core::MakeGuid())noexcept;
		Core::JFSMparameter* CreateFSMparameter(const size_t guid = Core::MakeGuid())noexcept;
		Core::JAnimationFSMstate* CreateFSMclip(Core::JAnimationFSMdiagram* diagram, const size_t guid = Core::MakeGuid())noexcept;
		Core::JAnimationFSMtransition* CreateFsmtransition(Core::JAnimationFSMdiagram* diagram,
			Core::JAnimationFSMstate* from, 
			Core::JAnimationFSMstate* to,
			const size_t guid = Core::MakeGuid())noexcept;
	private:  
		bool AddType(Core::JFSMdiagram* diagram) noexcept final;
		bool RemoveType(Core::JFSMdiagram* diagram) noexcept final;
		Core::JFSMparameterStorageUserAccess* GetParameterStorageUser()noexcept;
		Core::JAnimationFSMdiagram* FindDiagram(const size_t guid) noexcept;
	public:
		JAnimationControllerFrameUpdateInterface* FrameUpdateInterface();
	private:
		void Initialize(Core::JAnimationUpdateData* updateData)noexcept final;
		void Update(Core::JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept final;
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