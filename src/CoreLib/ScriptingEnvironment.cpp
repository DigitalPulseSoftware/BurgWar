// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/ScriptingEnvironment.hpp>
#include <CoreLib/Protocol/Packets.hpp>

namespace bw
{
	ScriptingEnvironment::ScriptingEnvironment(const Logger& logger, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<Nz::VirtualDirectory>& scriptDir)
	{
		m_scriptingContext = std::make_shared<ScriptingContext>(logger, scriptDir);
		m_scriptingContext->LoadLibrary(std::move(scriptingLibrary));

		m_scriptingContext->SetPrintFunction([this](const std::string& str, const Nz::Color& color)
		{
			m_outputCallback(str, color);
		});
	}

	bool ScriptingEnvironment::Execute(const std::string& command)
	{
		if (command.empty())
			return false;

		m_scriptingContext->Update();

		try
		{
			sol::state& luaState = m_scriptingContext->GetLuaState();
			sol::load_result loadResult = luaState.load(command.data(), {}, sol::load_mode::text);
			if (loadResult.valid())
			{
				sol::protected_function fun = loadResult;
				sol::coroutine co = m_scriptingContext->CreateCoroutine(fun);

				auto result = co();
				if (!result.valid())
				{
					sol::error err = result;
					if (m_outputCallback)
						m_outputCallback(err.what(), Nz::Color::Red);

					return false;
				}
			}
			else
			{
				sol::error err = loadResult;
				if (m_outputCallback)
					m_outputCallback(err.what(), Nz::Color::Red);

				return false;
			}

			return true;
		}
		catch (const std::exception& e)
		{
			if (m_outputCallback)
				m_outputCallback("PANIC: " + std::string(e.what()), Nz::Color::Red);

			return false;
		}
	}

	void ScriptingEnvironment::SetOutputCallback(OutputCallback callback)
	{
		m_outputCallback = std::move(callback);
	}
}
