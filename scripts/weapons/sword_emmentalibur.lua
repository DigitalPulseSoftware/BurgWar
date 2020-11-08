RegisterClientScript()

local scale = 0.6

local weapon = ScriptedWeapon({
	Cooldown = 0.4,
	Scale = scale,
	Sprite = "emmentalibur.png",
	SpriteOrigin = Vec2(40, 284) * scale,
	WeaponOffset = Vec2(20, -60), -- This should not be here
	AttackMode = WeaponAttackType.SingleShotRepeat
})

RegisterClientAssets(weapon.Sprite)

if (SERVER) then
	local rectSize = 64
	local halfRectSize = rectSize / 2

	weapon:On("attack", function (self)
		local pos = self:GetPosition()
		local dir = self:GetDirection()
		local scale = self:GetScale()

		local width = rectSize + halfRectSize * math.abs(dir.x)
		local height = rectSize + halfRectSize * math.abs(dir.y)

		local maxs = Vec2(width, height)
		local mins = Vec2(-width, -height)
	
		local origin = pos + dir * scale * 75
		local rect = Rect(origin + mins * scale, origin + maxs * scale)

		local ownerEntity = self:GetOwnerEntity()
		physics.RegionQuery(self:GetLayerIndex(), rect, function (entity)
			if (entity == ownerEntity) then
				return
			end

			entity:ApplyImpulse(dir * 10000)
			entity:Damage(math.random(50, 150))
		end)
	end)
end

if (CLIENT) then
	weapon:OnAsync("attack", function (self)
		local startRotation = self:GetRotation()
		local endRotation
		if (self:IsLookingRight()) then
			endRotation = startRotation + 180
		else
			endRotation = startRotation - 180
		end

		animation.Rotate(self, startRotation, endRotation, 0.1)
		animation.Rotate(self, endRotation, startRotation, 0.2)
	end)
end
