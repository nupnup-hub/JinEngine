/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once 
#include"JEditorEventType.h"
#include"../Page/JEditorPageEnum.h"
#include"../Page/JEditorWindowEnum.h"
#include"../Popup/JEditorPopupType.h"
#include"../Interface/JEditorTransitionInterface.h"
#include"../../Core/Log/JLog.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/JCoreEssential.h" 
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Object/Resource/JResourceObjectType.h"

namespace JinEngine
{
	namespace Core
	{
		class Core::JIdentifier;
	}
	namespace Editor
	{
		class JEditorPage;
		class JEditorWindow;
		class JEditorPopupWindow;

		struct JEditorEvStruct
		{
		public:
			enum class RANGE
			{
				CALLER = 1 << 0,
				OTHERS = 1 << 1,
				ALL = CALLER | OTHERS
			};
		public:
			const J_EDITOR_PAGE_TYPE pageType;
			const RANGE evRange = RANGE::ALL;
		public:
			JEditorEvStruct(const J_EDITOR_PAGE_TYPE pageType);
			JEditorEvStruct(const J_EDITOR_PAGE_TYPE pageType, const RANGE evRange);
			virtual ~JEditorEvStruct() = default;
		public:
			virtual bool PassDefectInspection()const noexcept = 0;
			virtual J_EDITOR_EVENT GetEventType()const noexcept = 0;
		public:
			bool AllowExecuteCallerEv()const noexcept;
			bool AllowExecuteOtherEv()const noexcept; 
			bool CanExecuteCallerEv(const size_t senderGuid, const size_t reciverGuid);
			bool CanExecuteOtherEv(const size_t senderGuid, const size_t reciverGuid);
		};
		struct JEditorMouseClickEvStruct : public JEditorEvStruct
		{
		public:
			const std::string windowName;
			//0 Left 1 middle 2right
			const uint clickBtn;
		public:
			JEditorMouseClickEvStruct(const std::string& windowName, const uint clickBtn, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorPushSelectObjectEvStruct : public JEditorEvStruct
		{
		public:
			const J_EDITOR_WINDOW_TYPE wndType;
			std::vector<JUserPtr<Core::JIdentifier>> selectObjVec;
		public:
			JEditorPushSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_TYPE wndType,
				const JUserPtr<Core::JIdentifier> selectObj,
				const RANGE evRange);
			JEditorPushSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_TYPE wndType,
				const std::vector<JUserPtr<Core::JIdentifier>> selectObj,
				const RANGE evRange);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			JUserPtr<Core::JIdentifier> GetFirstMatchedTypeObject(const Core::JTypeInfo& typeInfo)const noexcept;
			JUserPtr<Core::JIdentifier> GetLastMatchedTypeObject(const Core::JTypeInfo& typeInfo)const noexcept;
		};
		struct JEditorPopSelectObjectEvStruct : public JEditorEvStruct
		{
		public:
			std::vector<JUserPtr<Core::JIdentifier>> selectObjVec;
		public:
			JEditorPopSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType, 
				JUserPtr<Core::JIdentifier> selectObj,
				const RANGE evRange);
			JEditorPopSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType, 
				const std::vector<JUserPtr<Core::JIdentifier>> selectObj,
				const RANGE evRange);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			bool IsPopTarget(const size_t guid)const noexcept;
		};
		struct JEditorClearSelectObjectEvStruct : public JEditorEvStruct
		{
		public:
			JEditorClearSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType, const RANGE evRange);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorOpenPageEvStruct : public JEditorEvStruct
		{
		public:
			const size_t typeGuid;
			const size_t openSeletedGuid;
			const bool hasOpenSeleted = false;
		public:
			JEditorOpenPageEvStruct(const J_EDITOR_PAGE_TYPE pageType, JUserPtr<Core::JIdentifier> openSelected = {});
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			JUserPtr<Core::JIdentifier> GetOpenSeleted()const noexcept;
		};
		struct JEditorClosePageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorClosePageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorActPageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorActPageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorDeActPageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorDeActPageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorFocusPageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPage* focusPage;
		public:
			JEditorFocusPageEvStruct(JEditorPage* focusPage);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorUnFocusPageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPage* unFocusPage;
		public:
			JEditorUnFocusPageEvStruct(JEditorPage* unFocusPage);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorMaximizePageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPage* page = nullptr;
			JVector2F prePagePos;
			JVector2F prePageSize;
		public:
			JEditorMaximizePageEvStruct(JEditorPage* page,
				const JVector2F& prePagePos,
				const JVector2F& prePageSize);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorMinimizePageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPage* page = nullptr;
			JVector2F prePagePos;
			JVector2F prePageSize;
			float height;
		public:
			JEditorMinimizePageEvStruct(JEditorPage* page,
				const JVector2F& prePagePos,
				const JVector2F& prePageSize,
				const float height);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorPreviousSizePageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPage* page = nullptr;
			bool useLazy = true;
		public:
			JEditorPreviousSizePageEvStruct(JEditorPage* page, const bool useLazy);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorOpenWindowEvStruct : public JEditorEvStruct
		{
		public:
			const std::string openWindowName;
		public:
			JEditorOpenWindowEvStruct(const std::string& openWindowName, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorCloseWindowEvStruct : public JEditorEvStruct
		{
		public:
			const std::string closeWindowName;
		public:
			JEditorCloseWindowEvStruct(const std::string& closeWindowName, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorActWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* actWindow;
		public:
			JEditorActWindowEvStruct(JEditorWindow* actWindow);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorDeActWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* deActWindow;
		public:
			JEditorDeActWindowEvStruct(JEditorWindow* deActWindow);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorFocusWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* focusWindow;
		public:
			JEditorFocusWindowEvStruct(JEditorWindow* focusWindow);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorUnFocusWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* unFocusWindow;
		public:
			JEditorUnFocusWindowEvStruct(JEditorWindow* unFocusWindow);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorOpenPopupWindowEvStruct : public JEditorEvStruct
		{
		public:
			using PopupWndFuncTuple = std::tuple <J_EDITOR_POPUP_WINDOW_FUNC_TYPE, std::unique_ptr< Core::JBindHandleBase>>;
		public:
			JEditorPopupWindow* popupWindow = nullptr;
			J_EDITOR_POPUP_WINDOW_TYPE popupType;
			std::vector<PopupWndFuncTuple> tupleVec;
			std::string desc;
		public:
			JEditorOpenPopupWindowEvStruct(JEditorPopupWindow* popupWindow, const J_EDITOR_PAGE_TYPE pageType);
			JEditorOpenPopupWindowEvStruct(const J_EDITOR_POPUP_WINDOW_TYPE type, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			void AddBind(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type, std::unique_ptr< Core::JBindHandleBase>&& bind);
			void AddDesc(const std::string& newDesc);
		};
		struct JEditorClosePopupWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPopupWindow* popupWindow = nullptr;
			J_EDITOR_POPUP_WINDOW_TYPE popupType;
		public:
			JEditorClosePopupWindowEvStruct(JEditorPopupWindow* popupWindow, const J_EDITOR_PAGE_TYPE pageType);
			JEditorClosePopupWindowEvStruct(const J_EDITOR_POPUP_WINDOW_TYPE popupType, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorMaximizeWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* wnd = nullptr; 
			JVector2F preWindowPos;
			JVector2F preWindowSize;
		public:
			JEditorMaximizeWindowEvStruct(JEditorWindow* wnd, 
				const JVector2F& preWindowPos, 
				const JVector2F& preWindowSize);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorPreviousSizeWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* wnd = nullptr;
			bool useLazy = false;
		public:
			JEditorPreviousSizeWindowEvStruct(JEditorWindow* wnd, const bool useLazy);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorBindFuncEvStruct : public JEditorEvStruct
		{
		private:
			std::unique_ptr<Core::JBindHandleBase> bindHandle; 
			std::unique_ptr<Core::JLogBase> log;
		public:
			JEditorBindFuncEvStruct(std::unique_ptr<Core::JBindHandleBase> bindHandle, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			void SetLog(std::unique_ptr<Core::JLogBase>newLog);
		public:
			void Execute();
		};

		//support redo undo
		struct JEditorTBindFuncEvStruct : public JEditorEvStruct
		{
		public:
			const std::string taskName;
			const std::string taskDesc;
		public:
			JEditorTBindFuncEvStruct(const std::string& taskName, const std::string& taskDesc, const J_EDITOR_PAGE_TYPE pageType);
		public:
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			virtual void Execute() = 0;
		};

		struct JEditorTSetBindFuncEvStruct : public JEditorTBindFuncEvStruct
		{
		public:
			std::unique_ptr<Core::JBindHandleBase> doBindHandle;
			std::unique_ptr<Core::JBindHandleBase> undoBindHandle;
		public:
			JEditorTSetBindFuncEvStruct(const std::string& taskName,
				const std::string& taskDesc,
				const J_EDITOR_PAGE_TYPE pageType,
				std::unique_ptr<Core::JBindHandleBase> doBindHandle,
				std::unique_ptr<Core::JBindHandleBase> undoBindHandle);
		public:
			bool PassDefectInspection()const noexcept final;
			void Execute() final;
		};

		template<typename DataStructure, typename doHandle, typename undoHandle, bool isMultiCreation>
		struct JEditorTCreateBindFuncEvStruct : public JEditorTBindFuncEvStruct
		{
		public:
			using ProcessBindVec = typename Core::JTransitionTask::ProcessBindVec;
		private:
			ProcessBindVec preprocessDoVec;
			ProcessBindVec postprocessDoVec;
			ProcessBindVec preprocessUndoVec;
			ProcessBindVec postprocessUndoVec;
		public:
			std::unique_ptr<doHandle> doBindHandle;
			std::unique_ptr<undoHandle> undoBindHandle;
			DataStructure& structure; 
		public:
			JEditorTCreateBindFuncEvStruct(const std::string& taskName,
				const std::string& taskDesc,
				const J_EDITOR_PAGE_TYPE pageType,
				std::unique_ptr<doHandle> doBindHandle,
				std::unique_ptr<undoHandle> undoBindHandle,
				DataStructure& structure)
				:JEditorTBindFuncEvStruct(taskName, taskDesc, pageType),
				doBindHandle(std::move(doBindHandle)),
				undoBindHandle(std::move(undoBindHandle)),
				structure(structure)
			{}
		public:
			bool PassDefectInspection()const noexcept final
			{
				return doBindHandle != nullptr && undoBindHandle != nullptr;
			}
		public:
			void RegisterAddtionalProcess(const Core::JTransitionTask::ADDITONAL_PROCESS_TYPE type, ProcessBindVec&& vec)
			{
				if (vec.size() == 0)
					return;

				switch (type)
				{
				case Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_PRE:
				{
					preprocessDoVec = std::move(vec);
					break;
				}
				case Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST:
				{
					postprocessDoVec = std::move(vec);
					break;
				}
				case Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_PRE:
				{
					preprocessUndoVec = std::move(vec);
					break;
				}
				case Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_POST:
				{
					postprocessUndoVec = std::move(vec);
					break;
				}
				default:
					break;
				}
			}
			void Execute() final
			{
				std::unique_ptr<Core::JTransitionTask> task;
				if constexpr(isMultiCreation)
				{
					using JCreationTask = Core::JTransitionMultiCreationTask<DataStructure, doHandle, undoHandle>;
					task = std::make_unique<JCreationTask>(taskName,
						taskDesc,
						std::move(doBindHandle),
						std::move(undoBindHandle),
						structure);
				}
				else
				{
					using JCreationTask = Core::JTransitionCreationTask< DataStructure, doHandle, undoHandle>;
					task = std::make_unique<JCreationTask>(taskName,
						taskDesc,
						std::move(doBindHandle),
						std::move(undoBindHandle),
						structure);
				}

				task->RegisterAddtionalProcess(Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_PRE, std::move(preprocessDoVec));
				task->RegisterAddtionalProcess(Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST, std::move(postprocessDoVec));
				task->RegisterAddtionalProcess(Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_PRE, std::move(preprocessUndoVec));
				task->RegisterAddtionalProcess(Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_POST, std::move(postprocessUndoVec));

				JEditorTransition::Instance().Execute(std::move(task));
			}
		};
	}
}