#pragma once
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		template<typename IdentifierType, typename EVENTTYPE, typename ...Param>
		class JEventListener
		{
		protected: 
			virtual void OnEvent(const IdentifierType& iden, const EVENTTYPE& eventType, Param... var) = 0;
		protected: 
			template<typename EvInterface>
			bool AddEventListener(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType)
			{
				return evInterface.AddEventListener(&JEventListener::OnEvent, this, iden, eventType);
			}
			template<typename EvInterface>
			size_t AddEventListener(EvInterface& evInterface, const IdentifierType& iden, const std::vector<EVENTTYPE>& eventTypeVec)
			{
				return evInterface.AddEventListener(&JEventListener::OnEvent, this, iden, eventTypeVec);
			}
			template<typename EvInterface, typename ...Param>
			void NotifyEvent(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType, Param&&... var)
			{
				evInterface.NotifyEvent(iden, eventType, std::forward<Param>(var)...);
			}
			template<typename EvInterface>
			void EraseListener(EvInterface& evInterface, const IdentifierType& iden)
			{
				evInterface.EraseListener(iden);
			}
			template<typename EvInterface>
			void EraseListenerEvent(EvInterface& evInterface, const IdentifierType& iden, const EVENTTYPE& eventType)
			{
				evInterface.EraseListenerEvent(iden, eventType);
			}
		};

	}
}