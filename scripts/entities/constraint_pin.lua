RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "source_entity", Type = PropertyType.Entity, Shared = true },
		{ Name = "source_offset", Type = PropertyType.FloatPosition, Default = Vec2(0, 0), Shared = true },
		{ Name = "target_entity", Type = PropertyType.Entity, Shared = true },
		{ Name = "target_offset", Type = PropertyType.FloatPosition, Default = Vec2(0, 0), Shared = true },
	}
})

entity:On("init", function (self)
	local sourceEntity = self:GetProperty("source_entity")
	local sourceOffset = self:GetProperty("source_offset")
	local targetEntity = self:GetProperty("target_entity")
	local targetOffset = self:GetProperty("target_offset")
	if (sourceEntity == NoEntity or not sourceEntity:IsValid()) then
		self:Remove()
		return
	end

	if (targetEntity == NoEntity or not targetEntity:IsValid()) then
		self:Remove()
		return
	end

	self.Constraint = physics.CreatePinConstraint(sourceEntity, targetEntity, sourceOffset, targetOffset)
end)

function entity:OnKilled()
	if (self.Constraint) then
		self.Constraint:Remove()
	end
end
