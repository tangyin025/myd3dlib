for obj in theApp.NamedObjects do
	local postfix=string.match(obj.Name, "dialog0_static1_(.+)")
	if postfix then
		print(obj.Name,postfix)
	end
end