RegisterClientScript()

local scale = 0.75

local weapon = ScriptedWeapon({
	Cooldown = 0.5,
	Scale = scale,
	Sprite = "placeholder/physgun.png",
	SpriteOrigin = Vec2(40, 20) * scale,
	WeaponOffset = Vec2(20, -60) -- This should not be here
})

RegisterClientAssets(weapon.Sprite)

local maxDist = 1000

weapon:On("attack", function (self)
	if (self.Constraint) then
		self:Release()
	else
		self:Launch()
	end
end)

function weapon:Launch()
	local startOffset = Vec2(85 * self.Scale, 0)

	local startPos = self:ToGlobalPosition(startOffset)
	local endPos = startPos + self:GetDirection() * maxDist

	local nearestFraction = math.huge
	local nearestResult

	local traceResult = physics.TraceMultiple(self:GetLayerIndex(), startPos, endPos, function (result)
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
		LifeOwner = self,
		Position = nearestResult.hitPos,
		Properties = {
			target_entity = nearestResult.hitEntity,
			target_offset = nearestResult.hitEntity:ToLocalPosition(nearestResult.hitPos)
		}
	})
end

function weapon:Release()
	if (self.Constraint and self.Constraint:IsValid()) then
		self.Constraint:Remove()
	end
	self.Constraint = nil
end

weapon:On("tick", function (self)
	if (self.Constraint and self.Constraint:IsValid()) then
		self.Constraint:SetPosition(self:ToGlobalPosition(self.EntityOffset))
	end
end)
