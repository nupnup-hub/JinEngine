#include"JFSMinterface.h"
#include"JFSMinterfacePrivate.h"
#include"../File/JFileConstant.h"
#include"../../Utility/JCommonUtility.h"  
#include<fstream>

namespace JinEngine
{
	namespace Core
	{ 
		JFSMinterface::InitData::InitData(const JTypeInfo& initTypeInfo)
			:JIdentifier::InitData(initTypeInfo)
		{}
		JFSMinterface::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid)
			:JIdentifier::InitData(initTypeInfo, name, guid)
		{}
		JFSMinterface::JFSMinterface(const InitData& initData)
			:JIdentifier(initData)
		{}

		//fsm object can't copy
		using CreateInstanceInterface = JFSMinterfacePrivate::CreateInstanceInterface; 
		bool CreateInstanceInterface::Copy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to)
		{
			return false;
		}
		bool CreateInstanceInterface::CanCopy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to)noexcept
		{
			return false;
		}
	};
}
