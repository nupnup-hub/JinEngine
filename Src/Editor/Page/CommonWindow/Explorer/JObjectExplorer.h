#pragma once
#include"../../JEditorWindow.h"  
#include"../../../../Object/JObjectType.h"
#include"../../../../Utility/JDelegate.h" 
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h"
 
namespace JinEngine
{
	class JGameObject;
	class JObject;
	namespace Editor
	{
		class JEditorString;
		class JEditorPopup;
		enum class OBJECT_EXPLORER_EVENT_TYPE
		{
			NONE,
			CHANGE_PARENT,
			MAKE_MODEL
		};
		struct ObjectExplorerEvent
		{
		public:
			OBJECT_EXPLORER_EVENT_TYPE eventType = OBJECT_EXPLORER_EVENT_TYPE::NONE;
			JGameObject* selectGameObj = nullptr;
			JObject* tarObject = nullptr;
		};

		class JObjectExplorer : public JEditorWindow
		{
			struct JDelegateDataStruct
			{
			public:
				bool activatedRename;
				size_t renameObjectGuid;
				std::string renameBuf;
			};
		private:
			std::unique_ptr<JEditorString>editorString;
			std::unique_ptr<JEditorPopup>explorerPopup;
			JDelegateDataStruct JDelegateData;
			std::unordered_map<size_t, JDelegate<void(JEditorUtility*)>> gameObjFunc;
		public:
			JObjectExplorer(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JObjectExplorer();
			JObjectExplorer(const JObjectExplorer& rhs) = delete;
			JObjectExplorer& operator=(const JObjectExplorer& rhs) = delete;

			void Initialize(JEditorUtility* editorUtility)noexcept;
			void UpdateWindow(JEditorUtility* editorUtility)override;

			bool Activate(JEditorUtility* editorUtility) final;
			bool DeActivate(JEditorUtility* editorUtility) final;
			bool OnFocus(JEditorUtility* editorUtility) final;
			bool OffFocus(JEditorUtility* editorUtility) final;
		private:
			void BuildObjectExplorer(JScene* scene, JEditorUtility* editorUtility);
			void ObjectExplorerOnScreen(JScene* scene, JGameObject* obj, JEditorUtility* editorUtility, ObjectExplorerEvent& objectExplorerEvent);
		private:
			virtual void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)final;
		};
	}
}