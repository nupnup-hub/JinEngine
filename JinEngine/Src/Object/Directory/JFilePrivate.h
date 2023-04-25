#pragma once
#include<memory>

namespace JinEngine
{
	class JDirectory;
	class JFile;
	struct JFileInitData;
	class JFilePrivate final
	{
	private:
		friend class JDirectoryPrivate;
	private:
		std::unique_ptr<JFile> CreateFile(const JFileInitData& initData, JDirectory* ownerDir);
	private: 
		void ConvertToActFileData(JFile* file, JResourceObject* rObj);
		void ConvertToDeActFileData(JFile* file); 
	};
}