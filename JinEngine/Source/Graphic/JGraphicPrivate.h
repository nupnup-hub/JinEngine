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
#include"JGraphicConstants.h"    
#include"Thread/JGraphicThreadInfo.h"

#include"../Object/GraphicRule/FrameResource/JGraphicModuleFrameResourceType.h"
#include"../Object/GraphicRule/Culling/JGraphicModuleCullingType.h"
#include"../Object/GraphicRule/GraphicResource/JGraphicModuleTextureResourceType.h"
#include"../Object/GraphicRule/GpuAccelerator/JGraphicModuleGpuAcceleratorType.h"

namespace JinEngine
{  
	class JMain; 
	namespace Editor
	{
		class JGraphicResourceWatcher;
	}
	namespace Graphic
	{
		class JGraphicResourceManager; 
		class JGraphicAdapter;
		class JGuiBackendDataAdapter;
		class JGuiBackendInterface; 
		struct JGuiInitData;  
		class JGraphicPrivate
		{
		public:
			class DebugInterface
			{
			private:
				friend class Editor::JGraphicResourceWatcher;
			private:
				static JGraphicResourceManager* GetGraphicResourceManager()noexcept;
			}; 
			class MainAccess
			{
			private:
				friend class JMain;
			private:
				//guiManagerInterface is deleted when app close
				static void Initialize(std::unique_ptr<JGraphicAdapter>&& adapter,
					std::unique_ptr<JGuiBackendDataAdapter>&& guiAdapter,
					JGuiBackendInterface* guiBackendInterface,
					const JGraphicThreadInfo& threadInfo);
				static void Clear();
			private:
				static std::unique_ptr<JGuiInitData> GetGuiInitData()noexcept;
			private:
				static void UpdateWait();  
				static void Update();
			private:  
				static void Draw(const bool allowDrawScene);
			private:
				static void FlushCommandQueue();
			private:
				static void WriteLastRsTexture();
			};
		};
	}
}