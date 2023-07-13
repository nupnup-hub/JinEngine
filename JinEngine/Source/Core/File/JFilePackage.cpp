#include"JFilePackage.h"
#include"../Encrypt/JEncryption.h"
#include<fstream>
#include <filesystem>
#include <iostream>
#include<sstream>

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			static size_t CreateHash(const std::wstring& path)
			{
				return std::hash <std::wstring>{}(path);
			}
			static int GetPathLastBackSlash(const std::wstring& path)noexcept
			{
				return (int)path.find_last_of(L"\\") + 1;
			}
			static int GetPathLastBackSlash(const std::string& path)noexcept
			{
				return (int)path.find_last_of("\\") + 1;
			}
			static int GetPathLastPeriod(const std::wstring& path)noexcept
			{
				return (int)path.find_last_of(L".");
			}
			static int GetPathLastPeriod(const std::string& path)noexcept
			{
				return (int)path.find_last_of(".");
			}
			static void DecomposeFolderPath(const std::wstring& path, std::wstring& folderPath, std::wstring& name)noexcept
			{
				const int nameStIndex = GetPathLastBackSlash(path);
				if (nameStIndex != -1)
				{
					name = path.substr(nameStIndex, (path.size() - nameStIndex));
					folderPath = path.substr(0, nameStIndex);
				}
				else
				{
					folderPath = L"";
					name = path;
				}
			}
		}

		JFilePackage::FileInfo::FileInfo(const std::wstring name, const size_t guid, const size_t fileSize)
			:name(name), guid(guid), fileSize(fileSize)
		{}
		JFilePackage::FileInfo::FileInfo(const std::wstring name, const size_t guid, const size_t fileSize, const std::wstring& oriPath)
			: name(name), guid(guid), fileSize(fileSize), oriPath(oriPath)
		{}
		void JFilePackage::SetPackagePath(const std::wstring& path)
		{
			packagePath = path;
		}
		bool JFilePackage::AddFile(const std::wstring& path)
		{
			if (_waccess(path.c_str(), 00) == -1)
				return false;

			const size_t hash = CreateHash(path);
			auto data = header.fileInfo.find(hash);
			if (data != header.fileInfo.end())
				return false;

			std::wstring name;
			std::wstring folderPath;
			DecomposeFolderPath(path, folderPath, name);
			header.fileInfo.emplace(hash, FileInfo(name, hash, std::filesystem::file_size(path.c_str()), path));
			return true;
		}
		bool JFilePackage::PopFile(const std::wstring& path)
		{
			if (_waccess(path.c_str(), 00) == -1)
				return false;

			const size_t hash = CreateHash(path);
			auto data = header.fileInfo.find(hash);
			if (data == header.fileInfo.end())
				return false;

			header.fileInfo.erase(hash);
			return true;
		}
		bool JFilePackage::StorePackage(const bool applyEncryption, const bool clearIfSuccess)
		{
			if (packagePath.empty() || header.fileInfo.size() == 0)
				return false;

			header.isEncrypted = applyEncryption;

			std::wifstream from;
			std::wofstream to;
			to.open(packagePath.c_str(), std::ios::binary | std::ios::out);
			if (!to.is_open())
				return false;

			to << "FileCount: " << header.fileInfo.size() << "\n";
			to << "IsEncrypted: " << header.isEncrypted << "\n";
			for (const auto& data : header.fileInfo)
			{
				std::wstring message = L"Name: " + data.second.name +
					L" Guid: " + std::to_wstring(data.second.guid) +
					L" FileSize: " + std::to_wstring(data.second.fileSize) + L"\n";

				if(applyEncryption)
					JSecurityObscurity::EncryptM(message);
				to << message;
			}
			for (const auto& data : header.fileInfo)
			{
				from.open(data.second.oriPath, std::ios::binary | std::ios::in);
				to << from.rdbuf();
				from.close();
			}
			to.close();
			if(clearIfSuccess)
				Clear();
			return true;
		}
		bool JFilePackage::LoadPackage(JFilePackage::Header& result, const bool clearIfSuccess)
		{
			if (packagePath.empty())
				return false;

			std::wifstream from;
			from.open(packagePath.c_str(), std::ios::binary | std::ios::in);
			if (!from.is_open())
				return false;

			std::wstring guide;
			uint fileCount;
			bool isEncrypted;

			from >> guide;	from >> fileCount;
			from >> guide;	from >> isEncrypted;

			result.isEncrypted = isEncrypted;
			for (uint i = 0; i < fileCount; ++i)
			{
				std::wstring message;
				std::getline(from, message);
				if (isEncrypted)
					JSecurityObscurity::DecryptM(message);

				std::wstringstream stream(message.c_str());

				std::wstring name;
				size_t guid;
				size_t fileSize;
				stream >> guide; stream >> name;
				stream >> guide; stream >> guid;
				stream >> guide; stream >> fileSize;

				result.fileInfo.emplace(guid, FileInfo(name, guid, fileSize));
			}
			from.close();
			if (clearIfSuccess)
				Clear();
			return true;
		}
		void JFilePackage::Clear()
		{
			packagePath.clear();
			header.fileInfo.clear();
		}
	}
}