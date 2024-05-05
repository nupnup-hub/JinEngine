#pragma once 
#include"../JCoreEssential.h"
#include<cstdlib> 
#include<list> 
#include<algorithm> 
#include<stdexcept> 

namespace JinEngine
{
	namespace Core
	{
		typedef void(__cdecl* atexitFuncPtr)();
		template <class T>
		class JDefaultLifetime
		{
		public:
			static void ScheduleDestruction(T*, atexitFuncPtr fPtr)
			{
				atexit(fPtr);
			}
			static void OnDeadReference()
			{
				//수정필요 TypeList에서 GetName //+ typeid(T).name()
				throw std::logic_error("Dead Reference Detected");
			}
		};

		template <class T>
		class JNoDestroy
		{
		public:
			static void ScheduleDestruction(T* = nullptr, atexitFuncPtr fPtr = nullptr)
			{}
			static void OnDeadReference()
			{}
		};

		namespace Private
		{
			template <class T>
			struct Adapter
			{
				void operator()(T*) { return pFun(); }
				atexitFuncPtr pFun;
			};

			class LifetimeTracker;

			class LifetimeTracker
			{
			public:
				LifetimeTracker(unsigned int x) : longevity(x)
				{}
				virtual ~LifetimeTracker()
				{
				}
				static bool Compare(const LifetimeTracker* lhs, const LifetimeTracker* rhs)
				{
					return lhs->longevity > rhs->longevity;
				}
			private:
				unsigned int longevity;
			};

			// Helper destroyer function
			template <typename T>
			struct Deleter
			{
			public:
				typedef void (*Type)(T*);
				static void Delete(T* pObj)
				{
					delete pObj;
				}
			};

			template <typename T, typename Destroyer>
			class ConcreteLifetimeTracker : public LifetimeTracker
			{
			public:
				ConcreteLifetimeTracker(T* p, unsigned int longevity, Destroyer d)
					: LifetimeTracker(longevity)
					, pTracked(p)
					, destroyer(d)
				{}

				~ConcreteLifetimeTracker()
				{
					destroyer(pTracked);
				}
			private:
				T* pTracked;
				Destroyer destroyer;
			};

			static struct TrackerStruct
			{
			public:
				static std::list<LifetimeTracker*>& TrackerArray()
				{
					static std::list<LifetimeTracker*> trackerArray;
					return trackerArray;
				}
				static void TrackPopFunc()
				{
					if (TrackerArray().size() == 0)
						return;
					delete TrackerArray().back();
					TrackerArray().pop_back();
				}
			}Tracker;

			template <typename T, typename Destroyer>
			void SetLongevity(T* pDynObject, unsigned int longevity, Destroyer d)
			{
				// automatically delete the ConcreteLifetimeTracker object when a exception is thrown
				LifetimeTracker* p = new ConcreteLifetimeTracker<T, Destroyer>(pDynObject, longevity, d);

				if (TrackerStruct::TrackerArray().size() > 0)
				{
					std::list<LifetimeTracker*>::iterator pos = std::upper_bound(
						TrackerStruct::TrackerArray().begin(),
						TrackerStruct::TrackerArray().end(),
						p,
						LifetimeTracker::Compare);

					// Insert the pointer to the ConcreteLifetimeTracker object into the queue
					TrackerStruct::TrackerArray().insert(pos, p);
				}
				else
					TrackerStruct::TrackerArray().push_back(p);
				// Register a call to AtExitFn
				std::atexit(TrackerStruct::TrackPopFunc);
			}
			template <typename T>
			void SetLongevity(T* pDynObject, unsigned int longevity, typename Deleter<T>::Type d = Deleter<T>::Delete)
			{
				SetLongevity<T, typename Deleter<T>::Type>(pDynObject, longevity, d);
			}
		}

		template <unsigned int Longevity, class T>
		class JSingletonFixedLongevity
		{
		public:
			static void ScheduleDestruction(T* pObj, atexitFuncPtr pFun)
			{
				Private::Adapter<T> adapter = { pFun };
				Private::SetLongevity(pObj, Longevity, adapter);
			}
			static void OnDeadReference()
			{
				throw std::logic_error("Error_Dead Reference Detected");
			}
		};

		template <class T>
		class JDieLast : public JSingletonFixedLongevity<0xFFFFFFFF, T>
		{};

		template <class T>
		class JDieFirst : public JSingletonFixedLongevity<0, T>
		{};
	}
}