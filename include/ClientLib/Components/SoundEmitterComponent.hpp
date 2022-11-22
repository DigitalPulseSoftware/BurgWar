// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_SOUNDEMITTERCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_SOUNDEMITTERCOMPONENT_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <memory>
#include <tsl/hopscotch_set.h>

namespace bw
{
	class BURGWAR_CLIENTLIB_API SoundEmitterComponent
	{
		friend class SoundSystem;

		public:
			SoundEmitterComponent() = default;
			inline SoundEmitterComponent(const SoundEmitterComponent&);
			SoundEmitterComponent(SoundEmitterComponent&&) = default;
			~SoundEmitterComponent() = default;

			Nz::UInt32 PlaySound(const std::shared_ptr<Nz::SoundBuffer>& soundBuffer, const Nz::Vector3f& soundPosition, bool attachedToEntity, bool isLooping, bool isSpatialized);
			void StopSound(Nz::UInt32 soundId);

			SoundEmitterComponent& operator=(const SoundEmitterComponent&) = delete;
			SoundEmitterComponent& operator=(SoundEmitterComponent&&) = default;

		private:
			inline const Nz::Vector3f& GetLastPosition() const;
			inline tsl::hopscotch_set<Nz::UInt32>& GetSounds();

			inline void UpdateLastPosition(const Nz::Vector3f& position);

			tsl::hopscotch_set<Nz::UInt32> m_sounds;
			Nz::Vector3f m_lastPosition;
	};
}

#include <ClientLib/Components/SoundEmitterComponent.inl>

#endif
