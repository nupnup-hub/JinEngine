#pragma once  
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename IdentifierType,
			typename EVENTTYPE,
			typename ...Param>
			class JEventInterface;

		template<typename IdentifierType, typename EVENTTYPE, typename ...Param>
		class JEventListener
		{	
		public:
			using Listener = JEventListener<IdentifierType, EVENTTYPE, Param...>;
			using EvInterface = typename JEventInterface< IdentifierType, EVENTTYPE, Param...>;
			using NotifyPtr = void(Listener::*)(EvInterface&, const IdentifierType&, const EVENTTYPE&, Param...);
			using AddEventNotificationPtr = void(Listener::*)(EvInterface&, const IdentifierType&, const EVENTTYPE&, Param...);
		public:
			virtual ~JEventListener(){}
		protected: 
			virtual void OnEvent(const IdentifierType& iden, const EVENTTYPE& eventType, Param... var) = 0;
		protected:  
			bool AddEventListener(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType)
			{ 
				return evInterface.AddEventListener(&JEventListener::OnEvent, this, iden, eventType);
			} 
			size_t AddEventListener(EvInterface& evInterface, const IdentifierType& iden, const std::vector<EVENTTYPE>& eventTypeVec)
			{ 
				return evInterface.AddEventListener(&JEventListener::OnEvent, this, iden, eventTypeVec);
			} 
			void AddEventNotification(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType, Param... var)
			{ 
				evInterface.AddEventNotification(iden, eventType, std::forward<Param>(var)...);
			} 
			void NotifyEvent(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType, Param... var)
			{ 
				evInterface.NotifyEvent(iden, eventType, std::forward<Param>(var)...);
			} 
			void RemoveListener(EvInterface& evInterface, const IdentifierType& iden)
			{ 
				evInterface.RemoveListener(iden);
			} 
			void RemoveEventListener(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType)
			{ 
				evInterface.RemoveEventListener(iden, eventType);
			}
			void ResetEventListenerPointer(EvInterface& evInterface, const IdentifierType& iden)
			{ 
				evInterface.ResetListenerPointer(&JEventListener::OnEvent, this, iden);
			}
		};

	}
}