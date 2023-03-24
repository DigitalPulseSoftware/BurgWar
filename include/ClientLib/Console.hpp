// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CONSOLE_HPP
#define BURGWAR_CLIENTLIB_CONSOLE_HPP

#include <ClientLib/Export.hpp>
#include <NazaraUtils/Signal.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <Nazara/Widgets/Canvas.hpp>
#include <functional>
#include <memory>

namespace bw
{
	class BURGWAR_CLIENTLIB_API Console
	{
		public:
			using ExecuteCallback = std::function<bool(const std::string& command)>;

			Console(Nz::RenderTarget* window, Nz::Canvas* canvas);
			Console(const Console&) = delete;
			Console(Console&&) = delete;
			~Console();

			void Clear();

			inline void Hide();

			inline bool IsVisible() const;

			void Print(const std::string& str, Nz::Color color = Nz::Color::White);

			void SetExecuteCallback(ExecuteCallback callback);
			void Show(bool shouldShow);

			Console& operator=(const Console&) = delete;
			Console& operator=(Console&&) = delete;

		private:
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			ExecuteCallback m_callback;
			//Nz::Console* m_widget;
	};
}

#include <ClientLib/Console.inl>

#endif
