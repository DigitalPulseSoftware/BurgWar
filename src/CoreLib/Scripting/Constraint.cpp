// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CoreLib/Scripting/Constraint.hpp>
#include <NDK/Components/ConstraintComponent2D.hpp>

namespace bw
{
	Constraint::~Constraint() = default;
	
	void Constraint::EnableBodyCollision(bool enable)
	{
		AssertValid();
		m_constraint->EnableBodyCollision(enable);
	}
	
	bool Constraint::IsBodyCollisionEnabled() const
	{
		AssertValid();
		return m_constraint->IsBodyCollisionEnabled();
	}
	
	void Constraint::Remove()
	{
		AssertValid();
		if (m_entity && !m_entity->GetComponent<Ndk::ConstraintComponent2D>().RemoveConstraint(m_constraint))
			m_entity->Kill();
	}

	void Constraint::AssertValid() const
	{
		if (!IsValid())
			throw std::runtime_error("Constraint has been removed");
	}


	float PinConstraint::GetDistance() const
	{
		AssertValid();
		return GetConstraint<Nz::PinConstraint2D>()->GetDistance();
	}

	void PinConstraint::SetDistance(float distance)
	{
		AssertValid();
		GetConstraint<Nz::PinConstraint2D>()->SetDistance(distance);
	}


	Nz::RadianAnglef RotaryLimitConstraint::GetMaxAngle() const
	{
		AssertValid();
		return GetConstraint<Nz::RotaryLimitConstraint2D>()->GetMaxAngle();
	}

	Nz::RadianAnglef RotaryLimitConstraint::GetMinAngle() const
	{
		AssertValid();
		return GetConstraint<Nz::RotaryLimitConstraint2D>()->GetMinAngle();
	}

	void RotaryLimitConstraint::SetMaxAngle(Nz::RadianAnglef maxAngle)
	{
		AssertValid();
		GetConstraint<Nz::RotaryLimitConstraint2D>()->SetMaxAngle(maxAngle);
	}

	void RotaryLimitConstraint::SetMinAngle(Nz::RadianAnglef minAngle)
	{
		AssertValid();
		GetConstraint<Nz::RotaryLimitConstraint2D>()->SetMinAngle(minAngle);
	}
}
