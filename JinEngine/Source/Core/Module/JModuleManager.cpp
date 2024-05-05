#include"JModuleManager.h"
#include"JModuleManagerPrivate.h"
#include"JModule.h"
#include"JModulePrivate.h"
#include"JModuleIntializer.h"
#include"../Utility/JCommonUtility.h" 
 
namespace JinEngine::Core
{
	namespace
	{
		using ModuleIOInterface = JModulePrivate::IOInterface;
	}
	namespace
	{
		static size_t CreateModuleGuid(JModule* m)noexcept
		{
			return std::hash<std::wstring>{}(m->GetPath());
		}
		static size_t CreateModuleGuid(const std::wstring& path)noexcept
		{
			return std::hash<std::wstring>{}(path);
		}
		static bool IsSame(const JOwnerPtr<JModule>& m, const size_t guid)noexcept
		{
			return CreateModuleGuid(m.Get()) == guid;
		}

	}
	class JModuleManager::JModuleManagerImpl
	{
	public:
		std::vector<JOwnerPtr<JModule>> moduleVec;
		//key is path hash
		std::unordered_map<size_t, JUserPtr<JModule>> moduleMap;
	public:
		JUserPtr<JModule> GetModule(const std::wstring& path)
		{ 
			auto data = moduleMap.find(CreateModuleGuid(path));
			if (data == moduleMap.end())
				return nullptr;

			return data->second;
		}
	public:
		bool AddModule(JOwnerPtr<JModule>&& newMod)
		{  
			if (newMod == nullptr)
				return false;
			 
			const size_t guid = CreateModuleGuid(newMod.Get());
			auto data = moduleMap.find(guid);
			if (data != moduleMap.end())
				return false;
			 
			moduleMap.emplace(guid, newMod);
			moduleVec.push_back(std::move(newMod));
			return true;
		}
		bool RemoveModule(const size_t guid)
		{
			auto data = moduleMap.find(guid);
			if (data == moduleMap.end())
				return false;

			moduleMap.erase(guid);
			int index = JCUtil::GetIndex(moduleVec, &IsSame, guid);
			moduleVec.erase(moduleVec.begin() + index);
			return true;
		}
	public:
		uint LoadModule(const std::wstring& folder)
		{
			std::vector<std::wstring> metafilePath;
			JCUtil::FindFilePathByFormat(folder, JModule::MetafileFormat(), metafilePath);

			uint preCount = (uint)moduleVec.size();
			for (const auto& data : metafilePath)
			{
				auto m = ModuleIOInterface::LoadModule(data);
				if (m != nullptr)
					AddModule(std::move(m));
			}
			return (uint)moduleVec.size() - preCount;
		}
		bool StoreModule(const JUserPtr<JModule>& m)
		{
			return ModuleIOInterface::StoreModule(m);
		}
	public:
		JUserPtr<JModule> TryAddModule(const std::wstring& filePath, const bool keppLoadState)
		{ 
			auto exist = GetModule(filePath);
			if (exist != nullptr)
				return exist;
			 
			if (JModule::TryLoadModule(filePath) == NULL)
				return nullptr;		

			//dll에서 JModuleManager에 요청한 Register가 성공시 GetModule은 유효한 값을 반환
			exist = GetModule(filePath);
			if (exist == nullptr)
				return nullptr;

			if (!keppLoadState)
				exist->UnLoad();

			ModuleIOInterface::StoreModule(exist);
			return exist;
		}
	public: 
		void Clear()
		{  
			for (const auto& data : moduleVec)
				data->UnLoad();
			 
			moduleMap.clear(); 
			moduleVec.clear();
		}
	};

	JUserPtr<JModule> JModuleManager::GetModule(const std::wstring& path)const noexcept
	{
		return impl->GetModule(path);
	}
	bool JModuleManager::HasModule(const std::wstring& path)const noexcept
	{
		return impl->GetModule(path) != nullptr;
	}
	uint JModuleManager::LoadModule(const std::wstring& folder)
	{ 
		return impl->LoadModule(folder);
	}
	bool JModuleManager::StoreModule(const JUserPtr<JModule>& m)
	{
		return impl->StoreModule(m);
	}
	JUserPtr<JModule> JModuleManager::TryAddModule(const std::wstring& filePath, const bool keppLoadState)
	{
		return impl->TryAddModule(filePath, keppLoadState);
	}
	bool JModuleManager::RegisterModule(const JModuleIntializer& init)
	{ 
		return impl->AddModule(JPtrUtil::MakeOwnerPtr<JModule>(init.name, init.path, init.desc));
	}
	JOwnerPtr<JModule> JModuleManager::CreateModule(const JModuleIntializer& init)
	{
		return JPtrUtil::MakeOwnerPtr<JModule>(init.name, init.path, init.desc);
	}
	JModuleManager::JModuleManager()
		:impl(std::make_unique<JModuleManagerImpl>())
	{}
	JModuleManager::~JModuleManager()
	{
		impl.reset();
	}

	using MainAccess = JModuleManagerPrivate::MainAccess;
	 
	void MainAccess::Clear()
	{
		_JModuleManager::Instance().impl->Clear();
	}
}