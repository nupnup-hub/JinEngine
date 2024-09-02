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
#include"../JComponentPrivate.h"

namespace JinEngine
{ 
	class JLight;  	 
	class JLightPrivate : public JComponentPrivate
	{
	public:
		class AssetDataIOInterface : public JComponentPrivate::AssetDataIOInterface
		{
		protected:
			Core::J_FILE_IO_RESULT LoadLightData(JFileIOTool& tool, JUserPtr<JLight> user);
			Core::J_FILE_IO_RESULT StoreLightData(JFileIOTool& tool, const JUserPtr<JLight>& user);
		};
		class CreateInstanceInterface : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
		protected:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept override;
		};
		class DestroyInstanceInterface : public JComponentPrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced)override;
		};
	public:  
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override; 
	};
}