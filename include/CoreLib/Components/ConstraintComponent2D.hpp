// Copyright (C) 2019 Jérôme Leclercq
// This file is part of the "Nazara Development Kit"
// For conditions of distribution and use, see copyright notice in Prerequisites.hpp

#pragma once

#ifndef BURGWAR_CORELIB_CONSTRAINTCOMPONENT2D_HPP
#define BURGWAR_CORELIB_CONSTRAINTCOMPONENT2D_HPP

#include <CoreLib/Export.hpp>
#include <CoreLib/Components/DestructionWatcherComponent.hpp>
#include <Nazara/Physics2D/Constraint2D.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <vector>

namespace bw
{
	class BURGWAR_CORELIB_API ConstraintComponent2D
	{
		public:
			ConstraintComponent2D() = default;
			ConstraintComponent2D(const ConstraintComponent2D&) = delete;
			ConstraintComponent2D(ConstraintComponent2D&& joint) noexcept = default;

			template<typename T, typename... Args> T* CreateConstraint(entt::registry& registry, entt::entity first, entt::entity second, Args&&... args);
			bool RemoveConstraint(Nz::Constraint2D* constraint);

			ConstraintComponent2D& operator=(const ConstraintComponent2D& joint) = delete;
			ConstraintComponent2D& operator=(ConstraintComponent2D&& joint) noexcept = default;


		private:
			struct ConstraintData
			{
				std::unique_ptr<Nz::Constraint2D> constraint;

				NazaraSlot(DestructionWatcherComponent, OnDestruction, onBodyADestruction);
				NazaraSlot(DestructionWatcherComponent, OnDestruction, onBodyBDestruction);
			};

			std::vector<ConstraintData> m_constraints;
	};
}

#include <CoreLib/Components/ConstraintComponent2D.inl>

#endif // NDK_COMPONENTS_CONSTRAINTCOMPONENT2D_HPP
