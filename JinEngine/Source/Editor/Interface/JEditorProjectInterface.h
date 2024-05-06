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
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Application/Project/JApplicationProject.h"

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