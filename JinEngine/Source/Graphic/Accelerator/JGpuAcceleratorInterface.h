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