// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/SharedGamemode.hpp>
#include <CoreLib/SharedMatch.hpp>
#include <CoreLib/Utils.hpp>
#include <CoreLib/LogSystem/Logger.hpp>
#include <cassert>

namespace bw
{
	template<GamemodeEvent Event, typename... Args>
	std::enable_if_t<!HasReturnValue(Event), bool> SharedGamemode::ExecuteCallback(const Args&... args)
	{
		using EventData = GamemodeEventData<Event>;

		const auto& callbacks = m_eventCallbacks[UnderlyingCast(Event)];
		if (callbacks.empty())
			return true;

		bool ret = false;

		for (const auto& callbackData : callbacks)
		{
			sol::protected_function_result callbackResult;
			if (callbackData.async)
			{
				auto co = m_context->CreateCoroutine(callbackData.callback);
				callbackResult = co(m_gamemodeTable, args...);
			}
			else
				callbackResult = callbackData.callback(m_gamemodeTable, args...);

			if (!callbackResult.valid())
			{
				sol::error err = callbackResult;
				bwLog(m_sharedMatch.GetLogger(), LogLevel::Error, "{} callback failed: {}", ToString(Event), err.what());

				if constexpr (!EventData::FatalError)
					continue;

				return false;
			}

			ret = true;
		}

		return ret;
	}

	template<GamemodeEvent Event, typename... Args>
	std::enable_if_t<HasReturnValue(Event), std::optional<typename GamemodeEventData<Event>::ResultType>> SharedGamemode::ExecuteCallback(const Args&... args)
	{
		using EventData = GamemodeEventData<Event>;
		using ResultType = typename EventData::ResultType;

		std::optional<ResultType> combinedResult;

		const auto& callbacks = m_eventCallbacks[UnderlyingCast(Event)];
		for (const auto& callbackData : callbacks)
		{
			assert(!callbackData.async);

			auto callbackResult = callbackData.callback(m_gamemodeTable, args...);
			if (!callbackResult.valid())
			{
				sol::error err = callbackResult;
				bwLog(m_sharedMatch.GetLogger(), LogLevel::Error, "{} callback failed: {}", ToString(Event), err.what());

				if constexpr (!EventData::FatalError)
					continue;

				return {};
			}

			combinedResult = EventData::Combinator(combinedResult, callbackResult.get<ResultType>());
		}

		return combinedResult;
	}

	inline sol::table& SharedGamemode::GetTable()
	{
		return m_gamemodeTable;
	}

	inline bool SharedGamemode::HasCallbacks(GamemodeEvent event) const
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		return !callbacks.empty();
	}

	inline void SharedGamemode::RegisterCallback(GamemodeEvent event, sol::protected_function callback, bool async)
	{
		auto& callbacks = m_eventCallbacks[UnderlyingCast(event)];
		auto& callbackData = callbacks.emplace_back();
		callbackData.async = async;
		callbackData.callback = std::move(callback);
	}

	inline const std::filesystem::path& SharedGamemode::GetGamemodePath() const
	{
		return m_gamemodePath;
	}

	inline sol::table& SharedGamemode::GetGamemodeTable()
	{
		return m_gamemodeTable;
	}

	inline const std::shared_ptr<ScriptingContext>& SharedGamemode::GetScriptingContext() const
	{
		return m_context;
	}
}
