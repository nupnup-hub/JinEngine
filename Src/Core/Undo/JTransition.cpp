#include"JTransition.h" 
#include<stack>

namespace JinEngine
{
	namespace Core
	{
		JTransitionTask::JTransitionTask(const std::string& doTaskName, const std::string& undoTaskName)
			:doTaskName(doTaskName), undoTaskName(undoTaskName)
		{}
		JTransitionTask::~JTransitionTask()
		{ }
		std::string JTransitionTask::GetDoTaskName()const noexcept
		{
			return doTaskName;
		}
		std::string JTransitionTask::GetUndoTaskName()const noexcept
		{
			return undoTaskName;
		}

		JTransitionSetValueTask::JTransitionSetValueTask(const std::string& taskName,
			std::unique_ptr<Core::JBindHandleBase> doBindhanle,
			std::unique_ptr<Core::JBindHandleBase> undoBindhanle)
			:JTransitionTask("Do" + taskName, "Undo" + taskName),
			doBindhanle(std::move(doBindhanle)),
			undoBindhanle(std::move(undoBindhanle))
		{}
		void JTransitionSetValueTask::Do()
		{
			doBindhanle->InvokeCompletelyBind();
		}
		void JTransitionSetValueTask::Undo()
		{
			undoBindhanle->InvokeCompletelyBind();
		}
		void JTransitionSetValueTask::Clear()noexcept
		{
			doBindhanle.reset();
			undoBindhanle.reset();
		}
		bool JTransitionSetValueTask::IsValid()const noexcept
		{
			return doBindhanle != nullptr && undoBindhanle != nullptr;
		}

		namespace
		{
			static std::deque<std::unique_ptr<JTransitionTask>> doQueue;
			static std::stack<std::unique_ptr<JTransitionTask>> undoStack;
			static std::deque<std::string> logQueue;

			static int deactUndoIndex = -1;
			static int doIndex = 0;
			static int undoIndex = deactUndoIndex;
			static int logIndex = 0;
			static uint8 taskCapacity = 20;
			static uint logCapacity = 40;

			static void ClearDoQueue()
			{
				for (uint i = 0; i < taskCapacity; ++i)
					doQueue[i]->Clear();
			}
			static void ClearUndoStack()
			{
				if (!undoStack.empty())
				{
					while (!undoStack.empty())
					{
						undoStack.top()->Clear();
						undoStack.pop();
					}
				}
				undoIndex = deactUndoIndex;
			}
			//For Debug
			/*
			static void PrintQueueCondition()
			{
				#include<iostream>
								int validCount = 0;
				for (int i = 0; i < doQueue.size(); ++i)
					if (doQueue[i] != nullptr)
						++validCount;

				std::cout << "DoCount: " << validCount << std::endl;
				std::cout << "Undo: " << undoStack.size() << std::endl;

				std::cout << "doIndex: " << doIndex << std::endl;
				std::cout << "undoIndex: " << undoIndex << std::endl;
				std::cout << std::endl;
			}
			*/
		}

		void JTransition::Log(const std::string& str)
		{
			logQueue[logIndex] = str;
			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;
		}
		void JTransition::Execute(std::unique_ptr<JTransitionTask> task)
		{
			ClearUndoStack();

			logQueue[logIndex] = task->GetDoTaskName();
			doQueue[doIndex] = std::move(task);
			doQueue[doIndex]->Do();

			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;

			++doIndex;
			if (doIndex >= taskCapacity)
				doIndex = 0;
		}
		void JTransition::Undo()
		{
			if (undoIndex == deactUndoIndex)
				undoIndex = doIndex;

			int index = undoIndex - 1;
			if (index < 0)
				index = taskCapacity - 1;

			if (doQueue[index] == nullptr || !doQueue[index]->IsValid())
				return;

			logQueue[logIndex] = doQueue[index]->GetUndoTaskName();
			doQueue[index]->Undo();
			undoStack.push(std::move(doQueue[index]));

			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;

			undoIndex = index;
		}
		void JTransition::Redo()
		{
			if (undoStack.empty())
				return;

			logQueue[logIndex] = undoStack.top()->GetDoTaskName();
			undoStack.top()->Do();

			doQueue[undoIndex] = std::move(undoStack.top());
			undoStack.pop();

			++undoIndex;
			if (undoIndex >= taskCapacity)
				undoIndex = 0;

			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;
		}
		void JTransition::SetTaskCapacity(const uint8 value)noexcept
		{
			ClearDoQueue();
			ClearUndoStack();

			taskCapacity = value;
			if (taskCapacity > GetMaxTaskCapacity())
				taskCapacity = GetMaxTaskCapacity();

			doQueue.resize(taskCapacity);
			doIndex = 0;
		}
		void JTransition::SetLogCapacity(const uint value)noexcept
		{
			logCapacity = value;
			if (logCapacity > GetMaxLogCapacity())
				logCapacity = GetMaxLogCapacity();

			logQueue.resize(logCapacity);
		}
		const std::deque<std::string>& JTransition::GetLog()noexcept
		{
			return logQueue;
		}
		void JTransition::Initialize()
		{
			doQueue.resize(taskCapacity);
			logQueue.resize(logCapacity);
		}
		void JTransition::Clear()
		{
			doQueue.clear();
			logQueue.clear();
		}
	}
}