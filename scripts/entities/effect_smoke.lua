RegisterClientScript()
RegisterClientAssets("smoke.png")

ENTITY.IsNetworked = true
ENTITY.Properties = {}

function ENTITY:Initialize()
	self:SetLifeTime(10)

	self.ParticleGroup = particle.CreateGroup(10, "billboard2d")
	self.ParticleGroup:AddController("alphaFromLife", { maxLife = 5 })
	self.ParticleGroup:AddController("life")
	--self.ParticleGroup:AddController("velocity")
	self.ParticleGroup:AddGenerator("alpha", { alpha = 255 })
	self.ParticleGroup:AddGenerator("life", { min = 5, max = 5 })
	self.ParticleGroup:AddGenerator("position", { origin = self:GetPosition(), maxDist = 100 })
	self.ParticleGroup:AddGenerator("size", { min = Vec2(50, 50), max = Vec2(200, 200) })
	self.ParticleGroup:SetRenderer("billboard", {
		texturePath = "smoke.png"
	})

	self.ParticleGroup:GenerateParticles(10)
end

function ENTITY:OnTick()
end

function ENTITY:OnKilled()
	self.ParticleGroup:Kill()
end