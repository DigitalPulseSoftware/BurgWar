// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CHATBOX_HPP
#define BURGWAR_CLIENTLIB_CHATBOX_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Renderer/RenderTarget.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Entity.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/Widgets/RichTextAreaWidget.hpp>
#include <NDK/Widgets/ScrollAreaWidget.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>
#include <variant>

namespace bw
{
	class Logger;

	class Chatbox
	{
		public:
			struct ColorItem
			{
				Nz::Color color;
			};

			struct TextItem
			{
				std::string text;
			};

			using Item = std::variant<ColorItem, TextItem>;

			Chatbox(const Logger& logger, Nz::RenderTarget* rt, Ndk::Canvas* canvas);
			Chatbox(const Chatbox&) = delete;
			Chatbox(Chatbox&&) = delete;
			~Chatbox();

			void Clear();
			inline void Close();

			inline bool IsOpen() const;
			inline bool IsTyping() const;

			void Open(bool shouldOpen = true);

			void PrintMessage(std::vector<Item> message);

			Chatbox& operator=(const Chatbox&) = delete;
			Chatbox& operator=(Chatbox&&) = delete;

			NazaraSignal(OnChatMessage, const std::string& /*message*/);

		private:
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);
			void Refresh();

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			std::vector<std::vector<Item>> m_chatLines;
			Ndk::ScrollAreaWidget* m_chatboxScrollArea;
			Ndk::RichTextAreaWidget* m_chatBox;
			Ndk::TextAreaWidget* m_chatEnteringBox;
			const Logger& m_logger;
	};
}

#include <ClientLib/Chatbox.inl>

#endif
