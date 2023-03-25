// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_CLIENTLAYERSOUND_HPP
#define BURGWAR_CLIENTLIB_CLIENTLAYERSOUND_HPP

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
	class ClientLayer;
	class ClientLayerSound;
	class SoundEntity;

	using ClientLayerSoundHandle = Nz::ObjectHandle<ClientLayerSound>;

	class BURGWAR_CLIENTLIB_API ClientLayerSound final : public Nz::HandledObject<ClientLayerSound>
	{
		friend SoundEntity;

		public:
			inline ClientLayerSound(ClientLayer& layer, const Nz::Vector2f& position);
			inline ClientLayerSound(ClientLayer& layer, const Nz::Node& parentNode);
			ClientLayerSound(const ClientLayerSound&) = delete;
			inline ClientLayerSound(ClientLayerSound&& entity) noexcept;
			~ClientLayerSound() = default;

			Nz::Time GetSoundDuration(std::size_t soundIndex) const;

			LayerIndex GetLayerIndex() const;
			inline Nz::Vector2f GetPosition() const;

			std::size_t PlaySound(const std::shared_ptr<Nz::SoundBuffer>& soundBuffer, bool isLooping, bool isSpatialized);
			void StopSound(std::size_t soundIndex);

			bool Update(Nz::Time elapsedTime);

			inline void UpdatePosition(const Nz::Vector2f& position);

			ClientLayerSound& operator=(const ClientLayerSound&) = delete;
			ClientLayerSound& operator=(ClientLayerSound&&) = delete;

		private:
			inline ClientLayerSound(ClientLayer& layer);

			void NotifyAudibleSoundMoved(SoundEntity* oldPointer, SoundEntity* newPointer);
			void RegisterAudibleSound(SoundEntity* AudibleSound);
			void UnregisterAudibleSound(SoundEntity* AudibleSound);

			struct PlayingSound
			{
				std::shared_ptr<Nz::SoundBuffer> soundBuffer;
				Nz::Time currentOffset = Nz::Time::Zero();
				Nz::Time duration = Nz::Time::Zero();
				bool isLooping;
				bool isSpatialized;
			};

			std::vector<std::optional<PlayingSound>> m_playingSounds;
			std::vector<SoundEntity*> m_soundEntities;
			ClientLayer& m_layer;
			Nz::Node m_node;
	};
}

#include <ClientLib/ClientLayerSound.inl>

#endif
