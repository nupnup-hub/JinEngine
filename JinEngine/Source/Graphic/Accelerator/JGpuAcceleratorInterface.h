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
#include"JGpuAcceleratorType.h"
#include"../../Core/Reflection/JReflection.h"
#include"../../Core/Reflection//JTypeImplBase.h"

namespace JinEngine
{ 
	class JComponent;
	namespace Graphic
	{
		class JGpuAcceleratorInfo;
		class JGpuAcceleratorInterface : public Core::JTypeImplInterfacePointerHolder<JGpuAcceleratorInterface>
		{
		private:
			JUserPtr<JGpuAcceleratorInfo> info;
		public:
			virtual ~JGpuAcceleratorInterface() = default;
		protected:
			bool CreateGpuAccelerator(const JGpuAcceleratorBuildDesc& desc);
			bool DestroyGpuAccelerator();
		protected:
			void UpdateTransform(const JUserPtr<JComponent>& comp);
			void AddComponent(const JUserPtr<JComponent>& comp);
			void RemoveComponent(const JUserPtr<JComponent>& comp);
		public:
			int GetArrayIndex()const noexcept;
			J_GPU_ACCELERATOR_BUILD_OPTION GetBuildOption()const noexcept;
		public:
			bool HasInfo()const noexcept;
		protected: 
			static bool CanBuildGpuAccelerator()noexcept;
		};

		using JGpuAcceleratorInterfacePointer = Core::JTypeImplInterfacePointer<JGpuAcceleratorInterface>;
		class JGpuAcceleratorUserInterface final
		{
		private:
			JUserPtr<JGpuAcceleratorInterfacePointer> gPtrWrapper = nullptr;
		public:
			JGpuAcceleratorUserInterface() = default;
			JGpuAcceleratorUserInterface(JGpuAcceleratorInterface* gInterface);
			~JGpuAcceleratorUserInterface() = default;
		public:
			int GetArrayIndex()const noexcept;
			J_GPU_ACCELERATOR_BUILD_OPTION GetBuildOption()const noexcept;
		public:
			bool HasInfo()const noexcept;
		};

		class JGpuAcceleratorUserAccess
		{
		protected:
			JGpuAcceleratorUserAccess() = default;
			virtual ~JGpuAcceleratorUserAccess() = default;
		public:
			virtual const JGpuAcceleratorUserInterface GpuAcceleratorUserInterface()const noexcept = 0;
		};
	}
}