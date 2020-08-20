// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_GAMEMODEEVENTS_HPP
#define BURGWAR_CORELIB_SCRIPTING_GAMEMODEEVENTS_HPP

#include <CoreLib/Scripting/EventCombinator.hpp>
#include <Thirdparty/sol3/sol.hpp>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

namespace bw
{
	enum class GamemodeEvent
	{
#define BURGWAR_EVENT(X) X,
#define BURGWAR_EVENT_LAST(X) X, Max = X

#include <CoreLib/Scripting/GamemodeEventList.hpp>
	};

	template<GamemodeEvent E>
	struct GamemodeEventData
	{
		using ResultType = void;

		static constexpr bool FatalError = false;
	};

	template<>
	struct GamemodeEventData<GamemodeEvent::Init>
	{
		using ResultType = void;

		static constexpr bool FatalError = true;
	};

	using GamemodePlayerChatReturn = std::pair<sol::optional<bool>, sol::optional<std::string>>;

	struct ChatCombinator
	{
		GamemodePlayerChatReturn operator()(GamemodePlayerChatReturn previousValue, GamemodePlayerChatReturn newValue)
		{
			if (previousValue.first.has_value())
				return previousValue;

			return newValue;
		}
	};

	template<>
	struct GamemodeEventData<GamemodeEvent::PlayerChat>
	{
		using ResultType = GamemodePlayerChatReturn;

		static constexpr bool FatalError = false;
		static constexpr EventCombinator<GamemodePlayerChatReturn, ChatCombinator> Combinator = {};
	};

	constexpr std::size_t GamemodeEventCount = static_cast<std::size_t>(GamemodeEvent::Max) + 1;

	constexpr bool HasReturnValue(GamemodeEvent event);
	std::optional<GamemodeEvent> RetrieveGamemodeEvent(const std::string_view& eventName);
	std::string_view ToString(GamemodeEvent event);
}

#include <CoreLib/Scripting/GamemodeEvents.inl>

#endif
