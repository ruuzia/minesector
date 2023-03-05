#!/bin/env lua

local tests = {
    "tests/die",
    "tests/full",
    "tests/red",
}

if not os.execute("cmake . -DFRONTEND=TEST && make -j") then
    return io.stderr:write("Could not run tests: Failed to build testminesector\n")
end
if not os.execute("cmake . -DFRONTEND=NATIVE && make -j") then
    return io.stderr:write("Could not run tests: Failed to build minesector\n")
end

for _,file_name in ipairs(tests) do
    os.execute(string.format("./testminesector run %q", file_name))
end
