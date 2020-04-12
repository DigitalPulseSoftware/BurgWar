RegisterClientScript()
RegisterClientAssets("placeholder/cloves_base.png")
RegisterClientAssets("placeholder/cloves_eye.png")

ENTITY.ExplosionSounds = {
    "placeholder/explosion1.wav",
    "placeholder/explosion2.wav",
    "placeholder/explosion3.wav",
    "placeholder/explosion4.wav",
}
RegisterClientAssets(ENTITY.ExplosionSounds)

ENTITY.IsNetworked = true
ENTITY.MaxHealth = 1

ENTITY.Properties = {}

function ENTITY:Initialize()
	local centerOffset = -Vec2(96, 96) / 2
	local colliders = {
		Circle(Vec2(48, 27) + centerOffset, 21),
		Rect(Vec2(41, 47) + centerOffset, Vec2(55, 94) + centerOffset)
	}

	self:SetCollider(colliders)
	self:InitRigidBody(0, 10)

	if (CLIENT) then
		self:AddSprite({
			RenderOrder = -5,
			TexturePath = "placeholder/cloves_base.png"
		})

		self.Eyes = {}
		local function AddEye(pos)
			local sprite = self:AddSprite({
				Offset = pos + centerOffset,
				Scale = Vec2(0.2, 0.2),
				TexturePath = "placeholder/cloves_eye.png"
			})
			if (not EDITOR) then
				sprite:Hide()
			end

			table.insert(self.Eyes, sprite)
		end
		
		AddEye(Vec2(35, 35))
		AddEye(Vec2(61, 35))
	end
end

function ENTITY:OnTick()
	local pos = self:GetPosition()
	local size = Vec2(256, 256)
	local rect = Rect(pos - size, pos + size)

	local closestPlayerDistSq
	physics.RegionQuery(self:GetLayerIndex(), rect, function (entity)
		if (entity.Name == "burger") then
			local distSq = pos:SquaredDistance(entity:GetPosition())
			closestPlayerDistSq = closestPlayerDistSq and math.min(closestPlayerDistSq, distSq) or distSq
		end
	end)

	local closestPlayerDist = closestPlayerDistSq and math.sqrt(closestPlayerDistSq) or nil

	if (SERVER) then
		if (closestPlayerDist and closestPlayerDist < 256) then
			self:Explode()
		end
	else
		if (closestPlayerDist) then
			for _, eye in pairs(self.Eyes) do
				eye:Show()
			end
		else
			for _, eye in pairs(self.Eyes) do
				eye:Hide()
			end
		end
	end

	local nextTick = closestPlayerDist and closestPlayerDist / 128 or 2
	return nextTick
end

if (SERVER) then
	function ENTITY:Explode()
		if (self.HasExploded) then
			return
		end

		local height = 64
		local time = 200

		self.HasExploded = true
		self:SetVelocity(self:GetUpVector() * -height / time * 1000)
		coroutine.wrap(function()
			timer.Sleep(time)
			self:SetMass(10, true)
			timer.Sleep(500)
			local pos = self:GetPosition()
			local maxs = Vec2(256, 256)
			local mins = Vec2(-256, -256)

			self:DealDamage(self:GetPosition(), math.random(500, 1000), Rect(pos + mins, pos + maxs), 100000)
			
			self:Kill()
		end)()
	end
end

if (CLIENT) then
	function ENTITY:OnKilled()
		if (self:GetHealth() == 0) then
			self:PlaySound(self.ExplosionSounds[math.random(1, #self.ExplosionSounds)], false, false, true)
	
			local playerPosition = engine_GetPlayerPosition(0)
			if (playerPosition) then
				local distance = (self:GetPosition() - playerPosition):Length()
				local minDistance = 1000
				local strength = math.min(50, 250 / math.max(1, distance - minDistance))
				if (strength > 0.1) then
					GM:ShakeCamera(1, strength)
				end
			end
	
			match.CreateEntity({
				Type = "entity_effect_smoke",
				LayerIndex = self:GetLayerIndex(),
				Position = self:GetPosition(),
				Properties = {
					lifetime = math.random(2, 3)
				}
			})
		end
	end
end
