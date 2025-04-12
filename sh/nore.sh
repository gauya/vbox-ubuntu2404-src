#!/bin/bash

index=0
prg='mpg123'
opt=''
dir='.'
defdir="$HOME/media/sounds"
cur_pfn=0

clean() {
	exit 1
}

trap 'clean' SIGINT

keyin() {
	case "$1" in
		"n"|"N")
			kill "$cur_pfn" 2>/dev/null
			;;
		"s")
			kill -STOP "$cur_pfn" 2>/dev/null;
			read -sn 1 -p"paused... press any key to start";
			echo
			kill -CONT "$cur_pfn" 2>/dev/null
			;;
		"r"|"R")
			kill "$cur_pfn" 2>/dev/null;
			"$2" "$3" 2> /dev/null &
			cur_pfn=$!
			;;
		*) 
			;;
	esac
}

while getopts "zZk:d:" o; do
  case $o in
    z|Z) opt="$opt ""-$o" ;;
    d) dir=$OPTARG;;
    \?) ;;
  esac
done

prg="$prg$opt"

shift $((OPTIND - 1))

if [ "$dir" != "." ]; then cd "$dir"; fi

for fn in *.mp3; do
	echo $fn
	$prg "$fn" 2> /dev/null &
	cur_pfn=$!
	#wait $cur_pfn
	while kill -0 "$cur_pfn" 2>/dev/null; do
		if read -sn 1 -t 0.1 key; then
			keyin "$key" "$prg" "$fn"
		fi
		#sleep 1
	done
done
