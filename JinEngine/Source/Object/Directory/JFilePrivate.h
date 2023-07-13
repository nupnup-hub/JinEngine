#pragma once
#include<memory>
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	class JDirectory;
	class JFile;
	class JResourceObject;
	struct JFileInitData;
	class JFilePrivate final
	{
	private:
		friend class JDirectoryPrivate;
	private:
		JOwnerPtr<JFile> CreateFile(const JFileInitData& initData, const JUserPtr<JDirectory>& ownerDir);
	private: 
		void ConvertToActFileData(const JUserPtr<JFile>& file, const JUserPtr<JResourceObject>& rObj);
		void ConvertToDeActFileData(const JUserPtr<JFile>& file);
	};
}