// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorElementLibrary.hpp>
#include <sol/sol.hpp>

namespace bw
{
	void EditorElementLibrary::RegisterClientLibrary(sol::table& elementMetatable)
	{
		ClientElementLibrary::RegisterClientLibrary(elementMetatable);

		auto Dummy = []() {};

		elementMetatable["PlaySound"] = Dummy;
	}
}
