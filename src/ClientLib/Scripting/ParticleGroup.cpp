// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ParticleGroup.hpp>
#include <ClientLib/Scripting/ParticleRegistry.hpp>
//#include <NDK/Components/ParticleGroupComponent.hpp>
#include <cassert>
#include <stdexcept>

namespace bw
{
	ParticleGroup::ParticleGroup(const ParticleRegistry& particleRegistry, entt::handle particleGroup) :
	m_entity(particleGroup),
	m_registry(particleRegistry)
	{
		//assert(m_entity && m_entity->HasComponent<Ndk::ParticleGroupComponent>());
	}

	void ParticleGroup::AddController(const std::string& controllerName, const sol::table& parameters)
	{
		if (!m_entity)
			throw std::runtime_error("Particle group has been killed");

		/*auto controllerRef = m_registry.InstantiateController(controllerName, parameters);
		assert(controllerRef);

		auto& particleGroup = m_entity->GetComponent<Ndk::ParticleGroupComponent>();
		particleGroup.AddController(std::move(controllerRef));*/
	}

	void ParticleGroup::AddGenerator(const std::string& generatorName, const sol::table& parameters)
	{
		if (!m_entity)
			throw std::runtime_error("Particle group has been killed");

		/*auto generatorRef = m_registry.InstantiateGenerator(generatorName, parameters);
		assert(generatorRef);

		auto& particleGroup = m_entity->GetComponent<Ndk::ParticleGroupComponent>();
		particleGroup.AddGenerator(std::move(generatorRef));*/
	}

	void ParticleGroup::GenerateParticles(unsigned int count)
	{
		if (!m_entity)
			throw std::runtime_error("Particle group has been killed");

		/*auto& particleGroup = m_entity->GetComponent<Ndk::ParticleGroupComponent>();
		particleGroup.GenerateParticles(count);*/
	}

	std::size_t ParticleGroup::GetParticleCount() const
	{
		if (!m_entity)
			throw std::runtime_error("Particle group has been killed");

		/*auto& particleGroup = m_entity->GetComponent<Ndk::ParticleGroupComponent>();
		return particleGroup.GetParticleCount();*/
	}

	void ParticleGroup::Kill()
	{
		if (m_entity)
			m_entity->destroy();
	}
	
	void ParticleGroup::SetRenderer(const std::string& rendererName, const sol::table& parameters)
	{
		if (!m_entity)
			throw std::runtime_error("Particle group has been killed");

		/*auto rendererRef = m_registry.InstantiateRenderer(rendererName, parameters);
		assert(rendererRef);

		auto& particleGroup = m_entity->GetComponent<Ndk::ParticleGroupComponent>();
		particleGroup.SetRenderer(std::move(rendererRef));*/
	}
}
