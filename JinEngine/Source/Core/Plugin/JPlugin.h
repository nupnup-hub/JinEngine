#pragma once
#include"JPluginInterface.h" 

namespace JinEngine
{
	namespace Core
	{
		class JPluginManager;
		class JPluginPrivate;
		class JPlugin final : public JPluginInterface
		{
		private:
			friend class JPluginManager;
			friend class JPluginPrivate;
		private:
			/** @brief It is same as meta file name */
			const std::wstring name;
			/** @brief metafile path */
			const std::wstring path;
		private:
			JPluginDesc desc; 
		private:
			std::vector<JUserPtr<JModule>> moduleVec;
		public:
			JPlugin(const std::wstring& name, const std::wstring& filePath, const JPluginDesc& desc);
			~JPlugin();
		public:
			const std::wstring& GetName()const noexcept final;
			const std::wstring& GetMetaFilePath()const noexcept final; 
			JPluginDesc GetPluginDesc()const noexcept final; 
			uint GetModuleCount()const noexcept final;
			JUserPtr<JModule> GetModule(const uint index)const noexcept final;
			JUserPtr<JModule> GetModule(const std::wstring& name)const noexcept final;
			std::vector<JUserPtr<JModule>> GetAllModule()const noexcept final;
		};
	}
}