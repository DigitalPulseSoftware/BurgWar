RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "target_entity", Type = PropertyType.Entity, Shared = true },
		{ Name = "target_offset", Type = PropertyType.FloatPosition, Default = Vec2(0, 0), Shared = true },
	}
})

entity:On("init", function (self)
	self:InitRigidBody(0)

	local targetEntity = self:GetProperty("target_entity")
	local targetOffset = self:GetProperty("target_offset")
	if (targetEntity == NoEntity or not targetEntity:IsValid()) then
		self:Remove()
		return
	end

	self.PositionConstraint = physics.CreatePivotConstraint(self, targetEntity, Vec2(0,0), targetOffset)
end)

function entity:OnKilled()
	if (self.PositionConstraint) then
		self.PositionConstraint:Remove()
	end
end
