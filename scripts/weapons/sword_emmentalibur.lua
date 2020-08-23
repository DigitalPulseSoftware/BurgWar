RegisterClientScript()

local scale = 0.6

local weapon = ScriptedWeapon({
	Cooldown = 0.4,
	Scale = scale,
	Sprite = "emmentalibur.png",
	SpriteOrigin = Vec2(40, 284) * scale,
	WeaponOffset = Vec2(20, -60), -- This should not be here
	Animations = {
		{"attack", 0.3}
	},
	AttackMode = WeaponAttackType.SingleShotRepeat
})

RegisterClientAssets(weapon.Sprite)

if (SERVER) then
	weapon:On("attack", function (self)
		local pos = self:GetPosition()
		local maxs = Vec2(128, 66)
		local mins = Vec2(28, -76)

		if (not self:IsLookingRight()) then
			maxs = maxs * -1
			mins = mins * -1
		end

		self:PlayAnim("attack")
		self:DealDamage(pos, 100, Rect(pos + mins, pos + maxs), 20000)
	end)
end

if (CLIENT) then
	function weapon:OnAnimationStart(animationId)
		local startRotation = self:GetRotation()
		local endRotation
		if (self:IsLookingRight()) then
			endRotation = startRotation + 180
		else
			endRotation = startRotation - 180
		end

		animation.Rotate(self, startRotation, endRotation, 0.1)
		animation.Rotate(self, endRotation, startRotation, 0.2)
	end
end
