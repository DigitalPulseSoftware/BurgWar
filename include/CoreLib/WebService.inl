// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/WebService.hpp>

namespace bw
{
	const std::string& WebService::GetUserAgent()
	{
		return s_userAgent;
	}

	bool WebService::IsInitialized()
	{
		return s_isInitialized;
	}
}
