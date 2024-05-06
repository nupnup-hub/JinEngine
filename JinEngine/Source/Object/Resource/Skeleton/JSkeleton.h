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
#include"../../../Core/Animation/Joint.h" 
#include"../../../Core/JCoreEssential.h" 
#include"../../../Core/Reflection/JReflection.h"
#include<stdint.h> 

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
		const std::vector<Core::Joint> joint;
		const size_t skletonHash = 0;
	public:
		uint GetJointCount()const noexcept;
		size_t GetHash()const noexcept;
		Core::Joint GetJoint(int index)const noexcept;
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
		JSkeleton(std::vector<Core::Joint>&& joint);
		~JSkeleton();
		JSkeleton(const JSkeleton & skeleton) = delete;
		JSkeleton(JSkeleton && skeleton) = delete;
		JSkeleton& operator=(const JSkeleton& lhs) = delete;
		JSkeleton& operator=(JSkeleton && rhs) = delete;
	};
}