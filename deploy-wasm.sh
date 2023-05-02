#! /bin/sh

HOST="$1"

scp wasm/index.html $HOST/index.html
scp wasm/sqlux.* $HOST/
