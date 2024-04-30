#pragma once 
#include"JOccCulling.h"
#include"../../Shader/JShaderDataHandler.h"
#include"../../JGraphicSubClassInterface.h"
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicInfo;
		struct JGraphicOption;
		class JCullingInfo;

		/**
		* Hzb�� IsOccluder trigger�� ���� object�� ������ ����� Occlude�Ѵ�.
		*/
		class JHZBOccCulling : public JOccCulling, public JGraphicSubClassInterface
		{ 
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			virtual bool CanReadBackDebugInfo()const noexcept = 0; 
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void NotifyBuildNewHzbOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo) = 0;
			virtual void NotifyReBuildHzbOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo) = 0;
			virtual void NotifyDestroyHzbOccBuffer(JCullingInfo* cullingInfo) = 0;
		public:
			//Set upload buffer and shader macro
			virtual void ReBuildOcclusionMapInfoConstants(JGraphicDevice* device, const JGraphicInfo& info) = 0;	//������ occlusion map option �����ϴ� �������̽� �߰��ʿ�
		public:
			virtual void StreamOutDebugInfo(const JUserPtr<JCullingInfo>& cullingInfo, const std::wstring& path) = 0;
			virtual void StreamOutDebugInfo(const std::wstring& path) = 0;
		public:
			virtual void DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) = 0;
			virtual void DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) = 0;
		public:
			virtual void ComputeOcclusionCulling(const JGraphicHzbOccComputeSet* computeSet, const JDrawHelper& helper) = 0;
		};
	}
}