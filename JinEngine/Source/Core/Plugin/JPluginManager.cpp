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


#include"JPluginManager.h"
#include"JPluginManagerPrivate.h"
#include"JPlugin.h" 
#include"JPluginPrivate.h" 
#include"../File/JXmlHelper.h"
#include"../Utility/JCommonUtility.h"  

namespace JinEngine::Core
{
	namespace
	{
		using PluginIOInteface = JPluginPrivate::IOInterface;
	}
	namespace
	{
		static size_t CreatePluginGuid(JPluginInterface* p)noexcept
		{
			return std::hash<std::wstring>{}(p->GetMetaFilePath());
		}
		static size_t GetPlugMapKey(const std::wstring& path)
		{
			return std::hash<std::wstring>{}(path);
		}
		static std::wstring MetafileFormat()
		{
			return L".plugin";
		}
	}
	class JPluginManager::JPluginManagerImpl
	{
	public: 
		//key value is plugin metafile path
		std::unordered_map<size_t, JOwnerPtr<JPluginInterface>> pMap;
	public:
		JUserPtr<JPluginInterface> GetPlugIn(const std::wstring& path)const
		{
			auto data = pMap.find(GetPlugMapKey(path));
			return data != pMap.end() ? JUserPtr<JPluginInterface>(data->second) : nullptr;
		}
		std::vector<JUserPtr<JPluginInterface>> GetAllPlugin()const
		{
			std::vector<JUserPtr<JPluginInterface>> res;
			for (const auto& data : pMap)
				res.emplace_back(data.second);
			return res;
		}
	public:
		bool AddPlugin(JOwnerPtr<JPluginInterface>&& newP)
		{
			if (newP == nullptr)
				return false;

			const size_t guid = CreatePluginGuid(newP.Get());
			auto data = pMap.find(guid);
			if (data != pMap.end())
				return false;

			pMap.emplace(guid, std::move(newP));
			return true;
		}
		bool RemovePlugin(const size_t guid)
		{
			auto data = pMap.find(guid);
			if (data == pMap.end())
				return false;

			pMap.erase(guid);
			return true;
		}
	public: 
		uint LoadPlugin(const std::wstring& folder)
		{
			std::vector<std::wstring> metafilePath;
			JCUtil::FindFilePathByFormat(folder, MetafileFormat(), metafilePath);

			uint preCount = (uint)pMap.size();
			for (const auto& data : metafilePath)
			{
				auto p = PluginIOInteface::LoadPlugin(data);
				if (p != nullptr)
					AddPlugin(std::move(p));
			}
			return (uint)pMap.size() - preCount;
		}
		bool StorePlugin(JPluginInterface* p)
		{
			return PluginIOInteface::StorePlugin(p);
		}
	public:
		JPluginInterface* Plugin(const std::wstring& name,
			const std::wstring& folderPath,
			const std::vector<JUserPtr<JModule>>& modVec,
			JPluginDesc desc)
		{
			for (const auto& data : modVec)
			{
				if (data == nullptr)
					return nullptr;
			}

			const std::wstring path = folderPath + L"\\" + name + MetafileFormat();
			auto exist = GetPlugIn(path);
			if (exist != nullptr)
				return exist.Get();

			desc.isLoaded = false;
			 
			JOwnerPtr<JPlugin> newPlugin = JPtrUtil::MakeOwnerPtr<JPlugin>(name, path, desc);
			if (newPlugin == nullptr)
				return nullptr;
			 
			auto rawPtr = newPlugin.Get();
			if (!AddPlugin(std::move(rawPtr)))
				return nullptr;

			PluginIOInteface::StorePlugin(rawPtr);
			return rawPtr;
		}
	public:
		void Clear()
		{ 
			pMap.clear(); 
		}
	};


	JUserPtr<JPluginInterface> JPluginManager::GetPlugIn(const std::wstring& path)const
	{ 
		return impl->GetPlugIn(path);
	} 
	std::vector<JUserPtr<JPluginInterface>> JPluginManager::GetAllPlugin()const
	{
		return impl->GetAllPlugin();
	}
	uint JPluginManager::LoadPlugin(const std::wstring& folderPath)
	{
		return impl->LoadPlugin(folderPath);
	}
	bool JPluginManager::StorePlugin(JPluginInterface* p)
	{
		if (p == nullptr)
			return false;

		return impl->StorePlugin(p);
	}
	JPluginInterface* JPluginManager::TryAddPlugin(const std::wstring& name,
		const std::wstring& folderPath,
		const std::vector<JUserPtr<JModule>>& modVec,
		JPluginDesc desc)
	{
		return impl->Plugin(name, folderPath, modVec, desc);
	}
	JPluginManager::JPluginManager()
		:impl(std::make_unique<JPluginManagerImpl>())
	{}
	JPluginManager::~JPluginManager()
	{
		impl.reset();
	}

	using MainAccess = JPluginManagerPrivate::MainAccess;
	void MainAccess::Clear()
	{ 
		_JPluginManager::Instance().impl->Clear();
	}
}