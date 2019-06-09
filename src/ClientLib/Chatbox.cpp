// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Chatbox.hpp>
#include <Nazara/Utility/Font.hpp>
#include <iostream>

namespace bw
{
	static constexpr std::size_t maxChatLines = 15;

	Chatbox::Chatbox(Nz::RenderWindow* window, Ndk::Canvas* canvas) :
	m_chatLines(maxChatLines),
	m_chatEnteringBox(nullptr)
	{
		m_chatBox = canvas->Add<Ndk::TextAreaWidget>();
		m_chatBox->EnableBackground(false);
		m_chatBox->SetBackgroundColor(Nz::Color(0, 0, 0, 50));
		m_chatBox->SetCharacterSize(20);
		m_chatBox->SetTextColor(Nz::Color::White);
		m_chatBox->SetTextOutlineColor(Nz::Color::Black);
		m_chatBox->SetTextOutlineThickness(1.f);
		m_chatBox->SetReadOnly(true);

		m_chatBox->Resize({ 480.f, float(maxChatLines * Nz::Font::GetDefault()->GetSizeInfo(m_chatBox->GetCharacterSize()).lineHeight) });

		m_chatEnteringBox = canvas->Add<Ndk::TextAreaWidget>();
		m_chatEnteringBox->EnableBackground(true);
		m_chatEnteringBox->SetBackgroundColor(Nz::Color(255, 255, 255, 150));
		m_chatEnteringBox->SetTextColor(Nz::Color::Black);
		m_chatEnteringBox->Show(false);

		Nz::EventHandler& eventHandler = window->GetEventHandler();

		// Connect every slot
		m_onKeyPressedSlot.Connect(eventHandler.OnKeyPressed, this, &Chatbox::OnKeyPressed);
		m_onTargetChangeSizeSlot.Connect(window->OnRenderTargetSizeChange, this, &Chatbox::OnRenderTargetSizeChange);

		OnRenderTargetSizeChange(window);
	}

	Chatbox::~Chatbox()
	{
		m_chatBox->Destroy();

		if (m_chatEnteringBox)
			m_chatEnteringBox->Destroy();
	}

	void Chatbox::Clear()
	{
		m_chatLines.clear();
		m_chatBox->Clear();
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
	}

	void Chatbox::OnKeyPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::KeyEvent& event)
	{
		if (event.code == Nz::Keyboard::Return)
		{
			if (m_chatEnteringBox->IsVisible())
			{
				Nz::String text = m_chatEnteringBox->GetText();
				m_chatBox->EnableBackground(false);
				m_chatEnteringBox->Clear();
				m_chatEnteringBox->Show(false);

				if (!text.IsEmpty())
					OnChatMessage(text.ToStdString());
			}
			else
			{
				m_chatBox->EnableBackground(true);
				m_chatEnteringBox->Show(true);
				m_chatEnteringBox->SetFocus();
			}
		}
	}

	void Chatbox::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		Nz::Vector2f size = Nz::Vector2f(renderTarget->GetSize());

		m_chatEnteringBox->Resize({ size.x, 40.f });
		m_chatEnteringBox->SetPosition({ 0.f, size.y - m_chatEnteringBox->GetHeight() - 5.f, 0.f });

		m_chatBox->SetPosition({ 5.f, size.y - m_chatEnteringBox->GetHeight() - m_chatBox->GetHeight() - 5, 0.f });
	}
}
