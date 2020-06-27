// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEditorScriptingLibrary.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/Scripting/Sprite.hpp>
#include <ClientLib/Scripting/Texture.hpp>

namespace bw
{
	ClientEditorScriptingLibrary::ClientEditorScriptingLibrary(const Logger& logger, ClientAssetStore& assetStore) :
	AbstractScriptingLibrary(logger),
	m_assetStore(assetStore)
	{
	}

	void ClientEditorScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		sol::state& luaState = context.GetLuaState();
		sol::table assetTable = luaState.create_named_table("assets");

		RegisterAssetLibrary(context, assetTable);
		RegisterSpriteClass(context);
		RegisterTextureClass(context);
	}

	void ClientEditorScriptingLibrary::RegisterAssetLibrary(ScriptingContext& context, sol::table& library)
	{
		library["GetTexture"] = [this](const std::string& texturePath) -> std::optional<Texture>
		{
			const Nz::TextureRef& texture = m_assetStore.GetTexture(texturePath);
			if (texture)
				return Texture(texture);
			else
				return {};
		};
	}

	void ClientEditorScriptingLibrary::RegisterSpriteClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Sprite>("Sprite",
			"new", sol::no_constructor,

			"GetOrigin", &Sprite::GetOrigin,
			"GetSize", &Sprite::GetSize,

			"Hide", &Sprite::Hide,

			"IsValid", &Sprite::IsValid,
			"IsVisible", &Sprite::IsVisible,

			"SetColor", &Sprite::SetColor,
			"SetCornerColor", & Sprite::SetCornerColor,
			"SetCornerColors", &Sprite::SetCornerColors,
			"SetOffset", &Sprite::SetOffset,
			"SetRotation", &Sprite::SetRotation,
			"SetSize", &Sprite::SetSize,

			"Show", sol::overload(&Sprite::Show, [](Sprite* sprite) { return sprite->Show(); })
			);
	}

	void ClientEditorScriptingLibrary::RegisterTextureClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Texture>("Texture",
			"new", sol::no_constructor,

			"GetSize", &Texture::GetSize
		);
	}
}
