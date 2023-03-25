#!/bin/env lua

local tests = {
    "tests/die",
    "tests/full",
    "tests/red",
}

Exe = function(cmd)
    print("+ "..cmd)
    local success, result, code = os.execute(cmd)
    if result == "signal" then
        io.stderr:write("tests interrupted by signal "..tostring(code).."\n")
        os.exit(1)
    end
    assert(result == "exit")
    return success, code
end

if not Exe("cmake . -DFRONTEND=TEST && make -j") then
    return io.stderr:write("Could not run tests: Failed to build testminesector\n")
end
if not Exe("cmake . -DFRONTEND=NATIVE && make -j") then
    return io.stderr:write("Could not run tests: Failed to build minesector\n")
end

for _,file_name in ipairs(tests) do
    local success, code = Exe(string.format("./testminesector run %q", file_name))
    if not success then io.stderr:write("Test exited with code "..code.."\n") end
end
