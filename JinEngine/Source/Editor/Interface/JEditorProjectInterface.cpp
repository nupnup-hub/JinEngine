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


#include"JEditorProjectInterface.h"

namespace JinEngine::Editor
{
	JEditorProjectInterface::JEditorProjectInterface(std::unique_ptr<LoadProjectListF::Functor>&& loadProjectListF,
		std::unique_ptr<StoreProjectListF::Functor>&& storeProjectListF,
		std::unique_ptr<ReLoadProjectF::Functor>&& reLoadProjectF,
		std::unique_ptr<LoadOtherProjectF::Functor>&& loadOtherProjectF,
		std::unique_ptr<LoadUnRegisteredProjectF::Functor>&& loadUnRegisteredProjectF,
		std::unique_ptr<StoreProjectF::Functor>&& storeProjectF,
		std::unique_ptr<CreateProjectF::Functor>&& createProjectF,
		std::unique_ptr<DestroyProjectF::Functor> destroyProjectF,
		std::unique_ptr<StartProjectF::Functor>&& startProjectF,
		std::unique_ptr<ConfirmCloseProjectF::Functor>&& confirmCloseProjectF,
		std::unique_ptr<CloseCloseProjectF::Functor>&& cancelCloseProjectF)
		: loadProjectListF(std::move(loadProjectListF)),
		storeProjectListF(std::move(storeProjectListF)),
		reLoadProjectF(std::move(reLoadProjectF)),
		loadOtherProjectF(std::move(loadOtherProjectF)),
		loadUnRegisteredProjectF(std::move(loadUnRegisteredProjectF)),
		storeProjectF(std::move(storeProjectF)),
		createProjectF(std::move(createProjectF)),
		destroyProjectF(std::move(destroyProjectF)),
		startProjectF(std::move(startProjectF)),
		confirmCloseProjectF(std::move(confirmCloseProjectF)),
		cancelCloseProjectF(std::move(cancelCloseProjectF))
	{}
}