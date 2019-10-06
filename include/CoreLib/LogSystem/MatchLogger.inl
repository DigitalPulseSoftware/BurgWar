// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/LogSystem/MatchLogger.hpp>
#include <CoreLib/Match.hpp>

namespace bw
{
	template<typename T, typename Context>
	bool MatchLogger<T, Context>::ShouldLog(const Context& context) const
	{
		if (!Logger::ShouldLog(context))
			return false;

		return true;
	}

	template<typename T, typename Context>
	void MatchLogger<T, Context>::OverrideContent(const Context& context, std::string& content)
	{
		content = "[Match " + context.match->GetName() + "] " + content;
	}
}
