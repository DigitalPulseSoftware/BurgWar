// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef BURGWAR_CORELIB_SCRIPTING_CONSTRAINT_HPP
#define BURGWAR_CORELIB_SCRIPTING_CONSTRAINT_HPP

#include <Nazara/Physics2D/Constraint2D.hpp>
#include <NDK/Entity.hpp>

namespace bw
{
	class Constraint
	{
		public:
			Constraint(Ndk::EntityHandle entity, Nz::Constraint2DHandle constraint);
			Constraint(const Constraint&) = delete;
			Constraint(Constraint&& constraint) noexcept;
			virtual ~Constraint();

			void EnableBodyCollision(bool enable);

			float GetErrorBias() const;
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
			NazaraSlot(Nz::HandledObject<Nz::Constraint2D>, OnHandledObjectDestruction, m_onDestruction);

			Ndk::EntityHandle m_entity;
			Nz::Constraint2DHandle m_constraint;
	};

	class DampedSpringConstraint : public Constraint
	{
		public:
			inline DampedSpringConstraint(Ndk::EntityHandle entity, Nz::DampedSpringConstraint2D* constraint);
			DampedSpringConstraint(DampedSpringConstraint&&) noexcept = default;
			~DampedSpringConstraint() = default;

			DampedSpringConstraint& operator=(const DampedSpringConstraint&) = delete;
			DampedSpringConstraint& operator=(DampedSpringConstraint&&) = delete;
	};

	class PinConstraint : public Constraint
	{
		public:
			inline PinConstraint(Ndk::EntityHandle entity, Nz::PinConstraint2D* constraint);
			PinConstraint(PinConstraint&&) noexcept = default;
			~PinConstraint() = default;

			float GetDistance() const;

			void SetDistance(float distance);

			PinConstraint& operator=(const PinConstraint&) = delete;
			PinConstraint& operator=(PinConstraint&&) = delete;
	};

	class PivotConstraint : public Constraint
	{
	public:
		inline PivotConstraint(Ndk::EntityHandle entity, Nz::PivotConstraint2D* constraint);
		PivotConstraint(PivotConstraint&&) noexcept = default;
		~PivotConstraint() = default;

		PivotConstraint& operator=(const PivotConstraint&) = delete;
		PivotConstraint& operator=(PivotConstraint&&) = delete;
	};

	class RotaryLimitConstraint : public Constraint
	{
		public:
			inline RotaryLimitConstraint(Ndk::EntityHandle entity, Nz::RotaryLimitConstraint2D* constraint);
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
