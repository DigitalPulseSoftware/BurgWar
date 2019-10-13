// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientScriptingLibrary.hpp>
#include <ClientLib/DummyInputController.hpp>
#include <ClientLib/LocalMatch.hpp>
#include <ClientLib/Scripting/Sprite.hpp>

namespace bw
{
	ClientScriptingLibrary::ClientScriptingLibrary(LocalMatch& match) :
	SharedScriptingLibrary(match)
	{
	}

	void ClientScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		SharedScriptingLibrary::RegisterLibrary(context);

		sol::state& state = context.GetLuaState();
		state["CLIENT"] = true;
		state["SERVER"] = false;

		state["RegisterClientAssets"] = []() {}; // Dummy function
		state["RegisterClientScript"] = []() {}; // Dummy function

		RegisterDummyInputController(context);
		RegisterScriptLibrary(context);
		RegisterSpriteClass(context);

		context.Load("autorun");
	}

	void ClientScriptingLibrary::RegisterDummyInputController(ScriptingContext& context)
	{
#define BW_INPUT_PROPERTY(name, type) #name, sol::property( \
		[](DummyInputController& input) { return input.GetInputs(). name ; }, \
		[](DummyInputController& input, const type& newValue) { input.GetInputs(). name = newValue; })

		sol::state& state = context.GetLuaState();
		state.new_usertype<InputController>("InputController");

		state.new_usertype<DummyInputController>("DummyInputController",
			sol::base_classes, sol::bases<InputController>(),
			"new", sol::factories(&std::make_shared<DummyInputController>),

			BW_INPUT_PROPERTY(aimDirection, Nz::Vector2f),
			BW_INPUT_PROPERTY(isAttacking, bool),
			BW_INPUT_PROPERTY(isJumping, bool),
			BW_INPUT_PROPERTY(isMovingLeft, bool),
			BW_INPUT_PROPERTY(isMovingRight, bool)
		);

#undef BW_INPUT_PROPERTY
	}

	void ClientScriptingLibrary::RegisterScriptLibrary(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		sol::table script = state.create_table();

		script["ReloadAll"] = [this]()
		{
			throw std::runtime_error("Only the server can reload scripts");
		};

		state["scripts"] = script;
	}

	void ClientScriptingLibrary::RegisterSpriteClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Sprite>("Sprite",
			"new", sol::no_constructor,

			"GetOrigin", &Sprite::GetOrigin,
			"GetSize", &Sprite::GetSize,

			"Hide", &Sprite::Hide,
			"Show", sol::overload(&Sprite::Show, [](Sprite* sprite) { return sprite->Show(); })
		);
	}

	LocalMatch& ClientScriptingLibrary::GetMatch()
	{
		return static_cast<LocalMatch&>(GetSharedMatch());
	}
}
