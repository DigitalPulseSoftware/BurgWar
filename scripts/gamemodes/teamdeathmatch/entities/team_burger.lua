RegisterClientScript()

local entity = ScriptedEntity({
	Base = "entity_burger",
	IsNetworked = true,
	Properties = {
		{ Name = "bread", Type = PropertyType.Integer, Shared = true },
	}
})

function entity:BuildBurger(AddElement)
	local bread = self:GetProperty("bread")

	AddElement("Bottoms", bread)
	AddElement("Vegetables")
	AddElement("Sauces")
	AddElement("Meats")
	AddElement("Sauces")
	AddElement("Vegetables")
	AddElement("Cheeses")
	AddElement("Tops", bread)
end
