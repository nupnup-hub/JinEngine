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


#include"JSkeleton.h"
#include"JSkeletonFixedData.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	using namespace DirectX;
	//JSkeleton::JSkeleton(const JSkeleton& skeleton)
	///	:joint(skeleton.joint), jointCount((uint)joint.size()), skletonHash(skeleton.skletonHash)
	//{}
	uint JSkeleton::GetJointCount()const noexcept
	{ 
		return (uint)joint.size();
	}
	size_t JSkeleton::GetHash()const noexcept
	{
		return skletonHash;
	}
	Core::Joint JSkeleton::GetJoint(int index)const noexcept
	{
		return joint[index];
	}
	std::wstring JSkeleton::GetJointName(int index)const noexcept
	{
		if (index < 0 && index >= (int)joint.size())
			return std::wstring{};
		else
			return joint[index].name;
	}
	uint8 JSkeleton::GetJointParentIndex(int index)const noexcept
	{
		if (index < 0 && index >= (int)joint.size())
			return JSkeletonFixedData::incorrectJointIndex;
		else
			return joint[index].parentIndex;
	}
	DirectX::XMMATRIX JSkeleton::GetInBindPose(int index)const noexcept
	{
		return joint[index].inbindPose.LoadXM();
	}
	DirectX::XMMATRIX JSkeleton::GetBindPose(int index)const noexcept
	{
		return DirectX::XMMatrixInverse(nullptr, joint[index].inbindPose.LoadXM());
	}
	JVector4<float> JSkeleton::GetInBindQuaternion(int index)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t; 
		DirectX::XMMatrixDecompose(&s, &q, &t, joint[index].inbindPose.LoadXM());
		return DirectX::XMQuaternionNormalize(q);
	}
	JVector4<float> JSkeleton::GetBindQuaternion(int index)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t; 
		DirectX::XMMatrixDecompose(&s, &q, &t, DirectX::XMMatrixInverse(nullptr, joint[index].inbindPose.LoadXM()));
		return DirectX::XMQuaternionNormalize(q);
	}
	void JSkeleton::GetBindTQS(int index, JVector3<float>& outT, JVector4<float>& outQ, JVector3<float>& outS)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t;
		 
		DirectX::XMMatrixDecompose(&s, &q, &t, DirectX::XMMatrixInverse(nullptr, joint[index].inbindPose.LoadXM()));
		outT = t;
		outQ = q;
		outS = s; 
	}
	JVector4<float> JSkeleton::GetBindT(int index)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t;
		 
		DirectX::XMMatrixDecompose(&s, &q, &t, DirectX::XMMatrixInverse(nullptr, joint[index].inbindPose.LoadXM()));
		return t;
	}
	float JSkeleton::GetJointLength(int index)const noexcept
	{
		return joint[index].length;
	}
	bool JSkeleton::IsSame(const JSkeleton& skeleton)const noexcept
	{
		return skletonHash == skeleton.skletonHash;
	}
	bool JSkeleton::IsSame(const size_t skletonHash)const noexcept
	{ 
		return JSkeleton::skletonHash == skletonHash;
	}
	size_t JSkeleton::GetSkeletonHash()const noexcept
	{
		std::wstring totalName;
		const uint jointCount = (uint)joint.size();
		for (uint i = 0; i < jointCount; ++i)
			totalName += joint[i].name;
		return std::hash<std::wstring>{}(totalName);
	}
	JSkeleton::JSkeleton(std::vector<Core::Joint>&& joint)
		:joint(std::move(joint)), skletonHash(GetSkeletonHash())
	{ }
	JSkeleton::~JSkeleton() {}
}
/*
		if (skeleton.jointCount != jointCount)
			return false;

		for (int i = 0; i < jointCount; ++i)
		{
			if (skeleton.joint[i].name != joint[i].name)
				return false;

			if (skeleton.joint[i].length != joint[i].length)
				return false;

			if (skeleton.joint[i].parentIndex != joint[i].parentIndex)
				return false;

			if (!JMathHelper::IsSame(skeleton.joint[i].inbindPose, joint[i].inbindPose))
				return false;
		}
		return true;
*/