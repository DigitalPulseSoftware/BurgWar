// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/EscapeMenu.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>

namespace bw
{
	EscapeMenu::EscapeMenu(Nz::RenderTarget* rt, Ndk::Canvas* canvas)
	{
		m_backgroundWidget = canvas->Add<Ndk::BaseWidget>();
		m_backgroundWidget->EnableBackground(true);
		m_backgroundWidget->SetBackgroundColor(Nz::Color(0, 0, 0, 160));

		m_closeMenuButton = m_backgroundWidget->Add<Ndk::ButtonWidget>();
		m_closeMenuButton->UpdateText(Nz::SimpleTextDrawer::Draw("Close", 30));
		m_closeMenuButton->Resize(m_closeMenuButton->GetPreferredSize());
		m_closeMenuButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*) {
			Hide();
		});

		m_leaveMatchButton = m_backgroundWidget->Add<Ndk::ButtonWidget>();
		m_leaveMatchButton->UpdateText(Nz::SimpleTextDrawer::Draw("Leave match", 30));
		m_leaveMatchButton->Resize(m_leaveMatchButton->GetPreferredSize());
		m_leaveMatchButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*) {
			OnLeaveMatch(this);
		});

		m_quitAppButton = m_backgroundWidget->Add<Ndk::ButtonWidget>();
		m_quitAppButton->UpdateText(Nz::SimpleTextDrawer::Draw("Exit application", 30));
		m_quitAppButton->Resize(m_quitAppButton->GetPreferredSize());
		m_quitAppButton->OnButtonTrigger.Connect([this](const Ndk::ButtonWidget*) {
			OnQuitApp(this);
		});

		// Connect every slot
		m_onTargetChangeSizeSlot.Connect(rt->OnRenderTargetSizeChange, this, &EscapeMenu::OnRenderTargetSizeChange);

		OnRenderTargetSizeChange(rt);

		Hide();
	}

	EscapeMenu::~EscapeMenu()
	{
		m_backgroundWidget->Destroy();
	}

	void EscapeMenu::Show(bool shouldOpen)
	{
		if (IsVisible() != shouldOpen)
			m_backgroundWidget->Show(shouldOpen);
	}

	void EscapeMenu::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		Nz::Vector2f size = Nz::Vector2f(renderTarget->GetSize());

		constexpr float padding = 20.f;
		constexpr float buttonPadding = 10.f;

		std::array<Ndk::ButtonWidget*, 3> buttons = { m_closeMenuButton, m_leaveMatchButton, m_quitAppButton };

		float maxWidth = 0.f;
		float height = 0.f;
		for (Ndk::ButtonWidget* button : buttons)
		{
			maxWidth = std::max(maxWidth, button->GetWidth());
			height += button->GetHeight();
		}
		maxWidth += padding * 2.f;
		height += padding * 2.f + buttonPadding * (buttons.size() - 1);

		m_backgroundWidget->Resize({ maxWidth, height });
		m_backgroundWidget->Center();

		float cursor = padding;
		for (Ndk::ButtonWidget* button : buttons)
		{
			button->SetPosition(0.f, cursor);
			button->CenterHorizontal();

			cursor += button->GetHeight() + buttonPadding;
		}
	}
}
