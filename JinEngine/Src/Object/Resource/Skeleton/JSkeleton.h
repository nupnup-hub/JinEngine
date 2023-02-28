#pragma once 
#include<stdint.h>
#include<vector>
#include"Joint.h"
#include"../../../Core/JDataType.h" 

namespace JinEngine
{  
	struct JSkeleton
	{ 
	private: 
		const std::vector<Joint> joint;
		const size_t skletonHash = 0;
	public:
		uint GetJointCount()const noexcept;
		size_t GetHash()const noexcept;
		Joint GetJoint(int index)const noexcept;
		std::wstring GetJointName(int index)const noexcept;
		uint8 GetJointParentIndex(int index)const noexcept;
		DirectX::XMMATRIX GetInBindPose(int index)const noexcept;
		DirectX::XMMATRIX GetBindPose(int index)const noexcept;
		DirectX::XMFLOAT4 GetInBindQuaternion(int index)const noexcept;
		DirectX::XMVECTOR GetBindQuaternion(int index)const noexcept;
		void GetBindTQS(int index, DirectX::XMFLOAT3& outT, DirectX::XMFLOAT4& outQ, DirectX::XMFLOAT3& outS)const noexcept;
		DirectX::XMVECTOR GetBindT(int index)const noexcept;
		float GetJointLength(int index)const noexcept;
	public:
		bool IsSame(const JSkeleton& skeleton)const noexcept;
		bool IsSame(const size_t skletonHash)const noexcept;
	private:
		size_t GetSkeletonHash()const noexcept;
	public: 
		//JSkeleton() = default;
		JSkeleton(std::vector<Joint> && joint);
		~JSkeleton();

		JSkeleton(const JSkeleton & skeleton) = default;
		JSkeleton& operator=(const JSkeleton & lhs) = default;

		JSkeleton(JSkeleton && skeleton) = default;
		JSkeleton& operator=(JSkeleton && rhs) = default;
	};
}