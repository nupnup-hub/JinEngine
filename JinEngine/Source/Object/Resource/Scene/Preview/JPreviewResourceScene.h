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
#include"JPreviewScene.h"  

namespace JinEngine
{
	class JResourceObject; 
	class JPreviewResourceScene : public JPreviewScene
	{
	public: 
		JPreviewResourceScene(_In_ JUserPtr<JResourceObject> resource, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewResourceScene();
		JPreviewResourceScene(JPreviewResourceScene&& rhs) = default;
		JPreviewResourceScene& operator=(JPreviewResourceScene && rhs) = default;
	public:
		bool Initialze()noexcept; 
	private:
		bool MakeMeshPreviewScene()noexcept;
		bool MakeMaterialPreviewScene()noexcept;
		bool MakeUserTexturePreviewScene()noexcept;
		bool MakeEditorTexturePreviewScene(const J_DEFAULT_TEXTURE editorTextureType)noexcept;
	};
}