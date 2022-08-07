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
	class JResourceManager;
	class JResourceObjectFactoryImplBase;
	template<typename T> class JResourceObjectFactoryImpl;

	namespace
	{
		class JResourceObjectFactory
		{
		private:
			friend class JResourceObjectFactoryImplBase;
			template<typename T> friend class JResourceObjectFactoryImpl;
		private:
			Core::JFactory<std::string, false, JResourceObject*, JDirectory*> defaultFactory;
			Core::JFactory<std::string, false, JResourceObject*, const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*, const uint8> initFactory;
			Core::JFactory<std::string, false, JResourceObject*, JDirectory*, const JResourcePathData&> loadFactory;
			Core::JFactory<std::string, false, JResourceObject*, JResourceObject*> copyFactory;
		private:
			template<typename Type>
			bool Register(Core::JCallableInterface<JResourceObject*, JDirectory*>* callable)
			{
				if constexpr (std::is_base_of_v <JResourceObject, Type >)
					return defaultFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}
			template<typename Type>
			bool Register(Core::JCallableInterface<JResourceObject*, const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*, const uint8>* callable)
			{
				if constexpr (std::is_base_of_v <JResourceObject, Type >)
					return initFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}
			template<typename Type>
			bool Register(Core::JCallableInterface<JResourceObject*, JDirectory*, const JResourcePathData&>* callable)
			{
				if constexpr (std::is_base_of_v <JResourceObject, Type >)
					return loadFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}
			template<typename Type>
			bool Register(Core::JCallableInterface<JResourceObject*, JResourceObject*>* callable)
			{
				if constexpr (std::is_base_of_v <JResourceObject, Type >)
					return copyFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}

			JResourceObject* Create(const std::string& typeName, JDirectory& ownerDir)
			{
				return defaultFactory.Invoke(typeName, &ownerDir);
			}
			JResourceObject* Create(const std::string& typeName,
				const std::string& name,
				size_t guid,
				JOBJECT_FLAG flag,
				JDirectory& ownerDir,
				uint8 formatIndex)
			{
				return initFactory.Invoke(typeName, name, std::move(guid), std::move(flag), &ownerDir, std::move(formatIndex));
			}
			JResourceObject* Create(const std::string& typeName, JDirectory& ownerDir, const JResourcePathData& pathData)
			{
				return loadFactory.Invoke(typeName, &ownerDir, pathData);
			}

			template<typename Type>
			Type* Create(JDirectory& ownerDir)
			{
				return static_cast<Type*>(defaultFactory.Invoke(Type::TypeName(), &ownerDir));
			}
			template<typename Type>
			Type* Create(const std::string& name,
				size_t guid,
				JOBJECT_FLAG flag,
				JDirectory& ownerDir,
				uint8 formatIndex)
			{
				return static_cast<Type*>(initFactory.Invoke(Type::TypeName(), name, std::move(guid), std::move(flag), &ownerDir, std::move(formatIndex)));
			}
			template<typename Type>
			Type* Load(JDirectory& ownerDir, const JResourcePathData& pathData)
			{
				return static_cast<Type*>(loadFactory.Invoke(Type::TypeName(), &ownerDir, pathData));
			}
			template<typename Type>
			Type* Copy(Type& ori)
			{
				return static_cast<Type*>(copyFactory.Invoke(Type::TypeName(), &ori));
			}
		};
	}

	using JRF = Core::JSingletonHolder<JResourceObjectFactory>;

	class JResourceObjectFactoryImplBase
	{
	private:
		friend class JResourceIO;
		friend class JResourceManager;
	protected:
		using DefaultPtr = JResourceObject * (*)(JDirectory*);
		using InitPtr = JResourceObject * (*)(const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*, const uint8);
		using LoadPtr = JResourceObject * (*)(JDirectory*, const JResourcePathData&);
		using CopytPtr = JResourceObject * (*)(JResourceObject*);

		using DefaultCallable = Core::JStaticCallable<JResourceObject*, JDirectory*>;
		using InitCallable = Core::JStaticCallable<JResourceObject*, const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*, const uint8>;
		using LoadCallable = Core::JStaticCallable<JResourceObject*, JDirectory*, const JResourcePathData&>;
		using CopyCallable = Core::JStaticCallable<JResourceObject*, JResourceObject*>;
	private:
		using AddStoragePtr = JResourceObject * (JResourceManager::*)(JResourceObject&);
		using AddStorageCallable = Core::JMemeberCallable<JResourceManager, JResourceObject*, JResourceObject&>;
	protected:
		static AddStorageCallable* addStorage;
	public:
		static JResourceObject* Create(const std::string& typeName, JDirectory& ownerDir)
		{
			JResourceObject* res = JRF::Instance().Create(typeName, ownerDir);
			(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
		static JResourceObject* Create(const std::string& typeName,
			const std::string& name,
			size_t guid,
			JOBJECT_FLAG flag,
			JDirectory& ownerDir,
			uint8 formatIndex)
		{
			JResourceObject* res = JRF::Instance().Create(typeName, name, guid, flag, ownerDir, formatIndex);
			(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
	private:
		static JResourceObject* LoadByName(const std::string& typeName, JDirectory& ownerDir, const JResourcePathData& pathData)
		{
			JResourceObject* res = JRF::Instance().Create(typeName, ownerDir, pathData);
			(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
	protected:
		template<typename T>
		static void Register(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr)
		{
			static DefaultCallable defaultCallable{ defaultPtr };
			static InitCallable initCallable{ initPtr };
			static LoadCallable loadCallable{ loadPtr };
			static CopyCallable copyCallable{ copytPtr };

			JRF::Instance().Register<T>(&defaultCallable);
			JRF::Instance().Register<T>(&initCallable);
			JRF::Instance().Register<T>(&loadCallable);
			JRF::Instance().Register<T>(&copyCallable);
		}
	private:
		static void RegistAddStroage(AddStoragePtr addPtr)
		{
			static AddStorageCallable addStorage{ addPtr };
			JResourceObjectFactoryImplBase::addStorage = &addStorage;
		}
	};

	template<typename T>
	class JResourceObjectFactoryImpl : public JResourceObjectFactoryImplBase
	{
	private:
		friend T;
		friend class JResourceIO;
		friend class JResourceManager;
	public:
		static T* Create(JDirectory& ownerDir)
		{
			T* res = JRF::Instance().Create<T>(ownerDir);
			if (res != nullptr)
				(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
		static T* Create(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory& ownerDir, const uint8 formatIndex)
		{
			T* res = JRF::Instance().Create<T>(name, guid, flag, ownerDir, formatIndex);
			if (res != nullptr)
				(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
		static T* Copy(T& ori)
		{
			T* res = JRF::Instance().Copy<T>(ori);
			if (res != nullptr)
				(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
	private:
		static T* Load(JDirectory& ownerDir, const JResourcePathData& pathData)
		{
			T* res = JRF::Instance().Load<T>(ownerDir, pathData);
			if (res != nullptr)
				(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		} 
		static void Register(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr)
		{
			JResourceObjectFactoryImplBase::Register<T>(defaultPtr, initPtr, loadPtr, copytPtr);
		}
	};

	template<>
	class JResourceObjectFactoryImpl<JShader> : public JResourceObjectFactoryImplBase
	{
	private:
		friend class JResourceIO;
		friend class JShader;
	private:
		using SetShaderFuncPtr = void(JShader::*)(const J_SHADER_FUNCTION);
		using SetShaderFuncCallable = Core::JMemeberCallable<JShader, void, const J_SHADER_FUNCTION>;
	private:
		static SetShaderFuncCallable* setShaderFunc;
	public:
		static JShader* Create(JDirectory& ownerDir, const J_SHADER_FUNCTION newFunc)
		{
			JShader* res = FindOverlapShader(newFunc);
			if (res == nullptr)
			{
				res = JRF::Instance().Create<JShader>(ownerDir);
				CallSetShdaer(res, newFunc);
				(*addStorage)(&JResourceManager::Instance(), *res);
			}
			return res;
		}
		static JShader* Create(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory& ownerDir, const uint8 formatIndex, J_SHADER_FUNCTION newFunc)
		{
			JShader* res = FindOverlapShader(newFunc);
			if (res == nullptr)
			{
				res = JRF::Instance().Create<JShader>(name, guid, flag, ownerDir, formatIndex);
				CallSetShdaer(res, newFunc);
				(*addStorage)(&JResourceManager::Instance(), *res);
			}
			return res;
		}
	private:
		static JShader* Load(JDirectory& ownerDir, const JResourcePathData& pathData)
		{
			JShader* res = JRF::Instance().Load<JShader>(ownerDir, pathData);
			if (res != nullptr)
				(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
		static JShader* FindOverlapShader(const J_SHADER_FUNCTION newFunc)
		{
			uint count;
			std::vector<JResourceObject*>::const_iterator st = JResourceManager::Instance().GetResourceVectorHandle<JShader>(count);
			for (uint i = 0; i < count; ++i)
			{
				JShader* shader = static_cast<JShader*>(*(st + i));
				if (newFunc == shader->GetShaderFunctionFlag())
					return shader;
			}
			return nullptr;
		}
	private:
		static void CallSetShdaer(JShader* shader, J_SHADER_FUNCTION func)
		{
			(*setShaderFunc)(shader, std::move(func));
		}
	private:
		static void Register(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr, SetShaderFuncPtr setShaderPtr)
		{
			JResourceObjectFactoryImplBase::Register<JShader>(defaultPtr, initPtr, loadPtr, copytPtr);
			static SetShaderFuncCallable setShaderFunc{ setShaderPtr };
			JResourceObjectFactoryImpl::setShaderFunc = &setShaderFunc;
		}	
	};

	template<>
	class JResourceObjectFactoryImpl<JSkeletonAsset> : public JResourceObjectFactoryImplBase
	{
	private:
		friend class JResourceIO;
		friend class JSkeletonAsset;
	private:
		using SetSkeletonPtr = void(JSkeletonAsset::*)(JSkeleton&& );
		using SetSkeletonCallable = Core::JMemeberCallable<JSkeletonAsset, void, JSkeleton&&>;
	private:
		static SetSkeletonCallable* setSkeleton;
	public:
		static JSkeletonAsset* Create(JDirectory& ownerDir, JSkeleton&& newSkeleton)
		{
			JSkeletonAsset* res = JRF::Instance().Create<JSkeletonAsset>(ownerDir);
			CallSetSkeleton(res, std::move(newSkeleton));
			(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
		static JSkeletonAsset* Create(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory& ownerDir, const uint8 formatIndex, JSkeleton&& newSkeleton)
		{
			JSkeletonAsset* res = JRF::Instance().Create<JSkeletonAsset>(name, guid, flag, ownerDir, formatIndex);
			CallSetSkeleton(res, std::move(newSkeleton));
			(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
		static JSkeletonAsset* Copy(JSkeletonAsset& ori)
		{
			JSkeletonAsset* res = JRF::Instance().Copy<JSkeletonAsset>(ori);
			if (res != nullptr)
				(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
	private:
		static JSkeletonAsset* Load(JDirectory& ownerDir, const JResourcePathData& pathData)
		{
			JSkeletonAsset* res = JRF::Instance().Load<JSkeletonAsset>(ownerDir, pathData);
			if (res != nullptr)
				(*addStorage)(&JResourceManager::Instance(), *res);
			return res;
		}
	private:
		static void CallSetSkeleton(JSkeletonAsset* skeletonAsset, JSkeleton&& newSkeleton)
		{
			(*setSkeleton)(skeletonAsset, std::move(newSkeleton));
		}
		static void RegisterFunc(SetSkeletonCallable& setSkeleton)
		{
			JResourceObjectFactoryImpl::setSkeleton = &setSkeleton;
		}

		static void Register(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr, SetSkeletonPtr setSkeletonPtr)
		{
			JResourceObjectFactoryImplBase::Register<JShader>(defaultPtr, initPtr, loadPtr, copytPtr);
			static SetSkeletonCallable setSkeleton{ setSkeletonPtr };
			JResourceObjectFactoryImpl::setSkeleton = &setSkeleton;
		}
	};
	 
	template<typename T>
	using JRFI = JResourceObjectFactoryImpl<T>;
}