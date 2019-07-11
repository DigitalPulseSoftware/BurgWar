// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <Nazara/Math/Rect.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <cassert>

namespace bw
{
	ScriptingContext::ScriptingContext(std::shared_ptr<VirtualDirectory> scriptDir) :
	m_scriptDirectory(std::move(scriptDir))
	{
	}

	template<typename... Args>
	sol::coroutine ScriptingContext::CreateCoroutine(Args&&... args)
	{
		auto CreateThread = [&]() -> sol::thread&
		{
			return m_runningThreads.emplace_back(sol::thread::create(m_luaState));
		};

		auto PopThread = [&]() -> sol::thread&
		{
			sol::thread& thread = m_runningThreads.emplace_back(std::move(m_availableThreads.back()));
			m_availableThreads.pop_back();

			return thread;
		};

		sol::thread& thread = (!m_availableThreads.empty()) ? PopThread() : CreateThread();

		return sol::coroutine(thread.state(), std::forward<Args>(args)...);
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
	
	inline void ScriptingContext::UpdateScriptDirectory(std::shared_ptr<VirtualDirectory> scriptDir)
	{
		m_scriptDirectory = std::move(scriptDir);
	}
}
