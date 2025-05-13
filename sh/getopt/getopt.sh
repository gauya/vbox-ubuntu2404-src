#!/bin/bash

OPTIONS=$(getopt -o ab:c:: --long a-option,b-option:,c-option:: -- "$@")
eval set -- "$OPTIONS"

# 잘못된 옵션 감지
if [[ $? -ne 0 ]]; then
  echo "Invalid options provided"
  exit 1
fi

while true; do
  case "$1" in
    -a|--a-option)
      echo "Option a is set"
      shift ;;
    -b|--b-option)
      echo "Option b is set with value '$2'"
      shift 2 ;;
    -c|--c-option)
      case "$2" in
        "") echo "Option c is set without value"; shift 2 ;;
        *)  echo "Option c is set with value '$2'"; shift 2 ;;
      esac ;;
    --) shift ; break ;;
    *) echo "Internal error!" ; exit 1 ;;
  esac
done

echo "Remaining arguments: $@"

#-o ab:c::: 짧은 옵션 a, b, c를 정의합니다. b는 인수가 필요하고, c는 선택적 인수를 가집니다.
#--long a-option,b-option:,c-option::: 긴 옵션을 정의합니다.
#-- "$@": 옵션과 일반 인수를 구분합니다.
#eval set -- "$OPTIONS": 파싱된 옵션으로 positional parameter를 설정합니다.
