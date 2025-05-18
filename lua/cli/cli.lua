#!/usr/bin/env lua

-- 간단한 CLI 예제

-- 도움말 함수
function print_help()
  print("간단한 CLI 도구")
  print("사용법: cli.lua <명령> [인수...]")
  print("")
  print("사용 가능한 명령:")
  print("  hello [이름]   : 인사말을 출력합니다.")
  print("  add <숫자1> <숫자2> : 두 숫자의 합을 출력합니다.")
  print("  help           : 이 도움말을 출력합니다.")
end

-- 인사말 처리 함수
function handle_hello(args)
  if #args > 0 then
    local name = table.concat(args, " ")
    print(string.format("안녕하세요, %s님!", name))
  else
    print("안녕하세요!")
  end
end

-- 덧셈 처리 함수
function handle_add(args)
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

-- 명령 처리
if #arg == 0 or arg[1] == "help" then
  print_help()
elseif arg[1] == "hello" then
  local hello_args = {}
  for i = 2, #arg do
    table.insert(hello_args, arg[i])
  end
  handle_hello(hello_args)
elseif arg[1] == "add" then
  if #arg == 3 then
    local add_args = {arg[2], arg[3]}
    handle_add(add_args)
  else
    print("오류: 두 개의 숫자를 입력해주세요.")
  end
else
  print(string.format("알 수 없는 명령: %s", arg[1]))
  print("사용 가능한 명령을 보려면 'cli.lua help'를 실행하세요.")
end
