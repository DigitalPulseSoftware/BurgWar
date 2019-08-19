// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/ServerElementLibrary.hpp>
#include <CoreLib/Components/OwnerComponent.hpp>
#include <CoreLib/Player.hpp>
#include <iostream>

namespace bw
{
	void ServerElementLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		SharedElementLibrary::RegisterLibrary(elementMetatable);
	}

	void ServerElementLibrary::RegisterServerLibrary(sol::table& elementTable)
	{
		elementTable["GetOwner"] = [](sol::this_state s, const sol::table& table) -> sol::object
		{
			const Ndk::EntityHandle& entity = AssertScriptEntity(table);

			if (!entity->HasComponent<OwnerComponent>())
				return sol::nil;

			return sol::make_object(s, entity->GetComponent<OwnerComponent>().GetOwner()->CreateHandle());
		};
	}
}
