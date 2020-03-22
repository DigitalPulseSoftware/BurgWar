// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Console.hpp>
#include <NDK/Console.hpp>

namespace bw
{
	Console::Console(Nz::RenderTarget* window, Ndk::Canvas* canvas)
	{
		m_widget = canvas->Add<Ndk::Console>();
		m_widget->Show(false);

		m_widget->OnCommand.Connect([this](Ndk::Console*, const Nz::String& command)
		{
			if (m_callback)
				m_callback(command.ToStdString());
		});

		// Connect every slot
		m_onTargetChangeSizeSlot.Connect(window->OnRenderTargetSizeChange, this, &Console::OnRenderTargetSizeChange);

		OnRenderTargetSizeChange(window);
	}

	void Console::Clear()
	{
		m_widget->Clear();
	}

	void Console::Print(const std::string& str, Nz::Color color)
	{
		m_widget->AddLine(str, color);
	}

	void Console::SetExecuteCallback(ExecuteCallback callback)
	{
		m_callback = std::move(callback);
	}

	void Console::Show(bool shouldShow)
	{
		if (IsVisible() != shouldShow)
		{
			if (shouldShow)
			{
				m_widget->Show(true);
				m_widget->SetFocus();
			}
			else
				m_widget->Show(false);
		}
	}

	void Console::OnRenderTargetSizeChange(const Nz::RenderTarget* renderTarget)
	{
		Nz::Vector2f size = Nz::Vector2f(renderTarget->GetSize());

		m_widget->Resize({ size.x, size.y / 3.f });
		m_widget->SetPosition(0.f, 0.f);
	}
}
