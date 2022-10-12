#pragma once 
#include"../../Core/Factory/JFactory.h"
#include"../../Core/Func/Callable/JCallable.h"

#include"Shader/JShader.h" 
#include"Skeleton/JSkeleton.h"
#include"Skeleton/JSkeletonAsset.h"
#include"JResourceManager.h" 

namespace JinEngine
{
	class JResourceObject;
	class JResourceIO;
	class JResourceManagerImpl;
	class JResourceObjectFactoryImplBase;
	template<typename T> class JResourceObjectFactoryImpl;

	struct JResourceObject::JResourceInitData;
	using JResourceInitData = JResourceObject::JResourceInitData;

	class JResourceObjectFactory
	{
	private:
		friend class JResourceObjectFactoryImplBase;
		template<typename T> friend class JResourceObjectFactoryImpl;
	public:
		Core::JFactory<std::string, false, JResourceObject*, Core::JOwnerPtr<JResourceInitData>> defaultFactory;
		Core::JFactory<std::string, false, JResourceObject*, JDirectory*, const Core::JAssetFileLoadPathData&> loadFactory;
		Core::JFactory<std::string, false, JResourceObject*, JResourceObject*, JDirectory*> copyFactory;
	private:
		bool Register(const std::string iden, Core::JCallableInterface<JResourceObject*, Core::JOwnerPtr<JResourceInitData>>* callable);
		bool Register(const std::string iden, Core::JCallableInterface<JResourceObject*, JDirectory*, const Core::JAssetFileLoadPathData&>* callable);
		bool Register(const std::string iden, Core::JCallableInterface<JResourceObject*, JResourceObject*, JDirectory*>* callable);
		JResourceObject* Create(const std::string& iden, Core::JOwnerPtr<JResourceInitData> initdata);
		JResourceObject* Create(const std::string& iden, JDirectory& ownerDir, const Core::JAssetFileLoadPathData& pathData);
		JResourceObject* Copy(const std::string& iden, JResourceObject& ori, JDirectory& ownerDir);
	};

	using JRF = Core::JSingletonHolder<JResourceObjectFactory>;

	class JResourceObjectFactoryImplBase
	{
	private:
		friend class JDirectory;
		friend class JResourceIO;
		friend class JResourceManagerImpl;
	protected:
		using DefaultC = Core::JStaticCallableType<JResourceObject*, Core::JOwnerPtr<JResourceInitData>>;
		using LoadC = Core::JStaticCallableType<JResourceObject*, JDirectory*, const Core::JAssetFileLoadPathData&>;
		using CopyC = Core::JStaticCallableType<JResourceObject*, JResourceObject*, JDirectory*>;
	public:
		static JResourceObject* CreateByName(const std::string& typeName, Core::JOwnerPtr<JResourceInitData> initdata)
		{
			return JRF::Instance().Create(typeName, std::move(initdata));
		}
	private:
		static JResourceObject* LoadByName(const std::string& typeName, JDirectory& ownerDir, const Core::JAssetFileLoadPathData& pathData)
		{
			return JRF::Instance().Create(typeName, ownerDir, pathData);
		}
		static JResourceObject* CopyByName(const std::string& typeName, JResourceObject& ori, JDirectory& ownerDir)
		{
			return  JRF::Instance().Copy(typeName, ori, ownerDir);
		}
	protected:
		template<typename T, std::enable_if_t<std::is_base_of_v<JResourceObject, T>, int> = 0>
		static void Register(DefaultC::Ptr defaultPtr, LoadC::Ptr loadPtr, CopyC::Ptr copytPtr)
		{
			if (defaultPtr != nullptr)
			{
				static DefaultC::Callable defaultCallable{ defaultPtr };
				JRF::Instance().Register(T::TypeName(), &defaultCallable);
			}
			if (loadPtr != nullptr)
			{
				static LoadC::Callable loadCallable{ loadPtr };
				JRF::Instance().Register(T::TypeName(), &loadCallable);
			}
			if (copytPtr != nullptr)
			{
				static CopyC::Callable copyCallable{ copytPtr };
				JRF::Instance().Register(T::TypeName(), &copyCallable);
			}
		}
	};

	template<typename T>
	class JResourceObjectFactoryImpl : public JResourceObjectFactoryImplBase
	{
	private:
		friend T;
		friend class JResourceIO;
		friend class JResourceManagerImpl;
	public:
		static T* Create(Core::JOwnerPtr<JResourceInitData> initdata)
		{ 
			return static_cast<T*>(JRF::Instance().Create(T::TypeName(), std::move(initdata)));
		}
		static T* Copy(T& ori)
		{
			return static_cast<T*>(T::TypeName(), JRF::Instance().Copy(ori));
		}
	private:
		static T* Load(JDirectory& ownerDir, const Core::JAssetFileLoadPathData& pathData)
		{
			return static_cast<T*>(JRF::Instance().Load(T::TypeName(), ownerDir, pathData));
		}
		static void Register(DefaultC::Ptr defaultPtr, LoadC::Ptr loadPtr, CopyC::Ptr copytPtr)
		{
			JResourceObjectFactoryImplBase::Register<T>(defaultPtr, loadPtr, copytPtr);
		}
	};
	template<typename T>
	using JRFI = JResourceObjectFactoryImpl<T>;
	using JRFIB = JResourceObjectFactoryImplBase;
}