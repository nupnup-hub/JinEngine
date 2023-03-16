#include"JTransition.h" 
#include"../Log/JLog.h"
#include"../Time/JRealTime.h"
#include"../../Utility/JCommonUtility.h"
#include<stack>

namespace JinEngine
{
	namespace Core
	{
		JTransitionTask::JTransitionTask(const std::string& taskName, const std::string& taskDesc)
			:taskName(taskName), taskDesc(taskDesc)
		{}
		JTransitionTask::~JTransitionTask()
		{
			if (clearTaskB != nullptr)
				clearTaskB->InvokeCompletelyBind();
			preprocessDoVec.clear();
			postprocessDoVec.clear();
			preprocessUndoVec.clear();
			postprocessUndoVec.clear();
		}
		std::string JTransitionTask::GetDoTaskName()const noexcept
		{
			return "Do " + taskName;
		}
		std::string JTransitionTask::GetUndoTaskName()const noexcept
		{
			return "Undo " + taskName;
		}
		std::string JTransitionTask::GetDoTaskDesc()const noexcept
		{
			return "Do " + taskDesc;
		}
		std::string JTransitionTask::GetUndoTaskDesc()const noexcept
		{
			return "Undo " + taskDesc;
		}
		void JTransitionTask::RegisterAddtionalProcess(const ADDITONAL_PROCESS_TYPE type, ProcessBindVec&& vec)
		{
			if (vec.size() == 0)
				return;

			switch (type)
			{
			case ADDITONAL_PROCESS_TYPE::DO_PRE:
			{
				preprocessDoVec = std::move(vec);
				break;
			}
			case ADDITONAL_PROCESS_TYPE::DO_POST:
			{
				postprocessDoVec = std::move(vec);
				break;
			}
			case ADDITONAL_PROCESS_TYPE::UNDO_PRE:
			{
				preprocessUndoVec = std::move(vec);
				break;
			}
			case ADDITONAL_PROCESS_TYPE::UNDO_POST:
			{
				postprocessUndoVec = std::move(vec);
				break;
			}
			default:
				break;
			}
		}
		void JTransitionTask::RegisterClearTask(std::unique_ptr<ClearTaskB>&& clearTaskB)
		{
			JTransitionTask::clearTaskB = std::move(clearTaskB);
		}
		void JTransitionTask::Process(const ADDITONAL_PROCESS_TYPE type)
		{
			std::vector<std::unique_ptr<JBindHandleBase>>* vec = nullptr;
			switch (type)
			{
			case ADDITONAL_PROCESS_TYPE::DO_PRE:
			{
				vec = &preprocessDoVec;
				break;
			}
			case ADDITONAL_PROCESS_TYPE::DO_POST:
			{
				vec = &postprocessDoVec;
				break;
			}
			case ADDITONAL_PROCESS_TYPE::UNDO_PRE:
			{
				vec = &preprocessUndoVec;
				break;
			}
			case ADDITONAL_PROCESS_TYPE::UNDO_POST:
			{
				vec = &postprocessUndoVec;
				break;
			}
			default:
				break;
			}
			const uint count = (uint)vec->size();
			for (uint i = 0; i < count; ++i)
				(*vec)[i]->InvokeCompletelyBind();
		}

