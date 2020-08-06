// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_EVENTCOMBINATOR_HPP
#define BURGWAR_CORELIB_SCRIPTING_EVENTCOMBINATOR_HPP

#include <optional>

namespace bw
{
	template<typename T, typename C>
	class EventCombinator
	{
		public:
			template<typename U> T operator()(std::optional<T>& originalValue, U&& value) const;
	};
}

#include <CoreLib/Scripting/EventCombinator.inl>

#endif
