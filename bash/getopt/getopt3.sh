#!/bin/bash

# 정의: -f, -v는 짧은 옵션 / --file, --verbose는 긴 옵션
OPTIONS=f:v
LONGOPTIONS=file:,verbose

# getopt 호출 → $@는 모든 인자
PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTIONS --name "$0" -- "$@")

# getopt 오류 발생 시 종료
if [[ $? -ne 0 ]]; then
  exit 1
fi

# 옵션 파싱 결과 eval로 적용
eval set -- "$PARSED"

# 초기값
FILE=""
VERBOSE=false

# 옵션 루프
while true; do
  case "$1" in
    -f|--file)
      FILE="$2"
      shift 2
      ;;
    -v|--verbose)
      VERBOSE=true
      shift
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "알 수 없는 옵션: $1" >&2
      exit 1
      ;;
  esac
done

# 확인 출력
echo "FILE=$FILE"
echo "VERBOSE=$VERBOSE"
echo "남은 인자: $@"


# ./getopt3.sh -f hello.txt --verbose foo bar
