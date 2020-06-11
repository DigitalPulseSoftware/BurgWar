// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_PARTICLEGROUP_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_PARTICLEGROUP_HPP

#include <NDK/EntityOwner.hpp>
#include <Thirdparty/sol3/forward.hpp>

namespace bw
{
	class ParticleRegistry;

	class ParticleGroup
	{
		public:
			ParticleGroup(const ParticleRegistry& particleRegistry, const Ndk::EntityHandle& particleGroup);
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
			Ndk::EntityOwner m_entity;
			const ParticleRegistry& m_registry;
	};
}

#include <ClientLib/Scripting/ParticleGroup.inl>

#endif
