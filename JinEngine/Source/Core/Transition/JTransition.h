#pragma once
#include"../Log/JLogHandler.h"
#include"../Func/Functor/JFunctor.h"
#include"../Handle/JDataHandleStructure.h"
#include<deque> 

namespace JinEngine
{
	namespace Core
	{
		class JTransitionTask
		{
		public:
			enum class ADDITONAL_PROCESS_TYPE
			{
				DO_PRE,
				DO_POST,
				UNDO_PRE,
				UNDO_POST
			};
		public:
			using ProcessBindVec = std::vector<std::unique_ptr<JBindHandleBase>>;
			using ClearTaskB = Core::JBindHandleBase;
		private:
			ProcessBindVec preprocessDoVec;
			ProcessBindVec postprocessDoVec;
			ProcessBindVec preprocessUndoVec;
			ProcessBindVec postprocessUndoVec;
		private:
			std::unique_ptr<ClearTaskB> clearTaskB;
		private: 
			std::string taskName;
			std::string taskDesc;
		public: 
			JTransitionTask(const std::string& taskName, const std::string& taskDesc);
			virtual ~JTransitionTask();
		public:
			std::string GetDoTaskName()const noexcept;
			std::string GetUndoTaskName()const noexcept;
			std::string GetDoTaskDesc()const noexcept;
			std::string GetUndoTaskDesc()const noexcept;
		public:
			void RegisterAddtionalProcess(const ADDITONAL_PROCESS_TYPE type, ProcessBindVec&& vec);
			void RegisterClearTask(std::unique_ptr<ClearTaskB>&& clearTaskB);
		public:
			virtual void Do() = 0;
			virtual void Undo() = 0; 
		protected:
			void Process(const ADDITONAL_PROCESS_TYPE type);
		public:
			virtual bool IsValid()const noexcept = 0;
		};
		 
		class JTransitionSetValueTask final : public JTransitionTask
		{
		private:
			std::unique_ptr<JBindHandleBase> doBindhandle;
			std::unique_ptr<JBindHandleBase> undoBindhandle;
		public:  
			JTransitionSetValueTask(const std::string& taskName,
				const std::string& taskDesc,
				std::unique_ptr<JBindHandleBase>&& doBindhandle,
				std::unique_ptr<JBindHandleBase>&& undoBindhandle);
			~JTransitionSetValueTask();
		public:
			void Do() final;
			void Undo() final; 
		public:
			bool IsValid()const noexcept;
		};
 
		template<typename DataStructure, typename DoHandle, typename UndoHandle>
		class JTransitionCreationBase : public JTransitionTask
		{
		protected:
			std::unique_ptr<DoHandle> doBindhandle = nullptr;
			std::unique_ptr<UndoHandle> undoBindhandle = nullptr;
			DataStructure& dataStructure; 
		public:
			JTransitionCreationBase(const std::string& taskName,
				const std::string& taskDesc,
				std::unique_ptr<DoHandle> doBindhandle,
				std::unique_ptr<UndoHandle> undoBindhandle,
				DataStructure& dataStructure)
				:JTransitionTask(taskName, taskDesc),
				doBindhandle(std::move(doBindhandle)),
				undoBindhandle(std::move(undoBindhandle)),
				dataStructure(dataStructure)
			{}
			~JTransitionCreationBase()
			{ 
				doBindhandle.reset();
				undoBindhandle.reset();
			}	
		public:
			bool IsValid()const noexcept
			{
				return doBindhandle != nullptr && undoBindhandle != nullptr;
			}
		};
		 
