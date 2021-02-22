// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYLIBRARY_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Scripting/AbstractElementLibrary.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API SharedEntityLibrary : public AbstractElementLibrary
	{
		public:
			using AbstractElementLibrary::AbstractElementLibrary;
			~SharedEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		protected:
			virtual void InitRigidBody(lua_State* L, const Ndk::EntityHandle& entity, float mass);
			virtual void SetMass(lua_State* L, const Ndk::EntityHandle& entity, float mass, bool recomputeMomentOfInertia);
			virtual void SetMomentOfInertia(lua_State* L, const Ndk::EntityHandle& entity, float momentOfInertia);
			virtual void UpdatePlayerJumpHeight(lua_State* L, const Ndk::EntityHandle& entity, float jumpHeight, float jumpHeightBoost);
			virtual void UpdatePlayerMovement(lua_State* L, const Ndk::EntityHandle& entity, float movementSpeed);

		private:
			void RegisterSharedLibrary(sol::table& elementMetatable);
	};
}

#include <CoreLib/Scripting/SharedEntityLibrary.inl>

#endif
