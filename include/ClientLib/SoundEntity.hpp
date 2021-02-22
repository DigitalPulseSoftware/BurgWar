// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_SOUNDENTITY_HPP
#define BURGWAR_CLIENTLIB_SOUNDENTITY_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Audio/Sound.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Utility/Node.hpp>
#include <NDK/EntityOwner.hpp>

namespace bw
{
	class LocalLayerSound;
	using LocalLayerSoundHandle = Nz::ObjectHandle<LocalLayerSound>;

	class BURGWAR_CLIENTLIB_API SoundEntity
	{
		friend LocalLayerSound;

		public:
			SoundEntity(Ndk::World& renderWorld, LocalLayerSoundHandle layerEntityHandle, float depth = 0.f);
			SoundEntity(Ndk::World& renderWorld, LocalLayerSoundHandle layerEntityHandle, const Nz::Node& parentNode, float depth = 0.f);
			SoundEntity(const SoundEntity&) = delete;
			SoundEntity(SoundEntity&& entity) noexcept;
			~SoundEntity();

			inline const Ndk::EntityHandle& GetEntity() const;

			void PlaySound(std::size_t soundIndex, const Nz::SoundBufferRef& soundBuffer, bool isLooping, bool isSpatialized);

			void StopSound(std::size_t soundIndex);

			void Update(const Nz::Vector2f& position);

			SoundEntity& operator=(const SoundEntity&) = delete;
			SoundEntity& operator=(SoundEntity&& entity) = delete;

		private:
			std::vector<Nz::UInt32> m_soundIds;
			Ndk::EntityOwner m_entity;
			LocalLayerSoundHandle m_layerSound;
			float m_depth;
	};
}

#include <ClientLib/SoundEntity.inl>

#endif
