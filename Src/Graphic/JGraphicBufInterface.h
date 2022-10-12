#pragma once
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicBufManagerInterface;
		class JGraphicBufElementInterface
		{
		private:
			friend class JGraphicBufUserInterface;
			friend class JGraphicBufManagerInterface;
		private:
			int buffIndex;
		public:
			virtual ~JGraphicBufElementInterface() = default;
		protected:
			int GetGraphicBuffIndex()const noexcept;
			void SetGraphicBuffIndex(const int value)noexcept;
		};

		class JGraphicBufUserInterface
		{
		public:
			virtual ~JGraphicBufUserInterface() = default;
		protected:
			int GetGraphicBuffIndex(JGraphicBufElementInterface& bufEleInterface)const noexcept;
		};
		class JGraphicBufManagerInterface : public JGraphicBufUserInterface
		{
		protected:
			void SetGraphicBuffIndex(JGraphicBufElementInterface& bufEleInterface, int index)noexcept;
		};
	}
}