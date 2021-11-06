RegisterClientScript("rifle.lua")

local scale = 0.2

local weapon = ScriptedWeapon({
	Category = 3,
	Cooldown = 0.1,
	Scale = scale,
	Sprite = "rifle-30349_640.png",
	SpriteOrigin = Vec2(-100, 105) * scale,
	WeaponOffset = Vec2(0, -40) -- This should not be here
})

RegisterClientAssets(weapon.Sprite)

weapon.ShootSound = {
--[[    "sounds/FIREARM_Assault_Rifle_Model_01_Fire_Single_RR1_stereo.wav",
    "sounds/FIREARM_Assault_Rifle_Model_01_Fire_Single_RR2_stereo.wav",
    "sounds/FIREARM_Assault_Rifle_Model_01_Fire_Single_RR3_stereo.wav",]]
    "placeholder/piou1.wav",
    "placeholder/piou2.wav",
    "placeholder/piou3.wav",
}
RegisterClientAssets(weapon.ShootSound)


weapon:On("attack", function (self)
	if (CLIENT) then
		self:PlaySound(self.ShootSound[math.random(1, #self.ShootSound)], true, false, true)
	end

	local shootDir = self:GetDirection()
	local shootPos = self:GetPosition() + shootDir * 740 * self.Scale

	self:Shoot(shootPos, shootDir, 10, 10000)
end)
