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