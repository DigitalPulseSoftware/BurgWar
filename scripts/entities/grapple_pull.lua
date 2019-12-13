RegisterClientScript()

ENTITY.IsNetworked = false -- TODO

ENTITY.Properties = {}

function ENTITY:Initialize()
end

--FIXME: This is required because entities cannot be used as properties yet
function ENTITY:GrappleInit()
	if (not self.SourceEntity:IsValid()) then
		self:Remove()
	end

	self.Constraint = physics.CreateDampenedSpringConstraint(self.SourceEntity, self.TargetEntity, self.SourceOffset, self.TargetOffset, 10, 500, 0)
end

function ENTITY:OnKilled()
	if (self.Constraint) then
		self.Constraint:Remove()
	end
end