		template<typename DataStructure,  typename DoHandle, typename UndoHandle>
		class JTransitionCreationTask final : public JTransitionCreationBase< DataStructure, DoHandle, UndoHandle>
		{
		private:
			using Base = JTransitionCreationBase< DataStructure, DoHandle, UndoHandle>;
		private: 
			JDataHandle dataHandle;
		public: 
			JTransitionCreationTask(const std::string& taskName,
				const std::string& taskDesc,
				std::unique_ptr<DoHandle> doBindhandle,
				std::unique_ptr<UndoHandle> undoBindhandle,
				DataStructure& dataStructure)
				:Base(taskName, taskDesc, std::move(doBindhandle), std::move(undoBindhandle), dataStructure),
				dataHandle(dataStructure.CreateInvalidHandle())
			{}
			~JTransitionCreationTask()
			{
				if (dataHandle.IsValid())
					Base::dataStructure.Remove(dataHandle);
			}
		public:
			void Do() final
			{
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_PRE);
				Base::doBindhandle->Invoke<DataStructure&, JDataHandle&>(Base::dataStructure, dataHandle);
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST);
			}
			void Undo() final
			{
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_PRE);
				Base::undoBindhandle->Invoke<DataStructure&, JDataHandle&>(Base::dataStructure, dataHandle);
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_POST);
			}
		};
		
		template<typename DataStructure, typename DoHandle, typename UndoHandle>
		class JTransitionMultiCreationTask final : public JTransitionCreationBase< DataStructure, DoHandle, UndoHandle>
		{
		private:
			using Base = JTransitionCreationBase< DataStructure, DoHandle, UndoHandle>;
		private: 
			std::vector<JDataHandle> dataHandleVec;
		public:
			JTransitionMultiCreationTask(const std::string& taskName,
				const std::string& taskDesc,
				std::unique_ptr<DoHandle> doBindhandle,
				std::unique_ptr<UndoHandle> undoBindhandle,
				DataStructure& dataStructure)
				:Base(taskName, taskDesc, std::move(doBindhandle), std::move(undoBindhandle), dataStructure)
			{}
			~JTransitionMultiCreationTask()
			{
				for (auto& data : dataHandleVec)
				{
					if (data.IsValid())
						Base::dataStructure.Remove(data);
				}
				dataHandleVec.clear();
			}
		public:
			void Do() final
			{
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_PRE);
				Base::doBindhandle->Invoke<DataStructure&, std::vector<JDataHandle>&>(Base::dataStructure, dataHandleVec);
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST);
			}
			void Undo() final
			{
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_PRE);
				Base::undoBindhandle->Invoke<DataStructure&, std::vector<JDataHandle>&>(Base::dataStructure, dataHandleVec);
				Base::Process(JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_POST);
			}
		};

		template<typename DataStructure, typename ...Param>
		struct JTransitionCreationHandleType
		{ 
		public:
			using Functor = Core::JFunctor<void, DataStructure&, Core::JDataHandle&, Param...>;
			using Bind = Core::JBindHandle<Functor, const Core::JEmptyType&, const Core::JEmptyType&, Param...>;
		};
		template<typename DataStructure, typename ...Param>
		struct JTransitionMultiCreationHandleType
		{
		public:
			using Functor = Core::JFunctor<void, DataStructure&, std::vector<Core::JDataHandle>&, Param...>;
			using Bind = Core::JBindHandle<Functor, const Core::JEmptyType&, const Core::JEmptyType&, Param...>;
		};

		//Make transition create task bind unq
		template<typename BindType, typename Functor, typename ...Param>
		static std::unique_ptr<BindType> CTaskUptr(Functor& functor, Param... var)
		{
			return std::make_unique<BindType>(functor, empty, empty, std::forward<Param>(var)...);
		}
		 
		class JTransitonTaskInfo
		{
		public:
			std::string taskName;
			bool isUndo;
		};

		struct JTransitionData; 
		class JTransition
		{  
		private:
			struct TransitionTaskSet
			{
			public:
				std::unique_ptr<JTransitionTask> task;
				const uint frameCount = 0;
			public:
				TransitionTaskSet(std::unique_ptr<JTransitionTask>&& task, const uint frameCount);
			};
		private:
			std::string name;
		private:
			std::deque<std::unique_ptr<TransitionTaskSet>> doQueue;
			std::deque<std::unique_ptr<TransitionTaskSet>> undoQueue;
			//static std::stack<std::unique_ptr<JTransitionTask>> undoStack;
			std::unique_ptr<JPublicLogHolder> logHandler;
		private:
			int doCount = 0;
			int undoIndex = invalidIndex;
			uint8 taskCapacity = 25;
			size_t frame = 0;
			bool isLock = false;
		public:
			JTransition(const std::string& name);
		public:
			void Update();
		public:
			void Log(Core::JLogBase log);
			void Log(const std::string& title, const std::string& body = "");
			void Log(const std::wstring& title, const std::wstring& body = L"");
			void Execute(std::unique_ptr<JTransitionTask>&& task);
			void Undo();
			void UndoPerFrame();
			void Redo(); 
			void RedoPerFrame();
		private:
			bool UndoCommonProcess(_Inout_ int& index);
		public:
			static constexpr uint8 GetMaxTaskCapacity() noexcept
			{
				return 50;
			}
			static constexpr uint GetMaxLogCapacity() noexcept
			{
				return 50;
			} 
			std::vector<JTransitonTaskInfo> GetTaskInfo()noexcept; 
		public:
			void SetTaskCapacity(const uint8 value)noexcept;
			void SetLock(const bool value)noexcept;
		public:
			void Initialize();
			void Clear();
		private:
			std::vector<TransitionTaskSet*> GetSortedTask(_Out_ int& undoTaskStartIndex)noexcept;
		private:
			void ClearDoQueue()noexcept;
			void ClearUndoQueue()noexcept;
		};
	}
}