// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CONSOLE_HPP
#define BURGWAR_CLIENTLIB_CONSOLE_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Console.hpp>
#include <functional>
#include <memory>

namespace bw
{
	class Console
	{
		public:
			using ExecuteCallback = std::function<bool(const std::string& command)>;

			Console(Nz::RenderWindow* window, Ndk::Canvas* canvas);
			Console(const Console&) = delete;
			Console(Console&&) = delete;
			~Console() = default;

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
			Ndk::Console* m_widget;
	};
}

#include <ClientLib/Console.inl>

#endif
