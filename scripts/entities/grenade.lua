RegisterClientScript("grenade.lua")

ENTITY.IsNetworked = true
ENTITY.PlayerControlled = false
ENTITY.MaxHealth = 50

ENTITY.Properties = {
	{ Name = "lifetime", Type = PropertyType.Float, Default = 1.0, Shared = true }
}

ENTITY.ExplosionSounds = {
    "placeholder/explosion1.wav",
    "placeholder/explosion2.wav",
    "placeholder/explosion3.wav",
    "placeholder/explosion4.wav",
}

function ENTITY:Initialize()
	self.ExplosionTick = GM:GetMatchTick() + self:GetProperty("lifetime") / GM:GetTickDuration()
	self:SetCollider(Circle(Vec2(0, 0) * 0.2, 128 * 0.2))
	self:InitRigidBody(20, 10)

	if (CLIENT) then
		self:AddSprite("grenade.png", Vec2(0.2, 0.2))
	end
end

function ENTITY:OnTick()
	local currentTick = GM:GetMatchTick()
	if (currentTick >= self.ExplosionTick) then
		if (SERVER) then
			self:Kill()
			self:Explode()
		end
	end
end

function ENTITY:Explode()
	if (CLIENT) then
		self:PlaySound(self.ExplosionSounds[math.random(1, #self.ExplosionSounds)], false, false, true)
	end

	local pos = self:GetPosition()
	local maxs = Vec2(256, 256)
	local mins = Vec2(-256, -256)

	GM:DealDamage(self:GetPosition(), 200, Rect(pos + mins, pos + maxs), 50000)
end

if (CLIENT) then
	function ENTITY:OnKilled()
		self:Explode()
	end
end
