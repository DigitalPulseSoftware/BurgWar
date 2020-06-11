// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ParticleRegistry.hpp>

namespace bw
{
	inline void ParticleRegistry::RegisterController(std::string name, ControllerFactory controller)
	{
		m_controllers[std::move(name)] = std::move(controller);
	}

	inline void ParticleRegistry::RegisterGenerator(std::string name, GeneratorFactory generator)
	{
		m_generators[std::move(name)] = std::move(generator);
	}

	inline void ParticleRegistry::RegisterRenderer(std::string name, RendererFactory renderer)
	{
		m_renderers[std::move(name)] = std::move(renderer);
	}
	
	inline void ParticleRegistry::RegisterLayout(std::string name, Nz::ParticleDeclarationRef declaration)
	{
		m_layouts[std::move(name)] = std::move(declaration);
	}
}
