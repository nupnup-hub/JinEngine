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


#pragma once
#include"JGraphicModuleCsmOption.h"
#include"../../../Core/Func/Functor/JFunctor.h"
#include"../../../Core/Math/JMatrix.h" 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JCsmTargetInterface; 
	class JObject;
	using GetCsmTargetBoundingFrustumF = Core::JSFunctorType<DirectX::BoundingFrustum, JUserPtr<JObject>>;
	using GetCsmTargetBoundingFrustumB = Core::JBindHandle<GetCsmTargetBoundingFrustumF::Functor, JUserPtr<JObject>>;
	 
	using NotifyAddCsmTargetF = Core::JSFunctorType<void, JUserPtr<JObject>, int>;			// obj, index
	using NotifySubtractCsmTargetF = Core::JSFunctorType<void, JUserPtr<JObject>, int>;		// obj, index
	using NotifyAddCsmTargetB = Core::JBindHandle<NotifyAddCsmTargetF::Functor, JUserPtr<JObject>, Core::JEmptyType>;
	using NotifySubtractCsmTargetB = Core::JBindHandle<NotifySubtractCsmTargetF::Functor, JUserPtr<JObject>, Core::JEmptyType>;
	
	using GetCsmTargetBoundingFrustumBindPtr = std::unique_ptr<GetCsmTargetBoundingFrustumB>;
	using NotifyAddCsmTargetBindPtr = std::unique_ptr<NotifyAddCsmTargetB>;
	using NotifySubtractCsmTargetBindPtr = std::unique_ptr<NotifySubtractCsmTargetB>;

	struct JCsmHandleCreationDesc
	{
	public:
		const size_t handleGuid;
		const size_t areaGuid;
	public:
		NotifyAddCsmTargetBindPtr notifyAddCsmTargetB;
		NotifySubtractCsmTargetBindPtr notifySubtractCsmTargetB;
	public:
		JCsmHandleCreationDesc(const size_t handleGuid,
			const size_t areaGuid,
			NotifyAddCsmTargetBindPtr&& notifyAddCsmTargetB = nullptr,
			NotifySubtractCsmTargetBindPtr&& notifySubtractCsmTargetB = nullptr)
			:handleGuid(handleGuid),
			areaGuid(areaGuid),
			notifyAddCsmTargetB(std::move(notifyAddCsmTargetB)),
			notifySubtractCsmTargetB(std::move(notifySubtractCsmTargetB))
		{}
	};

	struct JCsmTargetCreationDesc
	{
	public:
		const size_t targetGuid;
		const size_t areaGuid;
	public:
		GetCsmTargetBoundingFrustumBindPtr getBoundingFrustumB;
	public:
		JCsmTargetCreationDesc(const size_t targetGuid, const size_t areaGuid, GetCsmTargetBoundingFrustumBindPtr&& getBoundingFrustumB)
			:targetGuid(targetGuid), areaGuid(areaGuid), getBoundingFrustumB(std::move(getBoundingFrustumB))
		{}
	};

	struct JCsmComputeResult
	{
	public:
		JMatrix4x4 shadowProjM[JCsmOption::maxCountOfSplit];
		JVector4<float> scale[JCsmOption::maxCountOfSplit];
		JVector4<float> posOffset[JCsmOption::maxCountOfSplit];
		//JVector2<float> frustumSize[JCsmOption::maxCountOfSplit];
		float splitRate[JCsmOption::maxCountOfSplit];
		float fNear[JCsmOption::maxCountOfSplit];
		float fFar[JCsmOption::maxCountOfSplit];
	public:
		uint subFrustumCount;
	};
}