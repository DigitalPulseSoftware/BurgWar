// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERENTITYLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERENTITYLIBRARY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/SharedEntityLibrary.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API ServerEntityLibrary : public SharedEntityLibrary
	{
		public:
			using SharedEntityLibrary::SharedEntityLibrary;
			~ServerEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterServerLibrary(sol::table& elementMetatable);

			void SetDirection(lua_State* L, const Ndk::EntityHandle& entity, const Nz::Vector2f& upVector) override;
			void SetMass(lua_State* L, const Ndk::EntityHandle& entity, float mass, bool recomputeMomentOfInertia) override;
			void SetMomentOfInertia(lua_State* L, const Ndk::EntityHandle& entity, float momentOfInertia) override;
			void SetPosition(lua_State* L, const Ndk::EntityHandle& entity, const Nz::Vector2f& position) override;
			void SetRotation(lua_State* L, const Ndk::EntityHandle& entity, const Nz::DegreeAnglef& rotation) override;
			void UpdatePlayerJumpHeight(lua_State* L, const Ndk::EntityHandle& entity, float jumpHeight, float jumpHeightBoost) override;
			void UpdatePlayerMovement(lua_State* L, const Ndk::EntityHandle& entity, float movementSpeed) override;
	};
}

#include <CoreLib/Scripting/ServerEntityLibrary.inl>

#endif
