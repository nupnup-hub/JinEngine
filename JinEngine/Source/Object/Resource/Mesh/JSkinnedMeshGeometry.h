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
#include"JMeshGeometry.h"

namespace JinEngine
{ 
	class JSkeletonAsset;
	class JSkinnedMeshGeometryPrivate;
	class JSkinnedMeshGeometry final : public JMeshGeometry
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JSkinnedMeshGeometry) 
	public: 
		class InitData : public JMeshGeometry::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData) 
		private:
			friend class JSkinnedMeshGeometry;
		private:
			JUserPtr<JSkeletonAsset> skeletonAsset;
		public:
			InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory, std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup);
			InitData(const size_t guid,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory,
				std::unique_ptr<Core::JMeshGroup>&& skinnedMeshGroup);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadMetaData final : public JMeshGeometry::LoadMetaData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadMetaData)
		public:
			Core::J_MESHGEOMETRY_TYPE meshType;
		public:
			LoadMetaData(const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JSkinnedMeshGeometryPrivate;
		class JSkinnedMeshGeometryImpl;
	private:
		std::unique_ptr<JSkinnedMeshGeometryImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		Core::J_MESHGEOMETRY_TYPE GetMeshGeometryType()const noexcept final; 
		JUserPtr<JSkeletonAsset> GetSkeletonAsset()const noexcept;
	private:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;   
	private:
		JSkinnedMeshGeometry(InitData& initData);
		~JSkinnedMeshGeometry();
	};
}
 