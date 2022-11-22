// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEditorScriptingLibrary.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <ClientLib/Scripting/Sprite.hpp>
#include <ClientLib/Scripting/Text.hpp>
#include <ClientLib/Scripting/Texture.hpp>
#include <ClientLib/Scripting/Tilemap.hpp>

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
		sol::table renderTable = luaState.create_named_table("render");

		RegisterAssetLibrary(context, assetTable);
		RegisterRenderLibrary(context, renderTable);
		RegisterSpriteClass(context);
		RegisterTilemapClass(context);
		RegisterTextClass(context);
		RegisterTextureClass(context);
	}

	void ClientEditorScriptingLibrary::RegisterAssetLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["GetTexture"] = [this](const std::string& texturePath) -> std::optional<Texture>
		{
			const std::shared_ptr<Nz::Texture>& texture = m_assetStore.GetTexture(texturePath);
			if (texture)
				return Texture(texture);
			else
				return {};
		};
	}

	void ClientEditorScriptingLibrary::RegisterRenderLibrary(ScriptingContext& /*context*/, sol::table& library)
	{
		library["GetFrametime"] = []()
		{
			return 1.f / 60.f; //< FIXME
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

	void ClientEditorScriptingLibrary::RegisterTextClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Text>("Text",
			"new", sol::no_constructor,

			"GetSize", &Text::GetSize,
			"GetText", &Text::GetText,

			"Hide", &Text::Hide,

			"IsValid", &Text::IsValid,
			"IsVisible", &Text::IsVisible,

			"SetColor", &Text::SetColor,
			"SetHoveringHeight", &Text::SetHoveringHeight,
			"SetOffset", &Text::SetOffset,
			"SetRotation", &Text::SetRotation,
			"SetText",  &Text::SetText,

			"Show", sol::overload(&Text::Show, [](Sprite* sprite) { return sprite->Show(); })
		);
	}

	void ClientEditorScriptingLibrary::RegisterTilemapClass(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();

		state.new_usertype<Tilemap>("Tilemap",
			"new", sol::no_constructor,

			"GetMapSize",  &Tilemap::GetMapSize,
			"GetSize",     &Tilemap::GetSize,
			"GetTileSize", &Tilemap::GetTileSize,

			"Hide", &Tilemap::Hide,

			"IsValid", &Tilemap::IsValid,
			"IsVisible", &Tilemap::IsVisible,

			"SetOffset", &Tilemap::SetOffset,
			"SetRotation", &Tilemap::SetRotation,
			"SetTileColor", &Tilemap::SetTileColor,

			"Show", sol::overload(&Tilemap::Show, [](Tilemap* tilemap) { return tilemap->Show(); })
		);
	}
}
