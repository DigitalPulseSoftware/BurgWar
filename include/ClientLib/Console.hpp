// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CONSOLE_HPP
#define BURGWAR_CLIENTLIB_CONSOLE_HPP

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Console.hpp>
#include <memory>

namespace bw
{
	class AbstractScriptingLibrary;

	class Console
	{
		public:
			Console(Nz::RenderWindow* window, Ndk::Canvas* canvas, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<VirtualDirectory>& scriptDir);
			Console(const Console&) = delete;
			Console(Console&&) = delete;
			~Console() = default;

			void Clear();

			inline bool IsVisible() const;

			inline void Hide();

			void Show(bool shouldShow);

			Console& operator=(const Console&) = delete;
			Console& operator=(Console&&) = delete;

		private:
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			std::shared_ptr<ScriptingContext> m_scriptingContext;
			Ndk::Console* m_widget;
	};
}

#include <ClientLib/Console.inl>

#endif
