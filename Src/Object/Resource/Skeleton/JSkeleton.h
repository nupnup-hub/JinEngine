#pragma once 
#include<stdint.h>
#include<vector>
#include"Joint.h"
#include"../../../Core/JDataType.h" 

namespace JinEngine
{
	namespace Core
	{
		class JFbxFileLoaderImpl;
	}

	struct JSkeleton
	{
		friend class Core::JFbxFileLoaderImpl;
	private:
		const uint jointCount = 0;
		const std::vector<Joint> joint;
		const size_t skletonHash;
	public:
		JSkeleton() = default;
		JSkeleton(std::vector<Joint>&& joint, const size_t nameHash);
		JSkeleton(const JSkeleton& skeleton);

		uint GetJointCount()const noexcept;
		size_t GetHash()const noexcept;
		Joint GetJoint(int index)const noexcept;
		std::string GetJointName(int index)const noexcept;
		uint8 GetJointParentIndex(int index)const noexcept;
		DirectX::XMMATRIX GetInBindPose(int index)const noexcept;
		DirectX::XMMATRIX GetBindPose(int index)const noexcept;
		DirectX::XMFLOAT4 GetInBindQuaternion(int index)const noexcept;
		DirectX::XMVECTOR GetBindQuaternion(int index)const noexcept;
		void GetBindTQS(int index, DirectX::XMFLOAT3& outT, DirectX::XMFLOAT4& outQ, DirectX::XMFLOAT3& outS)const noexcept;
		DirectX::XMVECTOR GetBindT(int index)const noexcept;
		float GetJointLength(int index)const noexcept;

		bool IsSame(const JSkeleton& skeleton)const noexcept;
		bool IsSame(const size_t skletonHash)const noexcept;
	private: 
		//static size_t CalSkeletonHash(const std::vector<Joint>& joint)noexcept;
	};
}