#!/usr/bin/env lua

-- 명령어 세트 정의
local commands = {}

-- 명령어 함수 정의
local function handle_hello(args)
  if #args > 0 then
    local name = table.concat(args, " ")
    print(string.format("안녕하세요, %s님!", name))
  else
    print("안녕하세요!")
  end
end

local function handle_add(args)
  if #args == 2 then
    local num1 = tonumber(args[1])
    local num2 = tonumber(args[2])
    if num1 and num2 then
      print(string.format("%d + %d = %d", num1, num2, num1 + num2))
    else
      print("오류: 유효한 숫자를 입력해주세요.")
    end
  else
    print("오류: 두 개의 숫자를 입력해주세요.")
  end
end

local function handle_greet(args)
  if #args == 1 then
    print(string.format("좋은 %s입니다!", args[1]))
  else
    print("사용법: greet <아침|점심|저녁>")
  end
end

local function handle_exit()
  print("CLI를 종료합니다.")
  os.exit(0)
end

local function handle_help()
  print("사용 가능한 명령:")
  for cmd, _ in pairs(commands) do
    print("  " .. cmd)
  end
end

-- 명령어 테이블에 할당
commands["hello"] = handle_hello
commands["add"] = handle_add
commands["greet"] = handle_greet
commands["exit"] = handle_exit
commands["help"] = handle_help


-- 프롬프트 표시 및 입력 처리 무한 루프
while true do
  io.write("> ") -- 프롬프트 표시
  local line = io.read() -- 사용자 입력 읽기

  if line then
    local parts = {}
    for word in string.gmatch(line, "%S+") do
      table.insert(parts, word)
    end

    if #parts > 0 then
      local command = parts[1]
      local args = {}
      for i = 2, #parts do
        table.insert(args, parts[i])
      end

      if commands[command] then
        commands[command](args)
      else
        print(string.format("알 수 없는 명령: %s", command))
        print("사용 가능한 명령을 보려면 'help'를 입력하세요.")
      end
    end
  else
    -- Ctrl+D (EOF)가 입력된 경우 종료
    print("\nEOF 감지. CLI를 종료합니다.")
    break
  end
end

