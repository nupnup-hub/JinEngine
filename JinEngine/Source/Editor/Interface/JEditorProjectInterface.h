#pragma once
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Application/JApplicationProject.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorProjectInterface
		{
		public:
			using LoadProjectListF = Core::JSFunctorType<void>;
			using StoreProjectListF = Core::JSFunctorType<void>;
		public:
			using ReLoadProjectF = Core::JSFunctorType<void>;
			using LoadOtherProjectF = Core::JSFunctorType<void>; 
			using LoadUnRegisteredProjectF = Core::JSFunctorType<void, std::wstring>;	//path
			using StoreProjectF = Core::JSFunctorType<void>;
		public:
			using CreateProjectF = Core::JSFunctorType<void, std::wstring, std::string>;	//path, version
			using DestroyProjectF = Core::JSFunctorType<void, int>;	//index
			using StartProjectF = Core::JSFunctorType<void, std::unique_ptr<JApplicationProjectInfo>&&>;
		public:
			using ConfirmCloseProjectF = Core::JSFunctorType<void>;
			using CloseCloseProjectF = Core::JSFunctorType<void>; 
		public:
			std::unique_ptr<LoadProjectListF::Functor> loadProjectListF;
			std::unique_ptr<StoreProjectListF::Functor> storeProjectListF;
		public:
			std::unique_ptr<ReLoadProjectF::Functor> reLoadProjectF;
			std::unique_ptr<LoadOtherProjectF::Functor> loadOtherProjectF;
			std::unique_ptr<LoadUnRegisteredProjectF::Functor> loadUnRegisteredProjectF;
			std::unique_ptr<StoreProjectF::Functor> storeProjectF;
		public:
			std::unique_ptr<CreateProjectF::Functor> createProjectF;
			std::unique_ptr<DestroyProjectF::Functor> destroyProjectF;
			std::unique_ptr<StartProjectF::Functor> startProjectF;
		public:
			std::unique_ptr<ConfirmCloseProjectF::Functor> confirmCloseProjectF;
			std::unique_ptr<CloseCloseProjectF::Functor> cancelCloseProjectF;
		public:
			JEditorProjectInterface(std::unique_ptr<LoadProjectListF::Functor>&& loadProjectListF,
				std::unique_ptr<StoreProjectListF::Functor>&& storeProjectListF,
				std::unique_ptr<ReLoadProjectF::Functor>&& reLoadProjectF,
				std::unique_ptr<LoadOtherProjectF::Functor>&& loadOtherProjectF,
				std::unique_ptr<LoadUnRegisteredProjectF::Functor>&& loadUnRegisteredProjectF,
				std::unique_ptr<StoreProjectF::Functor>&& storeProjectF,
				std::unique_ptr<CreateProjectF::Functor>&& createProjectF,
				std::unique_ptr<DestroyProjectF::Functor> destroyProjectF,
				std::unique_ptr<StartProjectF::Functor>&& startProjectF,
				std::unique_ptr<ConfirmCloseProjectF::Functor>&& confirmCloseProjectF,
				std::unique_ptr<CloseCloseProjectF::Functor>&& cancelCloseProjectF);
		};
	}
}