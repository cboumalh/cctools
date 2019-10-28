#!/bin/sh

set -e

. ../../dttools/test/test_runner_common.sh
. ./chirp-common.sh

c="./hostport.$PPID"

ticket=my.ticket

python=${CCTOOLS_PYTHON2}

check_needed()
{
	[ -f ../src/bindings/python/_CChirp.so ] || return 1
}

prepare()
{
	[ -n "$python" ] || return 1

	chirp_start local --auth=ticket
	echo "$hostport" > "$c"
	return 0
}

run()
{
	if ! [ -s "$c" ]; then
		return 0
	fi
	hostport=$(cat "$c")

	chirp -d all -a unix "$hostport" ticket_create -output "$ticket" -bits 1024 -duration 86400 -subject unix:`whoami` / write

	PYTHONPATH=../src/bindings/python ${python} ../src/bindings/python/chirp_python_example.py $hostport $ticket

	return 0
}

clean()
{
	chirp_clean
	rm -f "$c" "$ticket"
	return 0
}

dispatch "$@"

# vim: set noexpandtab tabstop=4:
