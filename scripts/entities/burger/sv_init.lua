RegisterClientScript("cl_init.lua")
RegisterClientScript("shared.lua")

function ENTITY:OnHealthChange(newHealth)
end

function ENTITY:OnDeath(attacker)
	print("Je meurs, oksour maman, eurgh")
end
