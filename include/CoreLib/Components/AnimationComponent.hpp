// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_ANIMATIONCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_ANIMATIONCOMPONENT_HPP

#include <Nazara/Core/Signal.hpp>
#include <NDK/Component.hpp>
#include <memory>
#include <optional>

namespace bw
{
	class AnimationStore;

	class AnimationComponent : public Ndk::Component<AnimationComponent>
	{
		friend class AnimationSystem;

		public:
			inline AnimationComponent(std::shared_ptr<AnimationStore> animStore);
			inline AnimationComponent(const AnimationComponent& animation);
			~AnimationComponent() = default;

			inline std::size_t GetAnimId() const;
			inline const std::shared_ptr<AnimationStore>& GetAnimationStore() const;
			inline Nz::UInt64 GetEndTime() const;
			inline Nz::UInt64 GetStartTime() const;
			inline bool IsPlaying() const;

			void Play(std::size_t animId, Nz::UInt64 animStartTime);

			static Ndk::ComponentIndex componentIndex;

			NazaraSignal(OnAnimationEnd, AnimationComponent* /*emitter*/);
			NazaraSignal(OnAnimationStart, AnimationComponent* /*emitter*/);

		private:
			void Update(Nz::UInt64 now);

			struct ActiveAnimation
			{
				std::size_t animId;
				Nz::UInt64 endTime;
				Nz::UInt64 startTime;
			};

			std::optional<ActiveAnimation> m_playingAnimation;
			std::shared_ptr<AnimationStore> m_animationStore;
	};
}

#include <CoreLib/Components/AnimationComponent.inl>

#endif
