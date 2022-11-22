// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_PARTICLEGROUP_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_PARTICLEGROUP_HPP

#include <ClientLib/Export.hpp>
#include <sol/forward.hpp>
#include <CoreLib/EntityOwner.hpp>

namespace bw
{
	class ParticleRegistry;

	class BURGWAR_CLIENTLIB_API ParticleGroup
	{
		public:
			ParticleGroup(const ParticleRegistry& particleRegistry, entt::entity particleGroup);
			ParticleGroup(const ParticleGroup&) = delete;
			ParticleGroup(ParticleGroup&&) noexcept = default;
			~ParticleGroup() = default;

			void AddController(const std::string& controllerName, const sol::table& parameters);
			void AddGenerator(const std::string& generatorName, const sol::table& parameters);

			void GenerateParticles(unsigned int count);

			std::size_t GetParticleCount() const;

			void Kill();

			void SetRenderer(const std::string& rendererName, const sol::table& parameters);

			ParticleGroup& operator=(const ParticleGroup&) = delete;
			ParticleGroup& operator=(ParticleGroup&&) noexcept = default;

		private:
			EntityOwner m_entity;
			const ParticleRegistry& m_registry;
	};
}

#include <ClientLib/Scripting/ParticleGroup.inl>

#endif
