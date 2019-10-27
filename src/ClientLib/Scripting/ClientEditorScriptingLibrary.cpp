// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ClientEditorScriptingLibrary.hpp>
#include <CoreLib/AssetStore.hpp>
#include <CoreLib/Scripting/ScriptingContext.hpp>
#include <ClientLib/Scripting/Texture.hpp>

namespace bw
{
	ClientEditorScriptingLibrary::ClientEditorScriptingLibrary(const Logger& logger, AssetStore& assetStore) :
	AbstractScriptingLibrary(logger),
	m_assetStore(assetStore)
	{
	}

	void ClientEditorScriptingLibrary::RegisterLibrary(ScriptingContext& context)
	{
		RegisterAssets(context);
		RegisterTextureClass(context);
	}

	void ClientEditorScriptingLibrary::RegisterAssets(ScriptingContext& context)
	{
		sol::state& state = context.GetLuaState();
		sol::table script = state.create_table();

		script["GetTexture"] = [this](const std::string& texturePath) -> std::optional<Texture>
		{
			const Nz::TextureRef& texture = m_assetStore.GetTexture(texturePath);
			if (texture)
				return Texture(texture);
			else
				return {};
		};

		state["assets"] = script;
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
