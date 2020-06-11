// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SCRIPTING_PARTICLEREGISTRY_HPP
#define BURGWAR_CLIENTLIB_SCRIPTING_PARTICLEREGISTRY_HPP

#include <Nazara/Graphics/ParticleController.hpp>
#include <Nazara/Graphics/ParticleDeclaration.hpp>
#include <Nazara/Graphics/ParticleGenerator.hpp>
#include <Nazara/Graphics/ParticleRenderer.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <Thirdparty/sol3/sol.hpp>
#include <string>

namespace bw
{
	class ClientAssetStore;

	class ParticleRegistry
	{
		public:
			using ControllerFactory = std::function<Nz::ParticleControllerRef(const sol::table & parameters)>;
			using GeneratorFactory = std::function<Nz::ParticleGeneratorRef(const sol::table & parameters)>;
			using RendererFactory = std::function<Nz::ParticleRendererRef(const sol::table& parameters)>;

			ParticleRegistry(ClientAssetStore& assetStore);
			~ParticleRegistry() = default;

			const Nz::ParticleDeclarationRef& GetLayout(const std::string& name) const;

			Nz::ParticleControllerRef InstantiateController(const std::string& name, const sol::table& parameters) const;
			Nz::ParticleGeneratorRef InstantiateGenerator(const std::string& name, const sol::table& parameters) const;
			Nz::ParticleRendererRef InstantiateRenderer(const std::string& name, const sol::table& parameters) const;

			inline void RegisterController(std::string name, ControllerFactory controller);
			inline void RegisterGenerator(std::string name, GeneratorFactory generator);
			inline void RegisterRenderer(std::string name, RendererFactory renderer);
			inline void RegisterLayout(std::string name, Nz::ParticleDeclarationRef declaration);

			ParticleRegistry& operator=(const ParticleRegistry&) = delete;
			ParticleRegistry& operator=(ParticleRegistry&& entity) = delete;

		private:
			tsl::hopscotch_map<std::string, ControllerFactory> m_controllers;
			tsl::hopscotch_map<std::string, GeneratorFactory> m_generators;
			tsl::hopscotch_map<std::string, RendererFactory> m_renderers;
			tsl::hopscotch_map<std::string, Nz::ParticleDeclarationRef> m_layouts;
			ClientAssetStore& m_clientAssetStore;
	};
}

#include <ClientLib/Scripting/ParticleRegistry.inl>

#endif
