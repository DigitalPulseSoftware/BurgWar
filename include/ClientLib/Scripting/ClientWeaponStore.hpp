// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CLIENTWEAPONSTORE_HPP
#define BURGWAR_CORELIB_SCRIPTING_CLIENTWEAPONSTORE_HPP

#include <CoreLib/Scripting/SharedWeaponStore.hpp>
#include <NDK/World.hpp>

namespace bw
{
	class ClientWeaponStore : public SharedWeaponStore
	{
		public:
			inline ClientWeaponStore(std::string resourceFolder, std::shared_ptr<ScriptingContext> context);
			~ClientWeaponStore() = default;

			const Ndk::EntityHandle& InstantiateWeapon(Ndk::World& world, std::size_t entityIndex, const EntityProperties& properties, const Ndk::EntityHandle& parent);

		private:
			void InitializeElementTable(sol::table& elementTable) override;
			void InitializeElement(sol::table& elementTable, ScriptedWeapon& weapon) override;

			std::string m_resourceFolder;
	};
}

#include <ClientLib/Scripting/ClientWeaponStore.inl>

#endif
