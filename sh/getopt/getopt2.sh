#!/bin/bash

# 처리할 짧은 옵션 및 긴 옵션 정의:
# -a, --all: 인수 없음
# -n, --name <value>: 인수 필요
# -v, --verbose: 인수 없음
SHORT_OPTIONS="an:v"
LONG_OPTIONS="all,name:,verbose"

# getopt 실행 및 결과 파싱
PARSED=$(getopt -o "$SHORT_OPTIONS" --long "$LONG_OPTIONS" -- "$@")
if [ $? -ne 0 ]; then
    echo "명령행 옵션 파싱 오류 발생" >&2
    exit 1
fi

eval set -- "$PARSED"

# 파싱된 옵션 처리
while true; do
    case "$1" in
        -a|--all) echo "옵션 --all (-a) 발견" ; shift ;;
        -n|--name) echo "옵션 --name (-n) 발견, 값: $2" ; shift 2 ;;
        -v|--verbose) echo "옵션 --verbose (-v) 발견" ; shift ;;
        --) shift ; break ;;
        *) echo "알 수 없는 옵션: $1" >&2 ; exit 1 ;;
    esac
done

# 나머지 인수 처리
echo "나머지 인수:"
for arg in "$@"; do
    echo "$arg"
done
