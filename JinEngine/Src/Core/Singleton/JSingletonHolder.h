#pragma once
#include"JCreatePolicy.h"
#include"JLifetimePolicy.h"
#include"JThreadingModel.h" 

namespace JinEngine
{
	namespace Core
	{
		template
			<
			typename T,
			template <class> class CreationPolicy = JCreateUsingNew,
			template <class> class LifetimePolicy = JDefaultLifetime,
			template <class> class ThreadingModel = JSingleThread
			>
		class JSingletonHolder
		{
		public:
			///  Type of the singleton object
			typedef T ObjectType;

			///  Returns a reference to singleton object
			static T& Instance();
		private:
			// Helpers
			static void MakeInstance();
			static void DestroySingleton();

			// Protection
			JSingletonHolder() = default;

			// Data
			typedef typename ThreadingModel<T*>::VolatileType PtrInstanceType;
			static PtrInstanceType pInstance;
			static bool destroyed;
		};

		////////////////////////////////////////////////////////////////////////////////
		// JSingletonHolder's data
		////////////////////////////////////////////////////////////////////////////////

		template
			<
			class T,
			template <class> class C,
			template <class> class L,
			template <class> class M
			>
			typename JSingletonHolder<T, C, L, M>::PtrInstanceType
			JSingletonHolder<T, C, L, M>::pInstance = 0;

		template
			<
			class T,
			template <class> class C,
			template <class> class L,
			template <class> class M
			>
			bool JSingletonHolder<T, C, L, M>::destroyed = false;

		////////////////////////////////////////////////////////////////////////////////
		// JSingletonHolder::Instance
		////////////////////////////////////////////////////////////////////////////////

		template
			<
			class T,
			template <class> class CreationPolicy,
			template <class> class LifetimePolicy,
			template <class> class ThreadingModel
			>
			inline T& JSingletonHolder<T, CreationPolicy, LifetimePolicy, ThreadingModel>::Instance()
		{
			if (!pInstance)
			{
				MakeInstance();
			}
			return *pInstance;
		}

		////////////////////////////////////////////////////////////////////////////////
		// JSingletonHolder::MakeInstance (helper for Instance)
		////////////////////////////////////////////////////////////////////////////////

		template
			<
			class T,
			template <class> class CreationPolicy,
			template <class> class LifetimePolicy,
			template <class> class ThreadingModel
			>
			void JSingletonHolder<T, CreationPolicy, LifetimePolicy, ThreadingModel>::MakeInstance()
		{
			typename ThreadingModel<JSingletonHolder>::Lock guard;
			(void)guard;

			if (!pInstance)
			{
				if (destroyed)
				{
					destroyed = false;
					LifetimePolicy<T>::OnDeadReference();
				}
				pInstance = CreationPolicy<T>::Create();
				LifetimePolicy<T>::ScheduleDestruction(pInstance, &DestroySingleton);
			}
		}

		template
			<
			class T,
			template <class> class CreationPolicy,
			template <class> class L,
			template <class> class M
			>
			void JSingletonHolder<T, CreationPolicy, L, M>::DestroySingleton()
		{
			assert(!destroyed);
			CreationPolicy<T>::Destroy(pInstance);
			pInstance = 0;
			destroyed = true;
		}
	}

}