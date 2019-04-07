RegisterClientScript("rifle.lua")

WEAPON.Scale = 0.2
WEAPON.Sprite = "../resources/rifle-30349_640.png"
WEAPON.SpriteOrigin = Vec2(-100, 105) * WEAPON.Scale
WEAPON.WeaponOffset = Vec2(0, -40) -- This should not be here
WEAPON.Animations = {}

function WEAPON:OnAttack()
	local shootDir = self:GetDirection()
	local shootPos = self:GetPosition() + shootDir * 740 * self.Scale

	self:Shoot(shootPos, shootDir, 100, 50000)
end
