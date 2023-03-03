#!/bin/env lua

local tests = {
    "tests/die",
    "tests/full",
    "tests/red",
}

if not os.execute("cmake . -DFRONTEND_TEST=1") then
    return io.stderr:write("Could not run tests: CMake Failed\n")
end
if not os.execute("make -j") then
    return io.stderr:write("Could not run tests: Make Failed\n")
end

for _,file_name in ipairs(tests) do
    os.execute(string.format("./minesector run %q", file_name))
end
