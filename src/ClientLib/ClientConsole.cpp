// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/ClientConsole.hpp>

namespace bw
{
	ClientConsole::ClientConsole(const Logger& logger, const Nz::RenderTarget& renderTarget, Nz::Canvas* canvas, std::shared_ptr<AbstractScriptingLibrary> scriptingLibrary, const std::shared_ptr<Nz::VirtualDirectory>& scriptDir) :
	Console(renderTarget, canvas),
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
