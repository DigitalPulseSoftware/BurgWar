// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedScriptingContext.hpp>
#include <CoreLib/Scripting/SharedScriptingLibrary.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <filesystem>
#include <iostream>

namespace bw
{
	SharedScriptingContext::~SharedScriptingContext()
	{
		m_availableThreads.clear();
		m_runningThreads.clear();
	}

	void SharedScriptingContext::LoadLibrary(std::shared_ptr<AbstractScriptingLibrary> library)
	{
		library->RegisterLibrary(*this);

		m_libraries.emplace_back(std::move(library)); //< Store library to ensure it won't be deleted
	}

	void SharedScriptingContext::Update()
	{
		for (auto it = m_runningThreads.begin(); it != m_runningThreads.end();)
		{
			sol::thread& runningThread = *it;
			lua_State* lthread = runningThread.thread_state();

			bool removeThread = true;
			switch (static_cast<sol::thread_status>(lua_status(lthread)))
			{
				case sol::thread_status::ok:
					// Coroutine has finished without error, we can recycle its thread
					m_availableThreads.emplace_back(std::move(runningThread));
					break;

				case sol::thread_status::yielded:
					removeThread = false;
					break;

				// Errors
				case sol::thread_status::dead:
				case sol::thread_status::handler:
				case sol::thread_status::gc:
				case sol::thread_status::memory:
				case sol::thread_status::runtime:
					break;
			}

			if (removeThread)
				it = m_runningThreads.erase(it);
			else
				++it;
		}
	}
}
