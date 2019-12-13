RegisterClientScript()

ENTITY.IsNetworked = false -- TODO

ENTITY.Properties = {}

function ENTITY:Initialize()
	self:InitRigidBody(0)
end

--FIXME: This is required because entities cannot be used as properties yet
function ENTITY:TempInit()
	if (not self.TargetEntity:IsValid()) then
		self:Remove()
		return
	end

	self.PositionConstraint = physics.CreatePinConstraint(self, self.TargetEntity, Vec2(0,0), self.TargetOffset)
	self.PositionConstraint:SetDistance(0)
end

function ENTITY:OnKilled()
	if (self.PositionConstraint) then
		self.PositionConstraint:Remove()
	end
end
