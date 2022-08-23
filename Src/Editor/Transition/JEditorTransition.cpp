#include"JEditorTransition.h" 

namespace JinEngine
{
	namespace Editor
	{
		template<typename OriFunctor, typename ...BindParam>
		JEditorTask::JEditorTask(Core::JBindHandle<OriFunctor, BindParam...>& bindhanle, const std::string& taskName)
			:bindhanle(std::make_unique<Core::JBindHandle<OriFunctor, BindParam...>>(bindhanle)), taskName(taskName)
		{}
		JEditorTask::~JEditorTask()
		{

		}
		JEditorTask::JEditorTask(JEditorTask&& rhs)
		{
			bindhanle = std::move(rhs.bindhanle);
			taskName = std::move(rhs.taskName);
		}
		JEditorTask& JEditorTask::operator=(JEditorTask&& rhs)
		{
			bindhanle = std::move(rhs.bindhanle);
			taskName = std::move(rhs.taskName);
			return *this;
		}
		void JEditorTask::Clear()
		{
			bindhanle.reset();
			taskName.clear();
		}

		void JEditorTaskSet::Push(JEditorTask&& doExe, JEditorTask&& undoExe)
		{
			doExe = std::move(doExe);
			undoExe = std::move(undoExe);
			isValid = true;
		}
		void JEditorTaskSet::Clear()
		{
			doExe.Clear();
			undoExe.Clear();
			isValid = false;
		}

		namespace
		{
			static std::deque<JEditorTaskSet> taskSetQueue;
			static std::deque<std::string> taskLogQueue;

			static uint8 doIndex = 0;
			static uint8 undoIndex = 0;
			static uint8 logIndex = 0;
			static uint8 taskSetCapacity = 20;
			static uint logCapacity = 40;
		}
		void JEditorTransition::Log(const std::string& str)
		{
			taskLogQueue[logIndex] = str;
			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;
		}
		void JEditorTransition::Execute(JEditorTask&& doExe, JEditorTask&& undoExe)
		{
			if (doIndex != undoIndex)
			{
				if (doIndex < undoIndex)
				{
					while (undoIndex != taskSetCapacity)
					{
						taskSetQueue[undoIndex].Clear();
						++undoIndex;
					}
					undoIndex = 0;
				}
				while (undoIndex != doIndex)
				{
					taskSetQueue[undoIndex].Clear();
					++undoIndex;
				}
			}

			taskLogQueue[logIndex] = doExe.taskName;
			taskSetQueue[doIndex].Push(std::move(doExe), std::move(undoExe));
			taskSetQueue[doIndex].doExe.bindhanle->InvokeCompletelyBind();

			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;

			++doIndex;
			undoIndex = doIndex;
			if (doIndex >= taskSetCapacity)
				doIndex = undoIndex = 0;	
		}
		void JEditorTransition::Undo()
		{
			if (undoIndex == (doIndex + 1) % taskSetCapacity)
				return;

			if (!taskSetQueue[undoIndex].isValid)
				return;

			taskLogQueue[logIndex] = taskSetQueue[undoIndex].undoExe.taskName; 
			taskSetQueue[undoIndex].undoExe.bindhanle->InvokeCompletelyBind();

			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;

			--undoIndex;
			if (undoIndex < 0)
				undoIndex = taskSetCapacity - 1;
		}
		void JEditorTransition::Redo()
		{
			if (undoIndex == doIndex)
				return;

			++undoIndex;
			if (undoIndex >= taskSetCapacity)
				undoIndex = 0;

			taskLogQueue[logIndex] = taskSetQueue[undoIndex].undoExe.taskName;
			taskSetQueue[undoIndex].doExe.bindhanle->InvokeCompletelyBind();

			++logIndex;
			if (logIndex >= logCapacity)
				logIndex = 0;
		}
		void JEditorTransition::SetTaskCapacity(const uint8 value)noexcept
		{
			for (uint i = 0; i < taskSetCapacity; ++i)
				taskSetQueue[i].Clear();

			taskSetCapacity = value;
			taskSetQueue.resize(taskSetCapacity);

			doIndex = 0;
			undoIndex = 0;
		}
		void JEditorTransition::SetLogCapacity(const uint value)noexcept
		{
			logCapacity = value;
			taskLogQueue.resize(logCapacity);
		}
		const std::deque<std::string>& JEditorTransition::GetLog()noexcept
		{
			return taskLogQueue;
		}
		void JEditorTransition::Initialize()
		{
			taskSetQueue.resize(taskSetCapacity);
			taskLogQueue.resize(logCapacity);
		}
		void JEditorTransition::Clear()
		{
			taskSetQueue.clear();
			taskLogQueue.clear();
		}
	}
}