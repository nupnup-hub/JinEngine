#pragma once 
#include<vector> 
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
		private:
			const size_t listenerGuid;
		public:
			JEventListener(const size_t listenerGuid)
				:listenerGuid(listenerGuid)
			{}
			virtual ~JEventListener() = default;
		protected: 
			virtual void OnEvent(const IdentifierType& iden, const EVENTTYPE& eventType, Param... var) = 0;
		protected:  
			bool AddEventListener(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType)
			{
				if (listenerGuid != iden)
					return false;
				return evInterface.AddEventListener(&JEventListener::OnEvent, this, iden, eventType);
			} 
			size_t AddEventListener(EvInterface& evInterface, const IdentifierType& iden, const std::vector<EVENTTYPE>& eventTypeVec)
			{
				if (listenerGuid != iden)
					return 0;
				return evInterface.AddEventListener(&JEventListener::OnEvent, this, iden, eventTypeVec);
			} 
			void AddEventNotification(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType, Param... var)
			{
				if (listenerGuid != iden)
					return;
				evInterface.AddEventNotification(iden, eventType, std::forward<Param>(var)...);
			} 
			void NotifyEvent(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType, Param... var)
			{
				if (listenerGuid != iden)
					return;
				evInterface.NotifyEvent(iden, eventType, std::forward<Param>(var)...);
			} 
			void RemoveListener(EvInterface& evInterface, const IdentifierType& iden)
			{
				if (listenerGuid != iden)
					return;
				evInterface.RemoveListener(iden);
			} 
			void RemoveEventListener(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType)
			{
				if (listenerGuid != iden)
					return;
				evInterface.RemoveEventListener(iden, eventType);
			}
		};

	}
}