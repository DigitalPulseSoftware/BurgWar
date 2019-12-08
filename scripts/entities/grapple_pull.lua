RegisterClientScript()

ENTITY.IsNetworked = false -- TODO

ENTITY.Properties = {
	{ Name = "duration", Type = PropertyType.Float, Default = 0 },
}

function ENTITY:Initialize()
end

--FIXME: This is required because entities cannot be used as properties yet
function ENTITY:GrappleInit()
	self:UpdatePositions()

	self.Constraint = physics.CreateDampenedSpringConstraint(self.SourceEntity, self.TargetEntity, self.SourceOffset, self.TargetOffset, 10, 500, 0)
end

function ENTITY:OnKilled()
	self.Constraint:Remove()
end

function ENTITY:UpdatePositions()
	if (self.SourceEntity:IsValid()) then
		self.startPos = self.SourceEntity:ToGlobalPosition(self.SourceOffset)
	else
		self:Remove()
		return
	end

	if (self.TargetEntity:IsValid()) then
		self.endPos = self.TargetEntity:ToGlobalPosition(self.TargetOffset)
	end
end
