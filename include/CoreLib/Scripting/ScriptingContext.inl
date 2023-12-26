// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <NazaraUtils/CallOnExit.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <cassert>

namespace bw
{
	template<typename... Args>
	sol::coroutine ScriptingContext::CreateCoroutine(Args&&... args)
	{
		sol::thread& thread = CreateThread();

		return sol::coroutine(thread.state(), std::forward<Args>(args)...);
	}

	template<typename... Args>
	std::optional<sol::object> ScriptingContext::Exec(FileLoadCoroutine& coroutineData, Args&&... args)
	{
		Nz::CallOnExit resetOnExit([this, currentFile = std::move(m_currentFile), currentFolder = std::move(m_currentFolder)]() mutable
		{
			m_currentFile = std::move(currentFile);
			m_currentFolder = std::move(currentFolder);
		});

		m_currentFile = coroutineData.filePath;
		m_currentFolder = m_currentFile.parent_path();

		sol::protected_function_result result = coroutineData.coroutine(std::forward<Args>(args)...);
		if (!result.valid())
		{
			sol::error err = result;
			bwLog(m_logger, LogLevel::Error, "failed to load {0}: {1}", m_currentFile, err.what());
			return {};
		}

		return result;
	}

	inline const std::filesystem::path& ScriptingContext::GetCurrentFile() const
	{
		return m_currentFile;
	}

	inline const std::filesystem::path& ScriptingContext::GetCurrentFolder() const
	{
		return m_currentFolder;
	}

	inline sol::state& ScriptingContext::GetLuaState()
	{
		return m_luaState;
	}

	inline const sol::state& ScriptingContext::GetLuaState() const
	{
		return m_luaState;
	}

	inline const std::shared_ptr<Nz::VirtualDirectory>& ScriptingContext::GetScriptDirectory() const
	{
		return m_scriptDirectory;
	}

	inline void ScriptingContext::Print(const std::string& str, const Nz::Color& color)
	{
		m_printFunction(str, color);
	}

	inline void ScriptingContext::SetPrintFunction(PrintFunction function)
	{
		m_printFunction = std::move(function);
	}

	inline void ScriptingContext::UpdateScriptDirectory(std::shared_ptr<Nz::VirtualDirectory> scriptDir)
	{
		m_scriptDirectory = std::move(scriptDir);
	}
}
