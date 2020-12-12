RegisterClientScript()

local entity = ScriptedEntity({
	Base = "entity_powerup_base",
	IsNetworked = true,
	Properties = {
		{ Name = "effect_scale", Type = PropertyType.Float, Default = 1 }
	}
})

if (SERVER) then
	function entity:Scale(playerEnt, scale)
		coroutine.wrap(function()
			timer.Sleep(0)
			playerEnt:SetScale(scale)
		end)()
	end

	entity:On("apply", function (self, playerEnt)
		local scale = self:GetProperty("effect_scale")
		self:Scale(playerEnt, scale)
	end)

	entity:On("unapply", function (self, playerEnt)
		self:Scale(playerEnt, 1.0)
	end)
end