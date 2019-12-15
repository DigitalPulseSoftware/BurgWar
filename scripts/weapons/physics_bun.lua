RegisterClientScript()

WEAPON.Cooldown = 0.5
WEAPON.Scale = 0.75
WEAPON.Sprite = "placeholder/physgun.png"
WEAPON.SpriteOrigin = Vec2(40, 20) * WEAPON.Scale
WEAPON.WeaponOffset = Vec2(20, -60) -- This should not be here
WEAPON.Animations = {}

RegisterClientAssets(WEAPON.Sprite)

local maxDist = 1000

function WEAPON:OnAttack()
	if (self.Constraint) then
		self:Release()
	else
		self:Launch()
	end
end

function WEAPON:Launch()
	local startOffset = Vec2(85 * self.Scale, 0)

	local startPos = self:ToGlobalPosition(startOffset)
	local endPos = startPos + self:GetDirection() * maxDist

	local nearestFraction = math.huge
	local nearestResult

	local traceResult = physics.TraceMultiple(0, startPos, endPos, function (result)
		if (result.fraction < nearestFraction) then
			if (result.hitEntity) then
				-- Ignore player
				if (result.hitEntity == self:GetOwnerEntity()) then
					return
				end
			end

			nearestResult = result
			nearestFraction = result.fraction
		end
	end)

	if (not nearestResult) then
		return
	end

	local entityMass = nearestResult.hitEntity:GetMass()
	if (entityMass <= 0) then
		return
	end

	self.EntityOffset = self:ToLocalPosition(nearestResult.hitPos)

	self.Constraint = match.CreateEntity({
		Type = "entity_constraint_position",
		LayerIndex = self:GetLayerIndex(),
		Position = nearestResult.hitPos,
		Properties = {
			target_entity = nearestResult.hitEntity,
			target_offset = nearestResult.hitEntity:ToLocalPosition(nearestResult.hitPos)
		}
	})
end

function WEAPON:Release()
	if (self.Constraint and self.Constraint:IsValid()) then
		self.Constraint:Remove()
	end
	self.Constraint = nil
end

function WEAPON:OnTick()
	if (self.Constraint and self.Constraint:IsValid()) then
		self.Constraint:SetPosition(self:ToGlobalPosition(self.EntityOffset))
	end
end
