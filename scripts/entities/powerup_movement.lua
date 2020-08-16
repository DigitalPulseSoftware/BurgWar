RegisterClientScript()

local entity = ScriptedEntity({
	Base = "entity_powerup_base",
	IsNetworked = true,
	Properties = {
		{ Name = "effect_speed", Type = PropertyType.Float, Default = 1 },
		{ Name = "effect_jump_height", Type = PropertyType.Float, Default = 1 },
		{ Name = "effect_jump_height_boost", Type = PropertyType.Float, Default = 1 },
	}
})

if (SERVER) then
	function entity:Apply(playerEnt)
		local movementSpeed = playerEnt:GetPlayerMovementSpeed()
		local jumpHeight, jumpBoostHeight = playerEnt:GetPlayerJumpHeight()

		local newSpeed = movementSpeed * self:GetProperty("effect_speed")
		local newJumpHeight = jumpHeight * self:GetProperty("effect_jump_height")
		local newJumpBoostHeight = jumpBoostHeight * self:GetProperty("effect_jump_height_boost")

		playerEnt:UpdatePlayerMovementSpeed(newSpeed)
		playerEnt:UpdatePlayerJumpHeight(newJumpHeight, newJumpBoostHeight)

		return {
			movementSpeed = movementSpeed,
			jumpHeight = jumpHeight,
			jumpBoostHeight = jumpBoostHeight
		}
	end

	function entity:Unapply(playerEnt, data)
		playerEnt:UpdatePlayerMovementSpeed(data.movementSpeed)
		playerEnt:UpdatePlayerJumpHeight(data.jumpHeight, data.jumpBoostHeight)
	end
end
