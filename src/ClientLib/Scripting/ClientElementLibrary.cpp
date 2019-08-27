// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientElementLibrary.hpp>

namespace bw
{
	void ClientElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedElementLibrary::RegisterLibrary(elementMetatable);
	}

	void ClientElementLibrary::RegisterClientLibrary(sol::table& elementTable)
	{
	}
}
