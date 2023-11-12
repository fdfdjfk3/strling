local os = require("os")

local flags = "-o strling"

local src = "src/"
local source_files = table.concat({
	src .. "*.c",
	src .. "cli/*.c",
	src .. "slstd/*.c",
	src .. "interner/*.c",
}, " ")

local function main()
	local extra_args = "-O3 -s"
	if #arg > 0 then
		extra_args = table.concat(arg, " ")
		print("Compiling with '" .. extra_args .. "'...")
	else
		print("Compiling release build...")
	end

	local success, _, code =
		os.execute("gcc " .. source_files .. " " .. flags .. " " .. extra_args)

	if success == false then
		print("gcc failed with exit code: " .. tostring(code))
		os.exit(false)
	end
end

main()


