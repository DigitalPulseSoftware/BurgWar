// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/EscapeMenu.hpp>
#include <ClientLib/ClientEditorAppComponent.hpp>
#include <ClientLib/OptionWidget.hpp>
#include <Nazara/TextRenderer/SimpleTextDrawer.hpp>

namespace bw
{
	EscapeMenu::EscapeMenu(ClientEditorAppComponent& clientEditorApp, Nz::Canvas* canvas) :
	m_app(clientEditorApp)
	{
		m_backgroundWidget = canvas->Add<Nz::BaseWidget>();
		m_backgroundWidget->EnableBackground(true);
		m_backgroundWidget->SetBackgroundColor(Nz::Color(0, 0, 0, 160));

		m_closeMenuButton = m_backgroundWidget->Add<Nz::ButtonWidget>();
		m_closeMenuButton->UpdateText(Nz::SimpleTextDrawer::Draw("Close", 30));
		m_closeMenuButton->Resize(m_closeMenuButton->GetPreferredSize());
		m_closeMenuButton->OnButtonTrigger.Connect([this](const Nz::ButtonWidget*) {
			Hide();
		});

		m_optionsButton = m_backgroundWidget->Add<Nz::ButtonWidget>();
		m_optionsButton->UpdateText(Nz::SimpleTextDrawer::Draw("Options", 30));
		m_optionsButton->Resize(m_optionsButton->GetPreferredSize());
		m_optionsButton->OnButtonTrigger.Connect([this](const Nz::ButtonWidget*) {
			OnOptionButtonPressed();
		});

		m_leaveMatchButton = m_backgroundWidget->Add<Nz::ButtonWidget>();
		m_leaveMatchButton->UpdateText(Nz::SimpleTextDrawer::Draw("Leave match", 30));
		m_leaveMatchButton->Resize(m_leaveMatchButton->GetPreferredSize());
		m_leaveMatchButton->OnButtonTrigger.Connect([this](const Nz::ButtonWidget*) {
			OnLeaveMatch(this);
		});

		m_quitAppButton = m_backgroundWidget->Add<Nz::ButtonWidget>();
		m_quitAppButton->UpdateText(Nz::SimpleTextDrawer::Draw("Exit application", 30));
		m_quitAppButton->Resize(m_quitAppButton->GetPreferredSize());
		m_quitAppButton->OnButtonTrigger.Connect([this](const Nz::ButtonWidget*) {
			OnQuitApp(this);
		});

		m_optionWidget = m_backgroundWidget->Add<OptionWidget>(m_app.GetPlayerSettings());
		m_optionWidget->Resize(m_optionWidget->GetPreferredSize());
		m_optionWidget->SetPosition(10.f, 10.f);
		m_optionWidget->Hide();
		
		m_optionWidget->OnBackButtonTriggered.Connect([this](OptionWidget*) {
			OnBackButtonPressed();
		});

		Layout();

		Hide();
	}

	EscapeMenu::~EscapeMenu()
	{
		m_backgroundWidget->Destroy();
	}

	void EscapeMenu::Show(bool shouldOpen)
	{
		if (IsVisible() != shouldOpen)
		{
			m_backgroundWidget->Show(shouldOpen);

			m_optionWidget->Hide();

			Layout();
		}
	}

	void EscapeMenu::OnBackButtonPressed()
	{
		m_app.SavePlayerConfig();

		m_optionWidget->Hide();

		m_closeMenuButton->Show();
		m_leaveMatchButton->Show();
		m_optionsButton->Show();
		m_quitAppButton->Show();

		Layout();
	}

	void EscapeMenu::OnOptionButtonPressed()
	{
		m_optionWidget->Show();

		m_closeMenuButton->Hide();
		m_leaveMatchButton->Hide();
		m_optionsButton->Hide();
		m_quitAppButton->Hide();

		Layout();
	}

	void EscapeMenu::Layout()
	{
		if (!m_optionWidget->IsVisible())
		{
			constexpr float padding = 20.f;
			constexpr float buttonPadding = 10.f;

			std::array<Nz::ButtonWidget*, 4> buttons = { m_closeMenuButton, m_optionsButton, m_leaveMatchButton, m_quitAppButton };

			float maxWidth = 0.f;
			float height = 0.f;
			for (Nz::ButtonWidget* button : buttons)
			{
				maxWidth = std::max(maxWidth, button->GetWidth());
				height += button->GetHeight();
			}
			maxWidth += padding * 2.f;
			height += padding * 2.f + buttonPadding * (buttons.size() - 1);

			m_backgroundWidget->Resize({ maxWidth, height });

			float cursor = height - padding;
			for (Nz::ButtonWidget* button : buttons)
			{
				button->SetPosition(0.f, cursor);
				button->CenterHorizontal();

				cursor -= button->GetHeight() + buttonPadding;
			}
		}
		else
		{
			Nz::Vector2f backgroundSize = m_optionWidget->GetSize();
			backgroundSize += Nz::Vector2f(20.f);

			m_backgroundWidget->Resize(backgroundSize);
		}

		m_backgroundWidget->Center();
	}
}
