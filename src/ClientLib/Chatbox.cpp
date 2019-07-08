// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Chatbox.hpp>
#include <Nazara/Utility/Font.hpp>
#include <NDK/Widgets.hpp>
#include <iostream>

namespace bw
{
	static constexpr std::size_t maxChatLines = 100;

	Chatbox::Chatbox(Nz::RenderWindow* window, Ndk::Canvas* canvas) :
	m_chatEnteringBox(nullptr)
	{
		m_chatBox = canvas->Add<Ndk::TextAreaWidget>();
		m_chatBox->EnableBackground(false);
		m_chatBox->EnableLineWrap(true);
		m_chatBox->SetBackgroundColor(Nz::Color(0, 0, 0, 50));
		m_chatBox->SetCharacterSize(20);
		m_chatBox->SetTextColor(Nz::Color::White);
		m_chatBox->SetTextOutlineColor(Nz::Color::Black);
		m_chatBox->SetTextOutlineThickness(1.f);
		m_chatBox->SetReadOnly(true);

		m_chatboxScrollArea = canvas->Add<Ndk::ScrollAreaWidget>(m_chatBox);
		m_chatboxScrollArea->Resize({ 480.f, 0.f });

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

	void Chatbox::PrintMessage(const std::string& message)
	{
		std::cout << message << std::endl;

		m_chatLines.emplace_back(message);
		if (m_chatLines.size() > maxChatLines)
			m_chatLines.erase(m_chatLines.begin());

		m_chatBox->Clear();
		for (const Nz::String& message : m_chatLines)
			m_chatBox->AppendText(message + "\n");

		m_chatBox->Resize({ m_chatBox->GetWidth(), m_chatBox->GetPreferredHeight() });
		m_chatboxScrollArea->Resize(m_chatboxScrollArea->GetSize()); // force layout update
		m_chatboxScrollArea->SetPosition({ 5.f, m_chatEnteringBox->GetPosition().y - m_chatboxScrollArea->GetHeight() - 5, 0.f });

		m_chatboxScrollArea->ScrollToRatio(1.f);
	}

	void Chatbox::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		Nz::Vector2f size = Nz::Vector2f(renderTarget->GetSize());

		m_chatEnteringBox->Resize({ size.x, 40.f });
		m_chatEnteringBox->SetPosition({ 0.f, size.y - m_chatEnteringBox->GetHeight() - 5.f, 0.f });
		m_chatboxScrollArea->Resize({ size.x / 3.f, size.y / 3.f });
		m_chatboxScrollArea->SetPosition({ 5.f, m_chatEnteringBox->GetPosition().y - m_chatboxScrollArea->GetHeight() - 5, 0.f });
	}
}
