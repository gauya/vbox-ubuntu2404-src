dofile("s0.lua")

szFile = io.open("README","r")

for line in szFile:lines() do
  print(line)
end


print("hello".." world")

