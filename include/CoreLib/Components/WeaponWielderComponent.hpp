// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_COMPONENTS_WEAPONWIELDERCOMPONENT_HPP
#define BURGWAR_CORELIB_COMPONENTS_WEAPONWIELDERCOMPONENT_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/EntityOwner.hpp>
#include <CoreLib/Components/BaseComponent.hpp>
#include <NazaraUtils/Signal.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <entt/entt.hpp>
#include <tsl/hopscotch_map.h>
#include <tl/function_ref.hpp>
#include <functional>
#include <limits>
 
namespace bw
{
	class BURGWAR_CORELIB_API WeaponWielderComponent : public BaseComponent
	{
		public:
			using WeaponInitCallback = tl::function_ref<entt::handle (const std::string& /*weaponName*/)>;

			inline WeaponWielderComponent(entt::handle entity);
			inline WeaponWielderComponent(const WeaponWielderComponent& weaponWielder);
			WeaponWielderComponent(WeaponWielderComponent&&) noexcept = default;
			~WeaponWielderComponent() = default;

			inline entt::handle GetActiveWeapon() const;
			inline std::size_t GetSelectedWeapon() const;
			inline entt::handle GetWeapon(std::size_t weaponIndex) const;
			inline const std::vector<EntityOwner>& GetWeapons() const;
			inline std::size_t GetWeaponCount() const;
			inline const Nz::Vector2f& GetWeaponOffset() const;

			std::size_t GiveWeapon(std::string weaponClass, const WeaponInitCallback& callback);

			inline bool HasActiveWeapon() const;
			inline bool HasWeapon(const std::string& weaponClass) const;

			void OverrideEntities(const std::function<void(EntityOwner& owner)>& callback);

			void RemoveWeapon(const std::string& weaponClass);

			void SelectWeapon(std::size_t weaponId);
			inline bool SelectWeapon(const std::string& weaponClass);
			inline void SetWeaponOffset(const Nz::Vector2f& weaponOffset);

			WeaponWielderComponent& operator=(const WeaponWielderComponent&) = delete;
			WeaponWielderComponent& operator=(WeaponWielderComponent&&) noexcept = default;

			static constexpr std::size_t NoWeapon = std::numeric_limits<std::size_t>::max();

			NazaraSignal(OnNewWeaponSelection, WeaponWielderComponent* /*wielder*/, std::size_t /*newWeaponIndex*/);
			NazaraSignal(OnWeaponAdded, WeaponWielderComponent* /*wielder*/, const std::string& /*weaponClass*/, std::size_t /*weaponIndex*/);
			NazaraSignal(OnWeaponRemove, WeaponWielderComponent* /*wielder*/, const std::string& /*weaponClass*/, std::size_t /*weaponIndex*/);

		private:
			std::size_t m_activeWeaponIndex;
			std::vector<EntityOwner> m_weapons;
			tsl::hopscotch_map<std::string /*weaponClass*/, std::size_t /*weaponIndex*/> m_weaponByName;
			Nz::Vector2f m_weaponOffset;
	};
}

#include <CoreLib/Components/WeaponWielderComponent.inl>

#endif
