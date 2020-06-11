// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_SERVERENTITYLIBRARY_HPP
#define BURGWAR_CORELIB_SCRIPTING_SERVERENTITYLIBRARY_HPP

#include <CoreLib/Scripting/SharedEntityLibrary.hpp>

namespace bw
{
	class ServerEntityLibrary : public SharedEntityLibrary
	{
		public:
			using SharedEntityLibrary::SharedEntityLibrary;
			~ServerEntityLibrary() = default;

			void RegisterLibrary(sol::table& elementMetatable) override;

		private:
			void RegisterServerLibrary(sol::table& elementMetatable);

			void SetMass(const Ndk::EntityHandle& entity, float mass, bool recomputeMomentOfInertia) override;
			void SetMomentOfInertia(const Ndk::EntityHandle& entity, float momentOfInertia) override;
	};
}

#include <CoreLib/Scripting/ServerEntityLibrary.inl>

#endif
