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


#include"JTransition.h"  
#include"../Time/JRealTime.h"
#include"../Utility/JCommonUtility.h"
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
 
		JTransition::TransitionTaskSet::TransitionTaskSet(std::unique_ptr<JTransitionTask> && task, const uint frameCount)
			:task(std::move(task)), frameCount(frameCount)
		{}

		JTransition::JTransition(const std::string& name)
			:name(name)
		{}
		void JTransition::Update()
		{
			++frame;
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
		void JTransition::Execute(std::unique_ptr<JTransitionTask>&& task)
		{
			if (isLock)
				return;

			ClearUndoQueue();

			Log(task->GetDoTaskName(), task->GetDoTaskDesc());
			doQueue[doCount] = std::make_unique<TransitionTaskSet>(std::move(task), frame);
			doQueue[doCount]->task->Do();

			++doCount;
			if (doCount >= taskCapacity)
				doCount = 0;
		}
		void JTransition::Undo()
		{
			int index = 0;
			if (!UndoCommonProcess(index))
				return;

			Log(doQueue[index]->task->GetUndoTaskName(), doQueue[index]->task->GetUndoTaskDesc());
			doQueue[index]->task->Undo();
			undoQueue.push_back(std::move(doQueue[index]));
			undoIndex = index;
		}
		void JTransition::UndoPerFrame()
		{
			int index = 0;
			if (!UndoCommonProcess(index))
				return;

			const uint frameCount = doQueue[index]->frameCount;
			while (doQueue[index] != nullptr && doQueue[index]->task->IsValid() && frameCount == doQueue[index]->frameCount)
			{
				Undo(); 
				index = undoIndex - 1;
				if (index < 0)
					index = taskCapacity - 1;
			}
		}
		void JTransition::Redo()
		{
			if (isLock || undoQueue.empty())
				return;

			Log(undoQueue.back()->task->GetDoTaskName(), undoQueue.back()->task->GetDoTaskDesc());
			undoQueue.back()->task->Do();

			doQueue[undoIndex] = std::move(undoQueue.back());
			undoQueue.pop_back();

			++undoIndex;
			if (undoIndex >= taskCapacity)
				undoIndex = 0;
		} 
		void JTransition::RedoPerFrame()
		{
			if (isLock || undoQueue.empty())
				return; 

			const uint frameCount = undoQueue.back()->frameCount;
			while (undoQueue.size() > 0 && frameCount == undoQueue.back()->frameCount)
				Redo();
		}
		bool JTransition::UndoCommonProcess(_Inout_ int& index)
		{ 
			if (isLock)
				return false;

			if (undoIndex == invalidIndex)
				undoIndex = doCount;

			index = undoIndex - 1;
			if (index < 0)
				index = taskCapacity - 1;

			if (doQueue[index] == nullptr || !doQueue[index]->task->IsValid())
				return false;
			return true;
		}
		std::vector<JTransitonTaskInfo> JTransition::GetTaskInfo()noexcept
		{
			int undoIndexStart;
			std::vector<TransitionTaskSet*> sortedTask = GetSortedTask(undoIndexStart);
			std::vector<JTransitonTaskInfo> taskInfo(sortedTask.size());
			const uint taskCount = (uint)sortedTask.size();

			if (undoIndexStart != invalidIndex)
			{
				for (uint i = 0; i < taskCount; ++i)
				{
					if (i < undoIndexStart)
					{
						taskInfo[i].taskName = sortedTask[i]->task->GetUndoTaskName();
						taskInfo[i].isUndo = true;
					}
					else
					{
						taskInfo[i].taskName = sortedTask[i]->task->GetDoTaskName();
						taskInfo[i].isUndo = false;
					}
				}
			}
			else
			{
				for (uint i = 0; i < taskCount; ++i)
				{
					taskInfo[i].taskName = sortedTask[i]->task->GetDoTaskName();
					taskInfo[i].isUndo = false;
				}
			}
			return taskInfo;
		}
		void JTransition::SetTaskCapacity(const uint8 value)noexcept
		{
			ClearDoQueue();
			ClearUndoQueue();

			taskCapacity = value;
			if (taskCapacity > GetMaxTaskCapacity())
				taskCapacity = GetMaxTaskCapacity();

			doQueue.resize(taskCapacity);
			doCount = 0;
		}
		void JTransition::SetLock(const bool value)noexcept
		{
			isLock = value;
		}
		void JTransition::Initialize()
		{ 
			doQueue.resize(taskCapacity);
			logHandler = std::make_unique<JPublicLogHolder>(name);
			
			JLogHolderInterface::LogHolderOption option;
			option.capacity = GetMaxLogCapacity();

			logHandler->SetOption(option); 
			isLock = false;
		}
		void JTransition::Clear()
		{ 
			doQueue.clear();
			undoQueue.clear();
			logHandler.reset();
			frame = 0;
			isLock = false;
		}
		std::vector<JTransition::TransitionTaskSet*> JTransition::GetSortedTask(_Out_ int& undoTaskStartIndex)noexcept
		{
			const uint doQueueCount = (uint)doQueue.size();
			const uint undoQueueCount = (uint)undoQueue.size();

			std::vector<TransitionTaskSet*> res;
			const bool isCirculateState = doQueue[doCount] != nullptr;
			if (isCirculateState)
			{
				res.resize(doQueueCount + undoQueue.size());
				for (uint i = doCount; i < doQueueCount; ++i)
					res[i - doCount] = doQueue[i].get();
				uint st = doQueueCount - doCount;
				for (uint i = 0; i < doCount; ++i)
					res[i + st] = doQueue[i].get(); 
			}
			else
			{
				res.resize(doCount + undoQueue.size());
				for (uint i = 0; i < doCount; ++i)
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