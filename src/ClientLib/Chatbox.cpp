// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Chatbox.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <Nazara/Utility/Font.hpp>
#include <NDK/Widgets.hpp>

namespace bw
{
	static constexpr std::size_t maxChatLines = 100;

	Chatbox::Chatbox(const Logger& logger, Nz::RenderWindow* window, Ndk::Canvas* canvas) :
	m_chatEnteringBox(nullptr),
	m_logger(logger)
	{
		m_chatBox = canvas->Add<Ndk::RichTextAreaWidget>();
		m_chatBox->EnableBackground(false);
		m_chatBox->EnableLineWrap(true);
		m_chatBox->SetBackgroundColor(Nz::Color(0, 0, 0, 50));
		m_chatBox->SetCharacterSize(22);
		m_chatBox->SetTextColor(Nz::Color::White);
		m_chatBox->SetTextOutlineColor(Nz::Color::Black);
		m_chatBox->SetTextOutlineThickness(1.f);
		m_chatBox->SetReadOnly(true);

		m_chatboxScrollArea = canvas->Add<Ndk::ScrollAreaWidget>(m_chatBox);
		m_chatboxScrollArea->Resize({ 480.f, 0.f });
		m_chatboxScrollArea->EnableScrollbar(false);

		m_chatEnteringBox = canvas->Add<Ndk::TextAreaWidget>();
		m_chatEnteringBox->EnableBackground(true);
		m_chatEnteringBox->SetBackgroundColor(Nz::Color(255, 255, 255, 150));
		m_chatEnteringBox->SetTextColor(Nz::Color::Black);
		m_chatEnteringBox->Show(false);

		// Connect every slot
		m_onTargetChangeSizeSlot.Connect(window->OnRenderTargetSizeChange, this, &Chatbox::OnRenderTargetSizeChange);

		OnRenderTargetSizeChange(window);
	}

	Chatbox::~Chatbox()
	{
		m_chatboxScrollArea->Destroy();

		if (m_chatEnteringBox)
			m_chatEnteringBox->Destroy();
	}

	void Chatbox::Clear()
	{
		m_chatLines.clear();
		m_chatBox->Clear();
	}

	void Chatbox::Open(bool shouldOpen)
	{
		if (IsOpen() != shouldOpen)
		{
			if (shouldOpen)
			{
				m_chatBox->EnableBackground(true);
				m_chatboxScrollArea->EnableScrollbar(true);
				m_chatEnteringBox->Show(true);
				m_chatEnteringBox->SetFocus();
			}
			else
			{
				Nz::String text = m_chatEnteringBox->GetText();
				m_chatBox->EnableBackground(false);
				m_chatboxScrollArea->EnableScrollbar(false);
				m_chatEnteringBox->Clear();
				m_chatEnteringBox->Show(false);

				if (!text.IsEmpty())
					OnChatMessage(text.ToStdString());
			}
		}
	}

	void Chatbox::PrintMessage(std::vector<Item> message)
	{
		auto logContext = m_logger.PushContext();
		logContext->level = LogLevel::Info;
		if (m_logger.ShouldLog(*logContext))
		{
			std::string textMessage;

			for (const Item& messageItem : message)
			{
				std::visit([&](auto&& item)
				{
					using T = std::decay_t<decltype(item)>;

					if constexpr (std::is_same_v<T, TextItem>)
						textMessage += item.text;

				}, messageItem);
			}

			m_logger.LogFormat(*logContext, "{0}", textMessage);
		}

		m_chatLines.emplace_back(std::move(message));
		if (m_chatLines.size() > maxChatLines)
			m_chatLines.erase(m_chatLines.begin());

		Refresh();
	}

	void Chatbox::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		Nz::Vector2f size = Nz::Vector2f(renderTarget->GetSize());

		m_chatEnteringBox->Resize({ size.x, 40.f });
		m_chatEnteringBox->SetPosition({ 0.f, size.y - m_chatEnteringBox->GetHeight() - 5.f, 0.f });
		m_chatboxScrollArea->Resize({ size.x / 3.f, size.y / 3.f });
		m_chatboxScrollArea->SetPosition({ 5.f, m_chatEnteringBox->GetPosition().y - m_chatboxScrollArea->GetHeight() - 5, 0.f });
	}
	
	void Chatbox::Refresh()
	{
		m_chatBox->Clear();
		for (const auto& lineItems : m_chatLines)
		{
			for (const Item& lineItem : lineItems)
			{
				std::visit([&](auto&& item)
				{
					using T = std::decay_t<decltype(item)>;

					if constexpr (std::is_same_v<T, ColorItem>)
					{
						m_chatBox->SetTextColor(item.color);
					}
					else if constexpr (std::is_same_v<T, TextItem>)
					{
						if (!item.text.empty())
							m_chatBox->AppendText(item.text);
					}
					else
						static_assert(AlwaysFalse<T>::value, "non-exhaustive visitor");

				}, lineItem);
			}

			m_chatBox->SetTextColor(Nz::Color::White);
			m_chatBox->AppendText("\n");
		}

		m_chatBox->Resize({ m_chatBox->GetWidth(), m_chatBox->GetPreferredHeight() });
		m_chatboxScrollArea->Resize(m_chatboxScrollArea->GetSize()); // force layout update
		m_chatboxScrollArea->SetPosition({ 5.f, m_chatEnteringBox->GetPosition().y - m_chatboxScrollArea->GetHeight() - 5, 0.f });

		m_chatboxScrollArea->ScrollToRatio(1.f);
	}
}
