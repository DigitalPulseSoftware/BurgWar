// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SHAREDENTITYLIBRARY_HPP

#include <Nazara/Math/Angle.hpp>
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
			virtual void SetDirection(lua_State* L, const Ndk::EntityHandle& entity, const Nz::Vector2f& upVector);
			virtual void SetMass(lua_State* L, const Ndk::EntityHandle& entity, float mass, bool recomputeMomentOfInertia);
			virtual void SetMomentOfInertia(lua_State* L, const Ndk::EntityHandle& entity, float momentOfInertia);
			virtual void SetPosition(lua_State* L, const Ndk::EntityHandle& entity, const Nz::Vector2f& position);
			virtual void SetRotation(lua_State* L, const Ndk::EntityHandle& entity, const Nz::DegreeAnglef& rotation);
			virtual void UpdatePlayerJumpHeight(lua_State* L, const Ndk::EntityHandle& entity, float jumpHeight, float jumpHeightBoost);
			virtual void UpdatePlayerMovement(lua_State* L, const Ndk::EntityHandle& entity, float movementSpeed);

		private:
			void RegisterSharedLibrary(sol::table& elementMetatable);
	};
}

#include <CoreLib/Scripting/SharedEntityLibrary.inl>

#endif
