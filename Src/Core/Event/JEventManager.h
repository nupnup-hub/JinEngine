#pragma once
#include"JEventListener.h"
#include"../JDataType.h"
#include"../Func/Functor/JFunctor.h"
#include"../Func/Callable/JCallable.h"
#include<unordered_map>
#include<vector>  

namespace JinEngine
{
	namespace Core
	{
		template<typename BaseClass,
			typename IdentifierType,
			typename EVENTTYPE,
			typename Ret,
			typename ...Param>
		class JEventInterface : public BaseClass
		{
		protected:
			using Listener = JEventListener<IdentifierType, EVENTTYPE, Param...>;
			using OnEventPtr = Ret(Listener::*)(const IdentifierType&, const EVENTTYPE&, Param...);
		private:
			friend Listener;
		public:
			virtual JEventInterface* EvInterface()noexcept = 0;
		protected:
			virtual bool AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const EVENTTYPE& eventType) = 0;
			virtual size_t AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const std::vector<EVENTTYPE>& eventTypeVec) = 0;
			virtual void NotifyEvent(const IdentifierType& iden, const EVENTTYPE& eventType, Param... var) = 0;
			virtual void EraseListener(const IdentifierType& iden) = 0;
			virtual void EraseListenerEvent(const IdentifierType& iden, const EVENTTYPE& eventType) = 0;
		};

		template<typename BaseClass,
			typename IdentifierType,
			typename EVENTTYPE,
			typename Ret,
			typename ...Param>
		class JEventManager : public JEventInterface< BaseClass, IdentifierType, EVENTTYPE, Ret, Param...>
		{
		public:
			using Interface = JEventInterface< BaseClass, IdentifierType, EVENTTYPE, Ret, Param...>;
			using Listener = typename Interface::Listener;
			using OnEventPtr = typename Interface::OnEventPtr;
			using OnEventFunctor = JFunctor<Ret, const IdentifierType&, const EVENTTYPE&, Param...>;
			using IdenComparePtr = bool(*)(const IdentifierType&, const IdentifierType&);
			using IdenCompareCallable = JStaticCallable<bool, const IdentifierType&, const IdentifierType&>;
		private:
			struct ListenerInfo
			{
			public:
				std::unique_ptr<OnEventFunctor> onEvent;
				const IdentifierType iden;
			public:
				ListenerInfo(OnEventPtr ptr, Listener* listener, const IdentifierType& iden)
					:onEvent(std::make_unique<OnEventFunctor>(ptr, listener)), iden(iden)
				{}
				~ListenerInfo() = default;
				ListenerInfo(ListenerInfo&& rhs) = default;
				ListenerInfo& operator=(ListenerInfo&& rhs) = default;
			};
		private:
			IdenCompareCallable* idenCompare;
			std::unordered_map<EVENTTYPE, std::vector<std::unique_ptr<ListenerInfo>>> eventDic;
		protected:
			bool AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const EVENTTYPE& eventType)final
			{
				auto vec = eventDic.find(eventType);
				if (vec == eventDic.end())
				{
					eventDic.emplace(eventType, std::vector<std::unique_ptr<ListenerInfo>>());
					vec = eventDic.find(eventType);
				}
				else
				{
					for (const auto& data : vec->second)
					{
						if ((*idenCompare)(nullptr, data->iden, iden))
							return false;
					}
				}
				vec->second.emplace_back(std::make_unique<ListenerInfo>(ptr, listener, iden));
				return true;
			}

			//Res = added 1 << (0..n -1) if fail add 
			size_t AddEventListener(OnEventPtr ptr, Listener* listener, const IdentifierType& iden, const std::vector<EVENTTYPE>& eventTypeVec)final
			{
				size_t resBit = 0;
				size_t factor = 1;
				const uint vecSize = (uint)eventTypeVec.size();
				for (uint i = 0; i < vecSize; ++i)
				{
					if (!AddEventListener(ptr, listener, iden, eventTypeVec[i]))
						resBit += factor << i;
				}
				return resBit;
			}
			void NotifyEvent(const IdentifierType& iden, const EVENTTYPE& eventType, Param... as)final
			{
				auto vec = eventDic.find(eventType);
				if (vec == eventDic.end())
					return;

				for (auto& data : vec->second)
					data->onEvent->Invoke(iden, eventType, std::forward<Param>(as)...);
			}
			void EraseListener(const IdentifierType& iden)final
			{
				for (auto& data : eventDic)
				{
					const uint vecSize = (uint)data.second.size();
					for (uint i = 0; i < vecSize; ++i)
					{
						if ((*idenCompare)(nullptr, data.second[i]->iden, iden))
						{
							data.second[i].release();
							data.second.erase(data.second.begin() + i);
							break;
						}
					}
				}
			}
			void EraseListenerEvent(const IdentifierType& iden, const EVENTTYPE& eventType)final
			{
				auto vec = eventDic.find(eventType);
				for (uint i = 0; i < vec->second.size(); ++i)
				{
					if ((*idenCompare)(nullptr, vec->second[i]->iden, iden))
					{
						vec->second[i].release();
						vec->second.erase(vec->second.begin() + i);
						return;
					}
				}
			}
			void RegistIdenCompareCallable(IdenComparePtr ptr)
			{
				idenCompare = new IdenCompareCallable(ptr);
			}
			void ClearEvent()
			{
				delete idenCompare;
				eventDic.clear();
			}
		};
	}
}
