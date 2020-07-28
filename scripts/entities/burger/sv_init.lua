RegisterClientScript("cl_init.lua")
RegisterClientScript("shared.lua")

local entity = ScriptedEntity()

function entity:OnHealthChange(newHealth)
end

function entity:OnDeath(attacker)
	print("Je meurs, oksour maman, eurgh")
end
