// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Console.hpp>
#include <NDK/Console.hpp>
#include <sstream>

namespace bw
{
	Console::Console(Nz::RenderWindow* window, Ndk::Canvas* canvas, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<VirtualDirectory>& scriptDir)
	{
		m_widget = canvas->Add<Ndk::Console>();
		m_widget->Show(false);

		m_scriptingContext = std::make_shared<ScriptingContext>(scriptDir);
		m_scriptingContext->LoadLibrary(std::move(scriptingLibrary));
		
		sol::state& luaState = m_scriptingContext->GetLuaState();
		luaState["print"] = [this](sol::this_state L, sol::variadic_args args)
		{
			bool first = true;

			std::ostringstream oss;
			for (auto v : args)
			{
				std::size_t length;
				const char* str = luaL_tolstring(L, v.stack_index(), &length);
				oss << std::string(str, length);
				if (!first)
					oss << "\t";

				first = false;
			}

			m_widget->AddLine(oss.str());
		};

		m_widget->OnCommand.Connect([this](Ndk::Console*, const Nz::String& command)
		{
			if (command.IsEmpty())
				return;

			try
			{
				m_scriptingContext->Update();

				sol::state& luaState = m_scriptingContext->GetLuaState();
				sol::load_result loadResult = luaState.load(command.GetConstBuffer(), {}, sol::load_mode::text);
				if (loadResult.valid())
				{
					sol::protected_function fun = loadResult;
					sol::coroutine co = m_scriptingContext->CreateCoroutine(fun);

					auto result = co();
					if (!result.valid())
					{
						sol::error err = result;
						m_widget->AddLine(err.what(), Nz::Color::Red);
					}
				}
				else
				{
					sol::error err = loadResult;
					m_widget->AddLine(err.what(), Nz::Color::Red);
				}
			}
			catch (const std::exception& e)
			{
				m_widget->AddLine("PANIC: " + std::string(e.what()), Nz::Color::Red);
			}
		});

		// Connect every slot
		m_onTargetChangeSizeSlot.Connect(window->OnRenderTargetSizeChange, this, &Console::OnRenderTargetSizeChange);

		OnRenderTargetSizeChange(window);
	}

	void Console::Clear()
	{
		m_widget->Clear();
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
