#include"JResourceObjectFactory.h"

namespace JinEngine
{
	JResourceObjectFactoryImplBase::AddStorageCallable* JResourceObjectFactoryImplBase::addStorage;
	JResourceObjectFactoryImpl<JShader>::SetShaderFuncCallable* JResourceObjectFactoryImpl<JShader>::setShaderFunc;
	JResourceObjectFactoryImpl<JSkeletonAsset>::SetSkeletonCallable* JResourceObjectFactoryImpl<JSkeletonAsset>::setSkeleton;
	JResourceObjectFactoryImpl<JSkeletonAsset>::SetModelGuidCallable* JResourceObjectFactoryImpl<JSkeletonAsset>::setModelGuid;
}