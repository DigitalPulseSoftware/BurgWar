// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerEntityLibrary.hpp>

namespace bw
{
	void ServerEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedEntityLibrary::RegisterLibrary(elementMetatable);

		RegisterServerLibrary(elementMetatable);
	}

	void ServerEntityLibrary::RegisterServerLibrary(sol::table& elementMetatable)
	{
	}
}
