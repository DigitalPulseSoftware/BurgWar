// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CHATBOX_HPP
#define BURGWAR_CLIENTLIB_CHATBOX_HPP

#include <Nazara/Core/Signal.hpp>
#include <Nazara/Renderer/RenderWindow.hpp>
#include <NDK/Canvas.hpp>
#include <NDK/Entity.hpp>
#include <NDK/EntityOwner.hpp>
#include <NDK/Widgets/ScrollAreaWidget.hpp>
#include <NDK/Widgets/TextAreaWidget.hpp>

namespace bw
{
	class Chatbox
	{
		public:
			Chatbox(Nz::RenderWindow* window, Ndk::Canvas* canvas);
			Chatbox(const Chatbox&) = delete;
			Chatbox(Chatbox&&) = delete;
			~Chatbox();

			void Clear();
			inline void Close();

			inline bool IsOpen() const;
			inline bool IsTyping() const;

			void Open(bool shouldOpen = true);

			void PrintMessage(const std::string& message);

			Chatbox& operator=(const Chatbox&) = delete;
			Chatbox& operator=(Chatbox&&) = delete;

			NazaraSignal(OnChatMessage, const std::string& /*message*/);

		private:
			void OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget);

			NazaraSlot(Nz::RenderTarget, OnRenderTargetSizeChange, m_onTargetChangeSizeSlot);

			std::vector<Nz::String> m_chatLines;
			Ndk::ScrollAreaWidget* m_chatboxScrollArea;
			Ndk::TextAreaWidget* m_chatBox;
			Ndk::TextAreaWidget* m_chatEnteringBox;
	};
}

#include <ClientLib/Chatbox.inl>

#endif
