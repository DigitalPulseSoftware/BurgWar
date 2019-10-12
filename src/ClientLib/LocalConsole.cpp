// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/LocalConsole.hpp>
#include <sstream>

namespace bw
{
	LocalConsole::LocalConsole(const Logger& logger, Nz::RenderWindow* window, Ndk::Canvas* canvas, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<VirtualDirectory>& scriptDir) :
	Console(window, canvas),
	m_environment(logger, std::move(scriptingLibrary), scriptDir)
	{
		m_environment.SetOutputCallback([this](const std::string& text, Nz::Color color)
		{
			Print(text, color);
		});

		SetExecuteCallback([this](const std::string& command)
		{
			if (command.empty())
				return false;

			return m_environment.Execute(command);
		});
	}
}
