// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/EventCombinator.hpp>

namespace bw
{
	template<typename T, typename C>
	template<typename U>
	T EventCombinator<T, C>::operator()(std::optional<T>& originalValue, U&& value) const
	{
		if (originalValue)
		{
			C c;

			return c(std::move(*originalValue), std::forward<U>(value));
		}
		else
			return std::forward<U>(value);
	}
}
