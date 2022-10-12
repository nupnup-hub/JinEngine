#pragma once
#include"../JObject.h"

namespace JinEngine
{
	class JComponent;
	class JScene;

	class JGameObjectCompInterface 
	{
	private:
		friend class JComponent;
	protected:
		virtual ~JGameObjectCompInterface() = default;
	public:
		virtual JGameObjectCompInterface* CompInterface() = 0;
	private:
		virtual bool AddComponent(JComponent& jComp)noexcept = 0;
		virtual bool RemoveComponent(JComponent& jComp)noexcept = 0;
	};

	class JGameObjectInterface : public JObject,
		public JGameObjectCompInterface
	{ 
	private:
		friend class JScene;
	protected:
		JGameObjectInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag);
	private:
		virtual Core::J_FILE_IO_RESULT CallStoreGameObject(std::wofstream& stream) = 0;
	}; 
}