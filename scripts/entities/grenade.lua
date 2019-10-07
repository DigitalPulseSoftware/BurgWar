RegisterClientScript("grenade.lua")
RegisterClientAssets("grenade.png")

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
RegisterClientAssets(ENTITY.ExplosionSounds)

function ENTITY:Initialize()
	self.ExplosionTick = GM:GetLocalTick() + self:GetProperty("lifetime") / GM:GetTickDuration()
	self:SetCollider(Circle(Vec2(0, 0) * 0.2, 128 * 0.2))
	self:InitRigidBody(20, 10)

	if (CLIENT) then
		self:AddSprite({
			Scale = Vec2(0.2, 0.2),
			TexturePath = "grenade.png"
		})
	end
end

function ENTITY:OnTick()
	local currentTick = GM:GetLocalTick()
	if (currentTick >= self.ExplosionTick) then
		self:Explode()

		if (SERVER) then
			self:Kill()
		end
	end
end

function ENTITY:Explode()
	if (self.Exploded) then
		return
	end

	self.Exploded = true

	if (CLIENT) then
		self:PlaySound(self.ExplosionSounds[math.random(1, #self.ExplosionSounds)], false, false, true)
	else
		local pos = self:GetPosition()
		local maxs = Vec2(256, 256)
		local mins = Vec2(-256, -256)

		self:DealDamage(self:GetPosition(), 200, Rect(pos + mins, pos + maxs), 100000)
	end
end

if (CLIENT) then
	function ENTITY:OnKilled()
		self:Explode()
	end
end
