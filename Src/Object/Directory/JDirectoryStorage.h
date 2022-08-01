#pragma once
#include"../../Core/Storage/JStorage.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"../../Core/File/JFileIOResult.h"
#include"../../Object/Directory/JDirectory.h"

namespace JinEngine
{
	class JDirectory;
	class JDirectoryStorage
	{
	private:
		using ResourceVector = Core::JVectorStorage<JDirectory>;
		using ResourceMap = Core::JMapStorage<JDirectory, size_t>;
		using StoreInfoVec = std::vector<Core::JFileIOResultInfo>;
	private:
		ResourceVector rVec;
		ResourceMap rMap;
	public:
		JDirectory* GetByPath(const std::string& path)
		{
			const uint count = rVec.Count();
			for (uint i = 0; i < count; ++i)
			{
				if (rVec[i]->GetPath() == path)
					return rVec[i];
			}
			return nullptr;
		}
		JDirectory* GetAcitvatedDirectory()
		{
			const uint count = rVec.Count();
			for (uint i = 0; i < count; ++i)
			{
				if (rVec[i]->IsActivated())
					return rVec[i];
			}
			return nullptr;
		}
		JDirectory* AddResource(JDirectory* dir)noexcept
		{
			if (dir == nullptr)
				return nullptr;
			rVec.Add(dir);
			rMap.Add(dir, dir->GetGuid());

			return dir;
		}
		bool EraseResource(JDirectory* dir)noexcept
		{
			if (dir == nullptr)
				return false;
 
			static auto equalLam = [](JDirectory* a, JDirectory* b) {return a->GetGuid() == b->GetGuid(); };
			
			rVec.Erase(dir, equalLam);
			rMap.Erase(dir->GetGuid());
			return dir;
		}
		void Clear()
		{
			rVec.Clear();
			rMap.Clear();
		}
	};

	using JDS = JDirectoryStorage;
}