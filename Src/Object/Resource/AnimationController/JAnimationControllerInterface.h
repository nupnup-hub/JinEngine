#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h" 

namespace JinEngine
{
	namespace Core
	{
		class JAnimationFSMdiagram;
		class JAnimationFSMstate;
		class JFSMcondition;
	}

	namespace Editor
	{
		class JAnimationControllerEditor;
	}
	class JAnimationControllerEditInterface
	{
	private:
		friend class Editor::JAnimationControllerEditor;
	protected:
		virtual ~JAnimationControllerEditInterface() = default;
	public:
		virtual JAnimationControllerEditInterface* EditorInterface() = 0;
	private:
		virtual Core::JAnimationFSMdiagram* GetDiagram(const size_t guid)noexcept = 0;
		virtual std::vector<Core::JAnimationFSMdiagram*> GetDiagramVec()noexcept = 0;
		virtual Core::JAnimationFSMstate* GetState(const size_t diagramGuid, const size_t stateGuid)noexcept = 0;
		virtual std::vector<Core::JAnimationFSMstate*>& GetStateVec(const size_t diagramGuid)noexcept = 0;
		virtual Core::JFSMcondition* GetCondition(const size_t guid)noexcept = 0; 
	};
	class JAnimationControllerInterface : public JResourceObject, 
		public JAnimationControllerEditInterface,
		public JClearableInterface
	{
	protected:
		JAnimationControllerInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}