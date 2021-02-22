// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_WEAPONWIELDERCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_WEAPONWIELDERCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <NDK/Component.hpp>
#include <NDK/EntityOwner.hpp>
#include <Thirdparty/tsl/hopscotch_map.h>
#include <functional>
#include <limits>

namespace bw
{
	class BURGWAR_CORELIB_API WeaponWielderComponent : public Ndk::Component<WeaponWielderComponent>
	{
		public:
			using WeaponInitCallback = std::function<Ndk::EntityHandle (const std::string& /*weaponName*/)>;

			inline WeaponWielderComponent();
			inline WeaponWielderComponent(const WeaponWielderComponent& weaponWielder);
			~WeaponWielderComponent() = default;

			inline const Ndk::EntityHandle& GetActiveWeapon() const;
			inline std::size_t GetSelectedWeapon() const;
			inline const Ndk::EntityHandle& GetWeapon(std::size_t weaponIndex) const;
			inline const std::vector<Ndk::EntityOwner>& GetWeapons() const;
			inline std::size_t GetWeaponCount() const;
			inline const Nz::Vector2f& GetWeaponOffset() const;

			std::size_t GiveWeapon(std::string weaponClass, const WeaponInitCallback& callback);

			inline bool HasActiveWeapon() const;
			inline bool HasWeapon(const std::string& weaponClass) const;

			void OverrideEntities(const std::function<void(Ndk::EntityOwner& owner)>& callback);

			void RemoveWeapon(const std::string& weaponClass);

			void SelectWeapon(std::size_t weaponId);
			inline bool SelectWeapon(const std::string& weaponClass);
			inline void SetWeaponOffset(const Nz::Vector2f& weaponOffset);

			static Ndk::ComponentIndex componentIndex;

			static constexpr std::size_t NoWeapon = std::numeric_limits<std::size_t>::max();

			NazaraSignal(OnNewWeaponSelection, WeaponWielderComponent* /*wielder*/, std::size_t /*newWeaponIndex*/);
			NazaraSignal(OnWeaponAdded, WeaponWielderComponent* /*wielder*/, const std::string& /*weaponClass*/, std::size_t /*weaponIndex*/);
			NazaraSignal(OnWeaponRemove, WeaponWielderComponent* /*wielder*/, const std::string& /*weaponClass*/, std::size_t /*weaponIndex*/);

		private:
			Nz::Vector2f m_weaponOffset;
			std::size_t m_activeWeaponIndex;
			std::vector<Ndk::EntityOwner> m_weapons;
			tsl::hopscotch_map<std::string /*weaponClass*/, std::size_t /*weaponIndex*/> m_weaponByName;
	};
}

#include <CoreLib/Components/WeaponWielderComponent.inl>

#endif
