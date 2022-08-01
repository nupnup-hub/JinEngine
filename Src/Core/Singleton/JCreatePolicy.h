#pragma once
namespace JinEngine
{
	namespace Core
	{
		template <class T>
		class JCreateUsingNew
		{
		public:
			static T* Create()
			{
				return new T;
			}
			static void Destroy(T* p)
			{
				delete p;
			}
		};

		template<template<class> class Alloc>
		class JCreateUsingAlloc
		{
		public:
			template <class T>
			struct Allocator
			{
				static Alloc<T> allocator;
				static T* Create()
				{
					return allocator.Allocate(sizeof(T));
				}

				static void Destroy(T* p)
				{
					//allocator.destroy(p);
					//p->~T();
					allocator.Deallocate(p, sizeof(T));
				}
			};
		};
	}
}