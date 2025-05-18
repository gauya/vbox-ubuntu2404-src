table = { "abc", 1, 2, name = 'han', age = 28, 999 }

print("pairs test")
for key, value in pairs(table) do
  print("key: " .. key, "value: " .. value)
end

print("")

print("ipairs test")
for index, value in ipairs(table) do
  print("index: " .. index, "value: " .. value)
end

table = {
  ["햄버거"] = 8000, ["치킨"] = 12000,
  ["피자"] = 15000, ["파스타"] = 14000,
  ["콜라"] = 1000, ["사이다"] = 1000
}

for key, value in pairs(table) do
  print(key, value)
end

local function add(a, b)
  print(a, b)
  return a + b
end

print(add(2, 1.2))

function average(...)
  local result = 0
  local arg = { ... }

  for i, v in ipairs(arg) do
    result = result + v
  end

  return result / #arg
end

print("average = " .. average(10, 5, 3, 4, 5, 6))

local year = 2024
local pi = 3.141592
local hi = "hello world"

print(string.format("%d년", year))
print(string.format("원주율 = %f", pi))
print(string.format("인사: %s", hi))


