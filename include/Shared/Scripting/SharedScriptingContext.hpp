// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Burgwar Shared" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_SHARED_SCRIPTINGCONTEXT_HPP
#define BURGWAR_SHARED_SCRIPTINGCONTEXT_HPP

#include <Nazara/Lua/LuaInstance.hpp>
#include <filesystem>

namespace bw
{
	class SharedScriptingContext
	{
		public:
			SharedScriptingContext();
			virtual ~SharedScriptingContext();

			inline Nz::LuaInstance& GetLuaInstance();
			inline const Nz::LuaInstance& GetLuaInstance() const;

			bool Load(const std::filesystem::path& folderOrFile);

		private:
			void RegisterMetatableLibrary();

			Nz::LuaInstance m_luaInstance;
	};
}

#include <Shared/Scripting/SharedScriptingContext.inl>

#endif
