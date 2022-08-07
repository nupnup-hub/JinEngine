#include"JSkeleton.h"
#include"JSkeletonFixedData.h"
#include"../../../Utility/JMathHelper.h"
#include"../../../Utility/JCommonUtility.h"

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
	Joint JSkeleton::GetJoint(int index)const noexcept
	{
		return joint[index];
	}
	std::string JSkeleton::GetJointName(int index)const noexcept
	{
		if (index < 0 && index >= (int)joint.size())
			return " ";
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
		return DirectX::XMLoadFloat4x4(&joint[index].inbindPose);
	}
	DirectX::XMMATRIX JSkeleton::GetBindPose(int index)const noexcept
	{
		return DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&joint[index].inbindPose));
	}
	DirectX::XMFLOAT4 JSkeleton::GetInBindQuaternion(int index)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t;
		DirectX::XMMATRIX bindM = DirectX::XMLoadFloat4x4(&joint[index].inbindPose);
		DirectX::XMMatrixDecompose(&s, &q, &t, bindM);
		XMFLOAT4 outQ;
		DirectX::XMStoreFloat4(&outQ, DirectX::XMQuaternionNormalize(q));
		return outQ;
	}
	DirectX::XMVECTOR JSkeleton::GetBindQuaternion(int index)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t;
		DirectX::XMMATRIX bindM = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&joint[index].inbindPose));
		DirectX::XMMatrixDecompose(&s, &q, &t, bindM);
		return DirectX::XMQuaternionNormalize(q);
	}
	void JSkeleton::GetBindTQS(int index, XMFLOAT3& outT, XMFLOAT4& outQ, XMFLOAT3& outS)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t;

		XMMATRIX bindM = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&joint[index].inbindPose));
		DirectX::XMMatrixDecompose(&s, &q, &t, bindM);
		DirectX::XMStoreFloat3(&outT, t);
		DirectX::XMStoreFloat4(&outQ, q);
		DirectX::XMStoreFloat3(&outS, s);
	}
	DirectX::XMVECTOR JSkeleton::GetBindT(int index)const noexcept
	{
		XMVECTOR s;
		XMVECTOR q;
		XMVECTOR t;

		XMMATRIX bindM = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&joint[index].inbindPose));
		DirectX::XMMatrixDecompose(&s, &q, &t, bindM);

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
	JSkeleton::JSkeleton(std::vector<Joint>&& joint, const size_t skletonHash)
		:joint(std::move(joint)), skletonHash(skletonHash)
	{}
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