// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <MapEditor/Scripting/EditorEntityLibrary.hpp>
#include <NDK/Components/PhysicsComponent2D.hpp>
#include <Thirdparty/sol3/sol.hpp>

namespace bw
{
	void EditorEntityLibrary::RegisterLibrary(sol::table& elementMetatable)
	{
		ClientEntityLibrary::RegisterLibrary(elementMetatable);

		RegisterEditorLibrary(elementMetatable);
	}

	void EditorEntityLibrary::InitRigidBody(lua_State* /*L*/, const Ndk::EntityHandle& /*entity*/, float /*mass*/)
	{
		// Do nothing
	}

	void EditorEntityLibrary::RegisterEditorLibrary(sol::table& elementMetatable)
	{
		auto Dummy = []() {};

		// FIXME
		elementMetatable["AddLayer"] = Dummy;
	}
}
