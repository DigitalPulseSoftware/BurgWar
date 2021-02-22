// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_LOCALLAYERSOUND_HPP
#define BURGWAR_CLIENTLIB_LOCALLAYERSOUND_HPP

#include <CoreLib/LayerIndex.hpp>
#include <CoreLib/PlayerInputData.hpp>
#include <ClientLib/Export.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Core/HandledObject.hpp>
#include <Nazara/Core/ObjectHandle.hpp>
#include <Nazara/Utility/Node.hpp>
#include <optional>
#include <vector>

namespace bw
{
	class LocalLayer;
	class LocalLayerSound;
	class SoundEntity;

	using LocalLayerSoundHandle = Nz::ObjectHandle<LocalLayerSound>;

	class BURGWAR_CLIENTLIB_API LocalLayerSound final : public Nz::HandledObject<LocalLayerSound>
	{
		friend SoundEntity;

		public:
			inline LocalLayerSound(LocalLayer& layer, const Nz::Vector2f& position);
			inline LocalLayerSound(LocalLayer& layer, const Nz::Node& parentNode);
			LocalLayerSound(const LocalLayerSound&) = delete;
			inline LocalLayerSound(LocalLayerSound&& entity) noexcept;
			~LocalLayerSound() = default;

			LayerIndex GetLayerIndex() const;
			inline Nz::Vector2f GetPosition() const;

			std::size_t PlaySound(const Nz::SoundBufferRef& soundBuffer, bool isLooping, bool isSpatialized);
			void StopSound(std::size_t soundIndex);

			bool Update(float elapsedTime);

			inline void UpdatePosition(const Nz::Vector2f& position);

			LocalLayerSound& operator=(const LocalLayerSound&) = delete;
			LocalLayerSound& operator=(LocalLayerSound&&) = delete;

		private:
			inline LocalLayerSound(LocalLayer& layer);

			void NotifyAudibleSoundMoved(SoundEntity* oldPointer, SoundEntity* newPointer);
			void RegisterAudibleSound(SoundEntity* AudibleSound);
			void UnregisterAudibleSound(SoundEntity* AudibleSound);

			struct PlayingSound
			{
				Nz::SoundBufferRef soundBuffer;
				float currentOffset = 0.f;
				float duration = 0.f;
				bool isLooping;
				bool isSpatialized;
			};

			std::vector<std::optional<PlayingSound>> m_playingSounds;
			std::vector<SoundEntity*> m_soundEntities;
			LocalLayer& m_layer;
			Nz::Node m_node;
	};
}

#include <ClientLib/LocalLayerSound.inl>

#endif
