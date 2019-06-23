RegisterClientScript("rifle.lua")

WEAPON.Scale = 0.2
WEAPON.Sprite = "rifle-30349_640.png"
WEAPON.SpriteOrigin = Vec2(-100, 105) * WEAPON.Scale
WEAPON.WeaponOffset = Vec2(0, -40) -- This should not be here
WEAPON.Cooldown = 0.1

WEAPON.ShootSound = {
--[[    "sounds/FIREARM_Assault_Rifle_Model_01_Fire_Single_RR1_stereo.wav",
    "sounds/FIREARM_Assault_Rifle_Model_01_Fire_Single_RR2_stereo.wav",
    "sounds/FIREARM_Assault_Rifle_Model_01_Fire_Single_RR3_stereo.wav",]]
    "placeholder/piou1.wav",
    "placeholder/piou2.wav",
    "placeholder/piou3.wav",
}

function WEAPON:OnAttack()
	if (CLIENT) then
		self:PlaySound(self.ShootSound[math.random(1, #self.ShootSound)], true, false, true)
	end

	local shootDir = self:GetDirection()
	local shootPos = self:GetPosition() + shootDir * 740 * self.Scale

	self:Shoot(shootPos, shootDir, 20, 10000)
end
