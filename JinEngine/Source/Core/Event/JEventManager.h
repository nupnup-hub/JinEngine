#pragma once
#include"JEventListener.h"
#include"../JCoreEssential.h"
#include"../Func/Functor/JFunctor.h"
//#include"../Func/Callable/JCallable.h"
#include<unordered_map>
#include<vector>   
  
namespace JinEngine
{
	namespace Core
	{
		template<typename IdentifierType,
			typename EVENTTYPE,
			typename ...Param>
			class JEventInterface
		{
		public:
			using Listener = JEventListener<IdentifierType, EVENTTYPE, Param...>;
			using OnEventPtr = void(Listener::*)(const IdentifierType&, const EVENTTYPE&, Param...);
			using NotifyF = JMFunctorType<Listener, void, const IdentifierType&, const EVENTTYPE&, Param...>;
			using AddEventNotificationF = JMFunctorType<Listener, void, IdentifierType, EVENTTYPE, Param...>;
		private:
			friend Listener;
		public:
			virtual JEventInterface* EvInterface() = 0;
		protected:
			virtual ~JEventInterface() = default;
		protected:
			virtual bool AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const EVENTTYPE& eventType) = 0;
			virtual size_t AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const std::vector<EVENTTYPE>& eventTypeVec) = 0;
			virtual void AddEventNotification(IdentifierType iden, EVENTTYPE eventType, Param... var) = 0;
			virtual void RemoveListener(const IdentifierType& iden) = 0;
			virtual void RemoveEventListener(const IdentifierType& iden, const EVENTTYPE& eventType) = 0;
			virtual void ResetListenerPointer(OnEventPtr ptr, Listener* listener, const IdentifierType& iden) = 0;
			virtual void NotifyEvent(const IdentifierType& iden, const EVENTTYPE& eventType, Param... var) = 0;
		};

		template<typename IdentifierType,
			typename EVENTTYPE,
			typename ...Param>
			class JEventManager : public JEventInterface<IdentifierType, EVENTTYPE, Param...>
		{
		public:
			using Interface = JEventInterface<IdentifierType, EVENTTYPE, Param...>;
			using Listener = typename Interface::Listener;
			using OnEventPtr = typename Interface::OnEventPtr;
			using OnEventFunctor = JFunctor<void, const IdentifierType&, const EVENTTYPE&, Param...>;
			using NotifyFunctor = OnEventFunctor;
			using NotifyBinder = JBindHandle<NotifyFunctor, IdentifierType, EVENTTYPE, Param...>;
			using IdenComparePtr = bool(*)(const IdentifierType&, const IdentifierType&);
			using IdenCompareFunctor = JFunctor<bool, const IdentifierType&, const IdentifierType&>;
		private:
			struct ListenerInfo
			{
			public:
				std::unique_ptr<OnEventFunctor> onEvent;
				IdentifierType iden;
				int listenEventCount = 0;
				bool isValid = true;
			public:
				ListenerInfo(OnEventPtr ptr, Listener* listener, IdentifierType iden)
					:onEvent(std::make_unique<OnEventFunctor>(ptr, listener)), iden(iden)
				{}
				~ListenerInfo() = default;
				ListenerInfo(ListenerInfo&& rhs) = default;
				ListenerInfo& operator=(ListenerInfo&& rhs) = default;
			public:
				void ResetOnEvent(OnEventPtr ptr, Listener* listener)
				{
					onEvent = std::make_unique<OnEventFunctor>(ptr, listener);
				}
			};
		private:
			std::unique_ptr<IdenCompareFunctor> idenCompare;
			NotifyFunctor notifyFunctor;
			std::vector<std::unique_ptr<NotifyBinder>> eventBinderVec;
			std::unordered_map<IdentifierType, std::unique_ptr<ListenerInfo>> listenerDic;
			std::unordered_map<EVENTTYPE, std::vector<ListenerInfo*>> eventDic;
			//std::unordered_map<EVENTTYPE, std::vector<std::unique_ptr<ListenerInfo>>> eventDic;
		private:
			std::vector<std::tuple<OnEventPtr, Listener*, IdentifierType, EVENTTYPE>> addWaitVec;
			std::vector<IdentifierType> remListenerWaitVec;
			std::vector<std::tuple<IdentifierType, EVENTTYPE>> remEvWaitVec;
			std::vector<std::tuple<OnEventPtr, Listener*, IdentifierType>> resetWaitVec;
			bool actNotify = false;
			size_t actListenerGuid = 0;
		protected:
			bool AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const EVENTTYPE& eventType)final
			{
				if (actNotify)
					return AddLoosly(ptr, listener, iden, eventType);
				else
					return AddDirectly(ptr, listener, iden, eventType);
			}
			//Res = success count
			size_t AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const std::vector<EVENTTYPE>& eventTypeVec)final
			{
				size_t sucCount = 0; 
				const uint vecSize = (uint)eventTypeVec.size();
				for (uint i = 0; i < vecSize; ++i)
				{
					if (AddEventListener(ptr, listener, iden, eventTypeVec[i]))
						++sucCount;
				}
				return sucCount;
			}
			void RemoveEventListener(const IdentifierType& iden, const EVENTTYPE& eventType)final
			{ 
				if (actNotify)
					RemoveEventLoosly(iden, eventType);
				else
					RemoveEventDirectly(iden, eventType);
			}
			void RemoveListener(const IdentifierType& iden)final
			{
				if (actNotify)
					RemoveListenerLoosly(iden);
				else
					RemoveListenerDirectly(iden);
			}
			void ResetListenerPointer(OnEventPtr ptr, Listener* listener, const IdentifierType& iden)
			{
				if (actNotify && actListenerGuid == iden)
					ResetLoosly(ptr, listener, iden);
				else
					ResetDirectly(ptr, listener, iden);
			}
			//Push Event Queue
			void AddEventNotification(IdentifierType iden, EVENTTYPE eventType, Param... var)final
			{
				eventBinderVec.emplace_back(std::make_unique<NotifyBinder>(notifyFunctor, std::move(iden), std::move(eventType), std::forward<Param>(var)...));
			}
			//Send Event Notification in Event Queue
			void SendEventNotification()
			{
				const uint binderVecCount = (uint)eventBinderVec.size(); 
				for (uint i = 0; i < binderVecCount; ++i)
					eventBinderVec[i]->InvokeCompletelyBind();

				//eventBinderVec에서 새로운 event를 add할수 있으므로
				//Clear는 하지않는다.
				uint invokeEndCount = binderVecCount;
				uint nowVecCount = eventBinderVec.size(); 
				while (invokeEndCount < nowVecCount)
				{
					for (uint i = invokeEndCount; i < nowVecCount; ++i)
						eventBinderVec[i]->InvokeCompletelyBind();
					invokeEndCount = nowVecCount;
					nowVecCount = (uint)eventBinderVec.size();
				}

				eventBinderVec.clear();
			}
			void NotifyEvent(const IdentifierType& iden, const EVENTTYPE& eventType, Param... as)final
			{ 
				actNotify = true;
				auto vec = eventDic.find(eventType);
				if (vec != eventDic.end())
				{
					for (auto& data : vec->second)
					{
						if (data->isValid)
						{
							actListenerGuid = data->iden;
							data->onEvent->Invoke(iden, eventType, std::forward<Param>(as)...);
						}
					}
				}
				actListenerGuid = 0;
				actNotify = false; 
				ExecuteWaitQuque();
			}
			void ClearEvent()
			{
				listenerDic.clear();
				eventDic.clear();
				eventBinderVec.clear();
			}
		private:
			bool AddDirectly(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const EVENTTYPE& eventType)
			{
				auto listenerData = listenerDic.find(iden);
				if (listenerData == listenerDic.end())
				{
					listenerDic.emplace(iden, std::make_unique<ListenerInfo>(ptr, listener, iden));
					listenerData = listenerDic.find(iden);
				}

				auto vec = eventDic.find(eventType);
				if (vec == eventDic.end())
				{
					eventDic.emplace(eventType, std::vector<ListenerInfo*>());
					vec = eventDic.find(eventType);
				}
				else
				{
					for (const auto& data : vec->second)
					{
						if ((*idenCompare)(data->iden, iden))
							return false;
					}
				}
				 
				vec->second.push_back(listenerData->second.get());
				++listenerData->second->listenEventCount;
				return true;
			}
			bool AddLoosly(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const EVENTTYPE& eventType)
			{
				addWaitVec.push_back(std::tuple(ptr, listener, iden, eventType));
				return true;
			}
			void RemoveEventDirectly(const IdentifierType& iden, const EVENTTYPE& eventType)
			{
				auto vec = eventDic.find(eventType);
				if (vec != eventDic.end())
				{
					const uint vecCount = (uint)vec->second.size();
					for (uint i = 0; i < vecCount; ++i)
					{
						if ((*idenCompare)(vec->second[i]->iden, iden))
						{
							--vec->second[i]->listenEventCount;
							vec->second.erase(vec->second.begin() + i);
							if (vec->second[i]->listenEventCount <= 0)
								listenerDic.erase(iden);
							return;
						}
					}
				}
			}
			void RemoveEventLoosly(const IdentifierType& iden, const EVENTTYPE& eventType)
			{ 
				remEvWaitVec.push_back(std::tuple(iden, eventType));
			}
			void RemoveListenerDirectly(const IdentifierType& iden)
			{
				for (auto& data : eventDic)
				{
					const uint vecSize = (uint)data.second.size();
					for (uint i = 0; i < vecSize; ++i)
					{
						if ((*idenCompare)(data.second[i]->iden, iden))
						{
							--data.second[i]->listenEventCount;
							data.second.erase(data.second.begin() + i);
							break;
						}
					}
				}
				listenerDic.erase(iden);
			}
			void RemoveListenerLoosly(const IdentifierType& iden)
			{
				auto data = listenerDic.find(iden);
				if(data != listenerDic.end())
				{
					data->second->isValid = false;
					remListenerWaitVec.push_back(iden);
				}
			}
			void ResetDirectly(OnEventPtr ptr, Listener* listener, const IdentifierType& iden)
			{
				auto data = listenerDic.find(iden);
				if(data != listenerDic.end())
					data->second->ResetOnEvent(ptr, listener);
			}
			void ResetLoosly(OnEventPtr ptr, Listener* listener, const IdentifierType& iden)
			{
				auto data = listenerDic.find(iden);
				if (data != listenerDic.end())
				{
					data->second->isValid = false;
					resetWaitVec.push_back(std::tuple(ptr, listener, iden));
				}
			}
		private:
			void ExecuteWaitQuque()
			{ 
				const uint resetCount = (uint)resetWaitVec.size();
				for (uint i = 0; i < resetCount; ++i)
					ResetDirectly(std::get<0>(resetWaitVec[i]), std::get<1>(resetWaitVec[i]), std::get<2>(resetWaitVec[i]));

				const uint addCount = (uint)addWaitVec.size();
				for (uint i = 0; i < addCount; ++i)
					AddDirectly(std::get<0>(addWaitVec[i]), std::get<1>(addWaitVec[i]), std::get<2>(addWaitVec[i]), std::get<3>(addWaitVec[i]));
				
				const uint remEvCount = (uint)remEvWaitVec.size();
				for (uint i = 0; i < remEvCount; ++i)
					RemoveEventDirectly(std::get<0>(remEvWaitVec[i]), std::get<1>(remEvWaitVec[i]));

				const uint remCount = (uint)remListenerWaitVec.size();
				for (uint i = 0; i < remCount; ++i)
					RemoveListenerDirectly(remListenerWaitVec[i]);

				resetWaitVec.clear();
				addWaitVec.clear();
				remEvWaitVec.clear();
				remListenerWaitVec.clear();
			}
		protected:
			JEventManager(IdenComparePtr ptr)
				:idenCompare(std::make_unique<IdenCompareFunctor>(ptr)),
				notifyFunctor(NotifyFunctor(&JEventManager::NotifyEvent, this))
			{ } 
		};
	}
}
