// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CLIENTLIB_COMPONENTS_SOUNDEMITTERCOMPONENT_HPP
#define BURGWAR_CLIENTLIB_COMPONENTS_SOUNDEMITTERCOMPONENT_HPP

#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <NDK/Component.hpp>
#include <memory>
#include <tsl/hopscotch_set.h>

namespace bw
{
	class SoundEmitterComponent : public Ndk::Component<SoundEmitterComponent>
	{
		friend class SoundSystem;

		public:
			SoundEmitterComponent() = default;
			~SoundEmitterComponent() = default;

			Nz::UInt32 PlaySound(const Nz::SoundBufferRef& soundBuffer, const Nz::Vector3f& soundPosition, bool attachedToEntity, bool isLooping, bool isSpatialized);
			void StopSound(Nz::UInt32 soundId);

			static Ndk::ComponentIndex componentIndex;

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
