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