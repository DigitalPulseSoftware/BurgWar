RegisterClientScript()

ENTITY.IsNetworked = false -- TODO

ENTITY.Properties = {}

function ENTITY:Initialize()
end

--FIXME: This is required because entities cannot be used as properties yet
function ENTITY:TempInit()
	if (not self.SourceEntity:IsValid()) then
		self:Remove()
		return
	end

	if (not self.TargetEntity:IsValid()) then
		self:Remove()
		return
	end

	self.Constraint = physics.CreatePinConstraint(self.SourceEntity, self.TargetEntity, self.SourceOffset, self.TargetOffset)
end

function ENTITY:OnKilled()
	if (self.Constraint) then
		self.Constraint:Remove()
	end
end