		JTransitionSetValueTask::JTransitionSetValueTask(const std::string& taskName,
			const std::string& taskDesc,
			std::unique_ptr<JBindHandleBase>&& doBindhandle,
			std::unique_ptr<JBindHandleBase>&& undoBindhandle)
			: JTransitionTask(taskName, taskDesc),
			doBindhandle(std::move(doBindhandle)),
			undoBindhandle(std::move(undoBindhandle))
		{}
		JTransitionSetValueTask::~JTransitionSetValueTask()
		{
			doBindhandle.reset();
			undoBindhandle.reset();
		}
		void JTransitionSetValueTask::Do()
		{
			Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_PRE);
			doBindhandle->InvokeCompletelyBind();
			Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST);
		}
		void JTransitionSetValueTask::Undo()
		{
			Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_PRE);
			undoBindhandle->InvokeCompletelyBind();
			Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_POST);
		}
		bool JTransitionSetValueTask::IsValid()const noexcept
		{
			return doBindhandle != nullptr && undoBindhandle != nullptr;
		}
 
		void JTransition::Log(Core::JLogBase log)
		{
			logHandler->PushLog(std::make_unique<Core::JLogBase>(log));
		}
		void JTransition::Log(const std::string& title, const std::string& body)
		{
			logHandler->PushLog(std::make_unique<Core::JLogBase>(title, body));
		}
		void JTransition::Log(const std::wstring& title, const std::wstring& body)
		{
			Log(JCUtil::WstrToU8Str(title), JCUtil::WstrToU8Str(body));
		}	
		void JTransition::Execute(std::unique_ptr<JTransitionTask> task)
		{
			if (isLock)
				return;

			ClearUndoQueue();

			Log(task->GetDoTaskName(), task->GetDoTaskDesc());
			doQueue[doIndex] = std::move(task);
			doQueue[doIndex]->Do();

			++doIndex;
			if (doIndex >= taskCapacity)
				doIndex = 0;
		}
		void JTransition::Undo()
		{
			if (isLock)
				return;

			if (undoIndex == invalidIndex)
				undoIndex = doIndex;

			int index = undoIndex - 1;
			if (index < 0)
				index = taskCapacity - 1;

			if (doQueue[index] == nullptr || !doQueue[index]->IsValid())
				return;

			Log(doQueue[index]->GetUndoTaskName(), doQueue[index]->GetUndoTaskDesc());
			doQueue[index]->Undo();
			undoQueue.push_back(std::move(doQueue[index]));
			undoIndex = index;
		}
		void JTransition::Redo()
		{
			if (isLock)
				return;

			if (undoQueue.empty())
				return;

			Log(undoQueue.back()->GetDoTaskName(), undoQueue.back()->GetDoTaskDesc());
			undoQueue.back()->Do();

			doQueue[undoIndex] = std::move(undoQueue.back());
			undoQueue.pop_back();

			++undoIndex;
			if (undoIndex >= taskCapacity)
				undoIndex = 0;
		}
		void JTransition::SetTaskCapacity(const uint8 value)noexcept
		{
			ClearDoQueue();
			ClearUndoQueue();

			taskCapacity = value;
			if (taskCapacity > GetMaxTaskCapacity())
				taskCapacity = GetMaxTaskCapacity();

			doQueue.resize(taskCapacity);
			doIndex = 0;
		}
		void JTransition::SetLock(const bool value)noexcept
		{
			isLock = value;
		}
		std::vector<JTransitonTaskInfo> JTransition::GetTaskInfo()noexcept
		{ 
			int undoIndexStart;
			std::vector<JTransitionTask*> sortedTask = GetSortedTask(undoIndexStart);
			std::vector<JTransitonTaskInfo> taskInfo(sortedTask.size());
			const uint taskCount = (uint)sortedTask.size();

			if (undoIndexStart != invalidIndex)
			{
				for (uint i = 0; i < taskCount; ++i)
				{
					if (i < undoIndexStart)
					{
						taskInfo[i].taskName = sortedTask[i]->GetUndoTaskName();
						taskInfo[i].isUndo = true;
					}
					else
					{
						taskInfo[i].taskName = sortedTask[i]->GetDoTaskName();
						taskInfo[i].isUndo = false;
					}
				}
			}
			else
			{
				for (uint i = 0; i < taskCount; ++i)
				{
					taskInfo[i].taskName = sortedTask[i]->GetDoTaskName();
					taskInfo[i].isUndo = false;
				}
			}
			return taskInfo;
		}
		const JLogHandler* JTransition::GetLogHandler()noexcept
		{
			return logHandler.get();
		}
		void JTransition::Initialize()
		{ 
			doQueue.resize(taskCapacity);
			logHandler = std::make_unique<JLogHandler>();
			logHandler->SetCapacity(GetMaxLogCapacity());
			isLock = false;
		}
		void JTransition::Clear()
		{ 
			doQueue.clear();
			undoQueue.clear();
			logHandler.reset();
			isLock = false;
		}
		std::vector<JTransitionTask*> JTransition::GetSortedTask(_Out_ int& undoTaskStartIndex)noexcept
		{
			const uint doQueueCount = (uint)doQueue.size();
			const uint undoQueueCount = (uint)undoQueue.size();

			std::vector<JTransitionTask*> res;
			const bool isCirculateState = doQueue[doIndex] != nullptr;
			if (isCirculateState)
			{
				res.resize(doQueueCount + undoQueue.size());
				for (uint i = doIndex; i < doQueueCount; ++i)
					res[i - doIndex] = doQueue[i].get();
				uint st = doQueueCount - doIndex;
				for (uint i = 0; i < doIndex; ++i)
					res[i + st] = doQueue[i].get(); 
			}
			else
			{
				res.resize(doIndex + undoQueue.size());
				for (uint i = 0; i < doIndex; ++i)
					res[i] = doQueue[i].get(); 
			}
			if (undoQueueCount > 0)
			{
				for (uint i = 0; i < undoQueueCount; ++i)
					res[i + doQueueCount] = undoQueue[i].get();
				undoTaskStartIndex = doQueueCount;
			}
			else
				undoTaskStartIndex = invalidIndex;
			return res;
		}
		void JTransition::ClearDoQueue()noexcept
		{
			for (uint i = 0; i < taskCapacity; ++i)
				doQueue[i] = nullptr;
		}
		void JTransition::ClearUndoQueue()noexcept
		{
			undoQueue.clear();
			undoIndex = invalidIndex;
		}
	}
}