RegisterClientScript("sword_emmentalibur.lua")

WEAPON.Scale = 0.6
WEAPON.Sprite = "../resources/emmentalibur.png"
WEAPON.SpriteOrigin = Vec2(40, 284) * WEAPON.Scale
WEAPON.WeaponOffset = Vec2(20, -60) -- This should not be here
WEAPON.Cooldown = 0.3
WEAPON.Animations = {
	{"attack", 0.3}
}

function WEAPON:OnAttack()
	if (self:IsPlayingAnimation()) then
		return
	end

	local pos = self:GetPosition()
	local maxs = Vec2(128, 66)
	local mins = Vec2(28, -76)

	if (not self:IsLookingRight()) then
		maxs = maxs * -1
		mins = mins * -1
	end

	self:PlayAnim("attack")
	self:DealDamage(100, Rect(pos + mins, pos + maxs), 50000)
end

if (CLIENT) then
	function WEAPON:OnAnimationStart(animationId)
		local startRotation = self:GetRotation()
		local endRotation = self:IsLookingRight() and 110 or -110

		animation.Rotate(self.Entity, startRotation, endRotation, 0.1)
		animation.Rotate(self.Entity, endRotation, startRotation, 0.2)
	end
end
