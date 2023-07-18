// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Burgwar" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/Constraint.hpp>
#include <cassert>

namespace bw
{
	inline bool Constraint::IsValid() const
	{
		return m_constraint.IsValid();
	}

	template<typename T>
	T* Constraint::GetConstraint()
	{
		assert(dynamic_cast<T*>(m_constraint.GetObject()) != nullptr);
		return static_cast<T*>(m_constraint.GetObject());
	}

	template<typename T>
	const T* Constraint::GetConstraint() const
	{
		assert(dynamic_cast<T*>(m_constraint.GetObject()) != nullptr);
		return static_cast<T*>(m_constraint.GetObject());
	}

	inline DampedSpringConstraint::DampedSpringConstraint(entt::handle entity, Nz::ChipmunkDampedSpringConstraint2D* constraint) :
	Constraint(entity, constraint->CreateHandle())
	{
	}

	inline PinConstraint::PinConstraint(entt::handle entity, Nz::ChipmunkPinConstraint2D* constraint) :
	Constraint(entity, constraint->CreateHandle())
	{
	}
	
	inline PivotConstraint::PivotConstraint(entt::handle entity, Nz::ChipmunkPivotConstraint2D* constraint) :
	Constraint(entity, constraint->CreateHandle())
	{
	}

	inline RotaryLimitConstraint::RotaryLimitConstraint(entt::handle entity, Nz::ChipmunkRotaryLimitConstraint2D* constraint) :
	Constraint(entity, constraint->CreateHandle())
	{
	}
}
