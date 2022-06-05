// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ClientLib/Scripting/ParticleRegistry.hpp>
#include <CoreLib/Scripting/AbstractScriptingLibrary.hpp>
#include <ClientLib/ClientAssetStore.hpp>
#include <Nazara/Core/OffsetOf.hpp>
#include <Nazara/Graphics/Material.hpp>
#include <Nazara/Graphics/ParticleFunctionController.hpp>
#include <Nazara/Graphics/ParticleFunctionGenerator.hpp>
#include <Nazara/Graphics/ParticleFunctionRenderer.hpp>
#include <Nazara/Graphics/ParticleGroup.hpp>
#include <Nazara/Graphics/ParticleMapper.hpp>
#include <Nazara/Graphics/AbstractRenderQueue.hpp>
#include <random>
#include <stdexcept>

namespace bw
{
	namespace
	{
		struct Billboard2D
		{
			Nz::Color color;
			Nz::Vector2f size;
			Nz::Vector2f velocity;
			Nz::Vector3f position;
			float life;
			float rotation;
		};

		std::mt19937& GetRandomGenerator()
		{
			static std::mt19937 gen;
			return gen;
		};
	}

	ParticleRegistry::ParticleRegistry(ClientAssetStore& assetStore) :
	m_clientAssetStore(assetStore)
	{
		Nz::ParticleDeclarationRef billboard2D = Nz::ParticleDeclaration::New();
		billboard2D->EnableComponent(Nz::ParticleComponent_Color,    Nz::ComponentType_Color,  NazaraOffsetOf(Billboard2D, color));
		billboard2D->EnableComponent(Nz::ParticleComponent_Life,     Nz::ComponentType_Float1, NazaraOffsetOf(Billboard2D, life));
		billboard2D->EnableComponent(Nz::ParticleComponent_Position, Nz::ComponentType_Float3, NazaraOffsetOf(Billboard2D, position));
		billboard2D->EnableComponent(Nz::ParticleComponent_Rotation, Nz::ComponentType_Float1, NazaraOffsetOf(Billboard2D, rotation));
		billboard2D->EnableComponent(Nz::ParticleComponent_Size,     Nz::ComponentType_Float2, NazaraOffsetOf(Billboard2D, size));
		billboard2D->EnableComponent(Nz::ParticleComponent_Velocity, Nz::ComponentType_Float2, NazaraOffsetOf(Billboard2D, velocity));

		RegisterLayout("billboard2d", std::move(billboard2D));
		
		RegisterController("alphaFromLife", [](const sol::table& parameters) -> Nz::ParticleControllerRef
		{
			float maxAlpha = parameters.get_or("maxAlpha", 255.f);
			float maxLife = parameters["maxLife"];

			return Nz::ParticleFunctionController::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float /*elapsedTime*/)
				{
					auto colorPtr = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);
					auto lifePtr = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

					for (unsigned int i = startId; i <= endId; ++i)
					{
						float& lifeValue = lifePtr[i];

						colorPtr[i].a = static_cast<Nz::UInt8>(Nz::Clamp(maxAlpha * lifeValue / maxLife, 0.f, 255.f));
					}
				}
			);
		});

		RegisterController("life", [](const sol::table& /*parameters*/) -> Nz::ParticleControllerRef
		{
			return Nz::ParticleFunctionController::New(
				[](Nz::ParticleGroup& group, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
				{
					auto lifePtr = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

					for (unsigned int i = startId; i <= endId; ++i)
					{
						float& lifeValue = lifePtr[i];

						lifeValue -= elapsedTime;
						if (lifeValue < 0.f)
							group.KillParticle(i);
					}
				}
			);
		});

		RegisterController("size", [](const sol::table& parameters) -> Nz::ParticleControllerRef
		{
			Nz::Vector2f growth = parameters["size"];

			return Nz::ParticleFunctionController::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
				{
					auto sizePtr = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size);

					for (unsigned int i = startId; i <= endId; ++i)
						sizePtr[i] += growth * elapsedTime;
				}
			);
		});

		RegisterController("velocity", [](const sol::table& parameters) -> Nz::ParticleControllerRef
		{
			float damping = parameters.get_or("damping", 1.f);

			if (Nz::NumberEquals(damping, 1.f))
			{
				return Nz::ParticleFunctionController::New(
					[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
					{
						auto posPtr = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Position);
						auto velPtr = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Velocity);

						for (unsigned int i = startId; i <= endId; ++i)
							posPtr[i] += Nz::Vector3f(velPtr[i] * elapsedTime);
					}
				);
			}
			else
			{
				return Nz::ParticleFunctionController::New(
					[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, float elapsedTime)
					{
						auto posPtr = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Position);
						auto velPtr = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Velocity);

						float dampingFactor = std::pow(damping, elapsedTime);

						for (unsigned int i = startId; i <= endId; ++i)
						{
							Nz::Vector2f& velocity = velPtr[i];

							velocity *= dampingFactor;
							posPtr[i] += Nz::Vector3f(velocity * elapsedTime);
						}
					}
				);
			}
		});
		
		RegisterGenerator("color", [](const sol::table& parameters) -> Nz::ParticleGeneratorRef
		{
			sol::table min = parameters["min"];
			float minR = min.get_or("r", 1.f);
			float minG = min.get_or("g", 1.f);
			float minB = min.get_or("b", 1.f);
			float minA = min.get_or("a", 1.f);

			sol::table max = parameters["max"];
			float maxR = max.get_or("r", 1.f);
			float maxG = max.get_or("g", 1.f);
			float maxB = max.get_or("b", 1.f);
			float maxA = max.get_or("a", 1.f);

			if (minR > maxR)
				std::swap(minR, maxR);

			if (minG > maxG)
				std::swap(minG, maxG);

			if (minB > maxB)
				std::swap(minB, maxB);

			if (minA > maxA)
				std::swap(minA, maxA);

			return Nz::ParticleFunctionGenerator::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId) mutable
				{
					auto colorPtr = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);

					auto& gen = GetRandomGenerator();
					std::uniform_real_distribution<float> disR(minR, maxR);
					std::uniform_real_distribution<float> disG(minG, maxG);
					std::uniform_real_distribution<float> disB(minB, maxB);
					std::uniform_real_distribution<float> disA(minA, maxA);

					auto Cast = [](float color) -> Nz::UInt8
					{
						return static_cast<Nz::UInt8>(Nz::Clamp(color * 255.f, 0.f, 255.f));
					};

					for (unsigned int i = startId; i <= endId; ++i)
					{
						float r = disR(gen);
						float g = disG(gen);
						float b = disB(gen);
						float a = disA(gen);

						colorPtr[i] = Nz::Color(Cast(r), Cast(g), Cast(b), Cast(a));
					}
				}
			);
		});

		RegisterGenerator("alpha", [](const sol::table& parameters) -> Nz::ParticleGeneratorRef
		{
			Nz::UInt8 alpha = static_cast<Nz::UInt8>(Nz::Clamp<unsigned int>(parameters.get_or("alpha", 255), 0, 255));

			return Nz::ParticleFunctionGenerator::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId) mutable
				{
					auto colorPtr = mapper.GetComponentPtr<Nz::Color>(Nz::ParticleComponent_Color);

					for (unsigned int i = startId; i <= endId; ++i)
						colorPtr[i] = Nz::Color(255, 255, 255, alpha);
				}
			);
		});

		RegisterGenerator("life", [](const sol::table& parameters) -> Nz::ParticleGeneratorRef
		{
			float max = parameters["max"];
			float min = parameters["min"];

			if (min > max)
				std::swap(min, max);

			return Nz::ParticleFunctionGenerator::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId) mutable
				{
					auto lifePtr = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Life);

					auto& gen = GetRandomGenerator();
					std::uniform_real_distribution<float> dis(min, max);

					for (unsigned int i = startId; i <= endId; ++i)
					{
						float& lifeValue = lifePtr[i];
						lifeValue = dis(gen);
					}
				}
			);
		});

		RegisterGenerator("position", [](const sol::table& parameters) -> Nz::ParticleGeneratorRef
		{
			Nz::Vector2f origin = parameters["origin"];
			float maxDist = parameters["maxDist"];
			float minDist = parameters.get_or("minDist", 0.f);

			if (minDist > maxDist)
				std::swap(minDist, maxDist);

			return Nz::ParticleFunctionGenerator::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId) mutable
				{
					auto positionPtr = mapper.GetComponentPtr<Nz::Vector3f>(Nz::ParticleComponent_Position);

					auto& gen = GetRandomGenerator();
					std::uniform_real_distribution<float> dis(minDist, maxDist);
					std::uniform_real_distribution<float> xy(-1.f, 1.f);

					for (unsigned int i = startId; i <= endId; ++i)
					{
						Nz::Vector2f offset(xy(gen), xy(gen));
						positionPtr[i] = origin + offset * dis(gen);
					}
				}
			);
		});
		
		RegisterGenerator("rotation", [](const sol::table& parameters) -> Nz::ParticleGeneratorRef
		{
			float max = parameters.get_or("max", 0.f);
			float min = parameters.get_or("min", 0.f);

			if (min > max)
				std::swap(min, max);

			return Nz::ParticleFunctionGenerator::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId) mutable
				{
					auto rotPtr = mapper.GetComponentPtr<float>(Nz::ParticleComponent_Rotation);

					auto& gen = GetRandomGenerator();
					std::uniform_real_distribution<float> dis(min, max);

					for (unsigned int i = startId; i <= endId; ++i)
					{
						float& rotationValue = rotPtr[i];
						rotationValue = dis(gen);
					}
				}
			);
		});

		RegisterGenerator("size", [](const sol::table& parameters) -> Nz::ParticleGeneratorRef
		{
			Nz::Vector2f maxSize = parameters["max"];
			Nz::Vector2f minSize = parameters["min"];

			if (minSize.x > maxSize.x)
				std::swap(minSize.x, maxSize.x);

			if (minSize.y > maxSize.y)
				std::swap(minSize.y, maxSize.y);

			return Nz::ParticleFunctionGenerator::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId) mutable
				{
					auto sizePtr = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Size);

					auto& gen = GetRandomGenerator();
					std::uniform_real_distribution<float> disX(minSize.x, maxSize.x);
					std::uniform_real_distribution<float> disY(minSize.y, maxSize.y);

					for (unsigned int i = startId; i <= endId; ++i)
						sizePtr[i] = Nz::Vector2f(disX(gen), disY(gen));
				}
			);
		});
		
		RegisterGenerator("velocity_unit_random", [](const sol::table& parameters) -> Nz::ParticleGeneratorRef
		{
			float minSpeed = parameters["minSpeed"];
			float maxSpeed = parameters["maxSpeed"];

			if (minSpeed > maxSpeed)
				std::swap(minSpeed, maxSpeed);

			return Nz::ParticleFunctionGenerator::New(
				[=](Nz::ParticleGroup& /*group*/, Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId) mutable
				{
					auto velPtr = mapper.GetComponentPtr<Nz::Vector2f>(Nz::ParticleComponent_Velocity);

					auto& gen = GetRandomGenerator();
					std::uniform_real_distribution<float> disAngle(-Nz::Pi<float>, Nz::Pi<float>);
					std::uniform_real_distribution<float> disLength(minSpeed, maxSpeed);

					for (unsigned int i = startId; i <= endId; ++i)
					{
						Nz::RadianAnglef angle(disAngle(gen));
						auto sincos = angle.GetSinCos();

						Nz::Vector2f velocity(sincos.first, sincos.second);
						velPtr[i] = velocity * disLength(gen);
					}
				}
			);
		});

		RegisterRenderer("billboard", [&](const sol::table& parameters) -> Nz::ParticleRendererRef
		{
			std::string texturePath = parameters["texturePath"];

			Nz::MaterialRef material = Nz::Material::New("Translucent2D");
			material->SetDiffuseMap(m_clientAssetStore.GetTexture(texturePath));

			return Nz::ParticleFunctionRenderer::New(
				[=](const Nz::ParticleGroup& /*group*/, const Nz::ParticleMapper& mapper, unsigned int startId, unsigned int endId, Nz::AbstractRenderQueue* renderQueue)
				{
					auto colorPtr = mapper.GetComponentPtr<const Nz::Color>(Nz::ParticleComponent_Color);
					auto positionPtr = mapper.GetComponentPtr<const Nz::Vector3f>(Nz::ParticleComponent_Position);
					auto rotationPtr = mapper.GetComponentPtr<const float>(Nz::ParticleComponent_Rotation);
					auto sizePtr = mapper.GetComponentPtr<const Nz::Vector2f>(Nz::ParticleComponent_Size);

					renderQueue->AddBillboards(0, material, endId - startId + 1, Nz::Recti(-1, -1), positionPtr, sizePtr, rotationPtr, colorPtr);
				}
			);
		});
	}

	const Nz::ParticleDeclarationRef& ParticleRegistry::GetLayout(const std::string& name) const
	{
		auto it = m_layouts.find(name);
		if (it == m_layouts.end())
		{
			static Nz::ParticleDeclarationRef dummy;
			return dummy;
		}

		return it.value();
	}

	Nz::ParticleControllerRef ParticleRegistry::InstantiateController(const std::string& name, const sol::table& parameters) const
	{
		auto it = m_controllers.find(name);
		if (it == m_controllers.end())
			throw std::runtime_error("Controller \"" + name + "\" doesn't exist");

		const ControllerFactory& factory = it.value();
		return factory(parameters);
	}

	Nz::ParticleGeneratorRef ParticleRegistry::InstantiateGenerator(const std::string& name, const sol::table& parameters) const
	{
		auto it = m_generators.find(name);
		if (it == m_generators.end())
			throw std::runtime_error("Generator \"" + name + "\" doesn't exist");

		const GeneratorFactory& factory = it.value();
		return factory(parameters);
	}

	Nz::ParticleRendererRef ParticleRegistry::InstantiateRenderer(const std::string& name, const sol::table& parameters) const
	{
		auto it = m_renderers.find(name);
		if (it == m_renderers.end())
			throw std::runtime_error("Renderer \"" + name + "\" doesn't exist");

		const RendererFactory& factory = it.value();
		return factory(parameters);
	}
}
