#!/bin/env lua

local tests = {
    "tests/die",
    "tests/full",
}

if not os.execute("cmake . -DFRONTEND_TEST=1") then
    error("Could not run tests: CMake Failed")
end
if not os.execute("make") then
    error("Could not run tests: Make Failed")
end

for _,file_name in ipairs(tests) do
    os.execute(string.format("./minesector run %q", file_name))
end
