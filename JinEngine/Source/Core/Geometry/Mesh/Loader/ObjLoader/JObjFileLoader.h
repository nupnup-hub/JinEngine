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
#include"JObjFileLoaderUtility.h" 
#include"../../../../Singleton/JSingletonHolder.h"   
 
struct DirectX::BoundingBox;
struct DirectX::BoundingSphere;

namespace JinEngine
{ 
	namespace Core
	{
		struct JFileImportPathData;
		using ObjMaterialMap = std::unordered_map<size_t, JObjFileMaterial>;
		using ObjMaterialRefMap = std::unordered_map<std::wstring, JObjFileMaterial&>;
		using ObjMaterialVec = std::vector<JObjFileMaterial>;

		class JObjFileLoader
		{
		private:
			struct FaceInfo
			{
			public:
				std::vector<JVector3<int>> ptn;
				bool hasUV;
				bool hasNormal;
			public:
				FaceInfo(const std::vector<JVector3<int>>& ptn);
			};
			struct LoadDataSet
			{
			public:
				ObjMaterialVec materialVec;		//store material
				ObjMaterialRefMap materialMap;	//store material ref key is material name
			public: 
				std::vector<std::wstring> meshName;
				std::vector<std::wstring> materialName;
				std::vector<JVector3<float>> position;
				std::vector<JVector2<float>> texture;
				std::vector<JVector3<float>> normal;
				std::vector<uint> index;
				std::vector<uint> vertexCount;
				std::vector<uint> indexCount;
				std::vector<std::vector<FaceInfo>> faceInfo;
				std::unordered_map<size_t, uint> vertexIndexMap;
			public:
				std::wstring materialLibName;
			public:
				int meshCount = 0;
				int faceCount = 0;
			public:
				bool hasMaterial = false;
			};
		private:
			std::unique_ptr<LoadDataSet> set;
		public:
			bool LoadObjFile(const JFileImportPathData& pathData, JStaticMeshGroup& meshGroup, ObjMaterialMap& objMatData);
		private:
			bool LoadMesh(const JFileImportPathData& pathData);
			bool LoadMaterial();
			bool CreateMeshGroup(JStaticMeshGroup& meshGroup, ObjMaterialMap& objMatData);
		private:
			void GetVectorIndex(const std::wstring& wstr, int& posIndex, int& uvIndex, int& normalIndex)const noexcept;
			std::wstring GetMaterialPath(const std::wstring& folderPath, const std::wstring& name)const noexcept;
		private:
			void DetectNan(float& x, float& y);
			void DetectNan(float& x, float& y, float& z);
		private:
			JVector3<float> SetLeftHand(const JVector3<float>& v)noexcept;
			size_t MakeVertexGuid(const uint x, const uint y, const uint z);
		};
	}
	using JObjFileLoader = Core::JSingletonHolder<Core::JObjFileLoader>;
}