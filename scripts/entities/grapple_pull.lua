RegisterClientScript()

local entity = ScriptedEntity({
	IsNetworked = true,
	Properties = {
		{ Name = "source_entity", Type = PropertyType.Entity, Shared = true },
		{ Name = "source_offset", Type = PropertyType.FloatPosition, Shared = true },
		{ Name = "target_entity", Type = PropertyType.Entity, Shared = true },
		{ Name = "target_offset", Type = PropertyType.FloatPosition, Shared = true },
		{ Name = "force", Type = PropertyType.Integer, Default = 100, Shared = true }
	}
})

entity:On("Init", function (self)
	local sourceEntity = self:GetProperty("source_entity")
	local sourceOffset = self:GetProperty("source_offset")
	local targetEntity = self:GetProperty("target_entity")
	local targetOffset = self:GetProperty("target_offset")
	if (sourceEntity == NoEntity or not sourceEntity:IsValid()) then
		self:Remove()
		return
	end

	self.Constraint = physics.CreateDampenedSpringConstraint(sourceEntity, targetEntity, sourceOffset, targetOffset, 10, self:GetProperty("force"), 0)
end)

entity:On("Destroyed", function (self)
	if (self.Constraint) then
		self.Constraint:Remove()
	end
end)
