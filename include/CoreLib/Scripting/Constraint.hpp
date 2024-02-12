// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CONSTRAINT_HPP
#define BURGWAR_CORELIB_SCRIPTING_CONSTRAINT_HPP

#include <CoreLib/Export.hpp>
#include <Nazara/Physics2D/PhysConstraint2D.hpp>
#include <entt/entt.hpp>

namespace bw
{
	class BURGWAR_CORELIB_API Constraint
	{
		public:
			Constraint(entt::handle entity, Nz::PhysConstraint2DHandle constraint);
			Constraint(const Constraint&) = delete;
			Constraint(Constraint&& constraint) noexcept;
			virtual ~Constraint();

			void EnableBodyCollision(bool enable);

			float GetErrorBias() const;
			float GetLastImpulse() const;
			float GetMaxForce() const;

			bool IsBodyCollisionEnabled() const;
			inline bool IsValid() const;

			void Remove();

			void SetErrorBias(float errorBias);
			void SetMaxForce(float maxForce);

			Constraint& operator=(const Constraint&) = delete;
			Constraint& operator=(Constraint&&) = delete;

		protected:
			void AssertValid() const;
			template<typename T> T* GetConstraint();
			template<typename T> const T* GetConstraint() const;
			void KillEntity();

		private:
			NazaraSlot(Nz::HandledObject<Nz::PhysConstraint2D>, OnHandledObjectDestruction, m_onDestruction);

			entt::handle m_entity;
			Nz::PhysConstraint2DHandle m_constraint;
	};

	class BURGWAR_CORELIB_API DampedSpringConstraint : public Constraint
	{
		public:
			inline DampedSpringConstraint(entt::handle entity, Nz::PhysDampedSpringConstraint2D* constraint);
			DampedSpringConstraint(DampedSpringConstraint&&) noexcept = default;
			~DampedSpringConstraint() = default;

			DampedSpringConstraint& operator=(const DampedSpringConstraint&) = delete;
			DampedSpringConstraint& operator=(DampedSpringConstraint&&) = delete;
	};

	class BURGWAR_CORELIB_API PinConstraint : public Constraint
	{
		public:
			inline PinConstraint(entt::handle entity, Nz::PhysPinConstraint2D* constraint);
			PinConstraint(PinConstraint&&) noexcept = default;
			~PinConstraint() = default;

			float GetDistance() const;

			void SetDistance(float distance);

			PinConstraint& operator=(const PinConstraint&) = delete;
			PinConstraint& operator=(PinConstraint&&) = delete;
	};

	class BURGWAR_CORELIB_API PivotConstraint : public Constraint
	{
		public:
			inline PivotConstraint(entt::handle entity, Nz::PhysPivotConstraint2D* constraint);
			PivotConstraint(PivotConstraint&&) noexcept = default;
			~PivotConstraint() = default;

			PivotConstraint& operator=(const PivotConstraint&) = delete;
			PivotConstraint& operator=(PivotConstraint&&) = delete;
	};

	class BURGWAR_CORELIB_API RotaryLimitConstraint : public Constraint
	{
		public:
			inline RotaryLimitConstraint(entt::handle entity, Nz::PhysRotaryLimitConstraint2D* constraint);
			RotaryLimitConstraint(RotaryLimitConstraint&&) noexcept = default;
			~RotaryLimitConstraint() = default;

			Nz::RadianAnglef GetMaxAngle() const;
			Nz::RadianAnglef GetMinAngle() const;

			void SetMaxAngle(Nz::RadianAnglef minAngle);
			void SetMinAngle(Nz::RadianAnglef maxAngle);

			RotaryLimitConstraint& operator=(const RotaryLimitConstraint&) = delete;
			RotaryLimitConstraint& operator=(RotaryLimitConstraint&&) = delete;
	};
}

#include <CoreLib/Scripting/Constraint.inl>

#endif
