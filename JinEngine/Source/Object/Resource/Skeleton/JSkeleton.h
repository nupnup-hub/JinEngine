#pragma once 
#include<stdint.h>
#include<vector>
#include"Joint.h" 
#include"../../../Core/JCoreEssential.h" 
#include"../../../Core/Reflection/JReflection.h"

namespace JinEngine
{  
	class JMeshGeometry;
	class JSkeletonAsset;
	struct JSkeleton
	{ 
		REGISTER_CLASS_USE_ALLOCATOR(JSkeleton)
	private:
		friend class JMeshGeometry;
		friend class JSkeletonAsset;
	private:
		JWeakPtr<JSkeletonAsset> ownerSkeleton;
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
		JVector4<float> GetInBindQuaternion(int index)const noexcept;
		JVector4<float> GetBindQuaternion(int index)const noexcept;
		void GetBindTQS(int index, JVector3<float>& outT, JVector4<float>& outQ, JVector3<float>& outS)const noexcept;
		JVector4<float> GetBindT(int index)const noexcept;
		float GetJointLength(int index)const noexcept;
	public:
		bool IsSame(const JSkeleton& skeleton)const noexcept;
		bool IsSame(const size_t skletonHash)const noexcept;
	private:
		size_t GetSkeletonHash()const noexcept;
	private: 
		//JSkeleton() = default;
		JSkeleton(std::vector<Joint>&& joint);
		~JSkeleton();
		JSkeleton(const JSkeleton & skeleton) = delete;
		JSkeleton(JSkeleton && skeleton) = delete;
		JSkeleton& operator=(const JSkeleton& lhs) = delete;
		JSkeleton& operator=(JSkeleton && rhs) = delete;
	};
}