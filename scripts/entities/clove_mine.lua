RegisterClientScript()
RegisterClientAssets("placeholder/cloves_base.png")
RegisterClientAssets("placeholder/cloves_eye.png")

local entity = ScriptedEntity({
	IsNetworked = true,
	MaxHealth = 1,
	Properties = {
		{ Name = "free", Type = PropertyType.Boolean, Default = true, Shared = true }
	}
})

entity.ExplosionSounds = {
	"placeholder/explosion1.wav",
	"placeholder/explosion2.wav",
	"placeholder/explosion3.wav",
	"placeholder/explosion4.wav",
}
RegisterClientAssets(entity.ExplosionSounds)

entity.Status = "free"

entity:On("init", function (self)
	local centerOffset = -Vec2(96, 96) / 2
	local colliders = {
		{
			Collider = Circle(Vec2(48, 27) + centerOffset, 21),
			ColliderType = SERVER and ColliderType.Callback or ColliderType.Default
		},
		{
			Collider = Rect(Vec2(41, 47) + centerOffset, Vec2(55, 94) + centerOffset),
			ColliderType = ColliderType.Default
		}
	}

	self:SetColliders(colliders)
	if (self:GetProperty("free")) then
		self:InitRigidBody(10, 10)
	else
		self:InitRigidBody(0, 10)
		self.Status = "digged"
	end

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
end)

entity:On("tick", function (self)
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
		if (self.ExplosionTime and match.GetMilliseconds() >= self.ExplosionTime) then
			self:Explode()
		elseif (closestPlayerDist and closestPlayerDist < 256) then
			self:Undig()
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
end)

if (SERVER) then
	entity:On("collisionstart", function (self, other)
		if (other.Name == self.Name) then
			return false
		end

		if (other:GetMass() == 0) then
			self:Dig()
		else
			self:Explode()
		end

		return true
	end)

	function entity:Dig()
		if (self.Status ~= "free") then
			return
		end

		local closestResult
		local closestEntityDist = math.huge

		physics.TraceMultiple(self:GetLayerIndex(), self:GetPosition() + self:GetUpVector() * -48, self:GetPosition() + self:GetUpVector() * 100, function (result)
			if (result.hitEntity == self or result.hitEntity:GetMass() > 0) then
				return
			end

			if (result.fraction < closestEntityDist) then
				closestResult = result
				closestEntityDist = result.fraction
			end
		end)

		if (closestResult) then
			self.Status = "digging"
			local digHeight = 36
			local time = 500

			self:SetMass(0)
			self:SetDirection(closestResult.hitNormal)
			self:SetPosition(closestResult.hitPos - closestResult.hitNormal * digHeight)
			self:SetVelocity(self:GetUpVector() * -digHeight / time * 1000)
			coroutine.wrap(function()
				timer.Sleep(time)
				self.Status = "digged"
			end)()
		end
	end

	function entity:Explode()
		if (self.Status ~= "free" and self.Status ~= "undigged") then
			return
		end

		local pos = self:GetPosition()
		local maxs = Vec2(256, 256)
		local mins = Vec2(-256, -256)

		self:DealDamage(self:GetPosition(), math.random(20, 70), Rect(pos + mins, pos + maxs), 100000)
		
		self:Kill()
	end

	function entity:Undig()
		if (self.Status ~= "digged") then
			return
		end

		self.Status = "undigging"

		local height = 64
		local time = 200

		self:SetVelocity(self:GetUpVector() * -height / time * 1000)
		coroutine.wrap(function()
			timer.Sleep(time)
			self:SetMass(10, true)
			self.ExplosionTime = match.GetMilliseconds() + 500
			self.Status = "undigged"
		end)()
	end
end

if (CLIENT) then
	entity:On("healthupdate", function (self, oldHealth, newHealth)
		if (newHealth == 0) then
			self:PlaySound(self.ExplosionSounds[math.random(1, #self.ExplosionSounds)], false, false, true)
	
			local playerPosition = engine_GetPlayerPosition(0)
			if (playerPosition) then
				local distance = (self:GetPosition() - playerPosition):Length()
				local minDistance = 1000
				local strength = math.min(50, 250 / math.max(1, distance - minDistance))
				if (strength > 0.1) then
					match.GetGamemode():ShakeCamera(1, strength)
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
	end)
end
