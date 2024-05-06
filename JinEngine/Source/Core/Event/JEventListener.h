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