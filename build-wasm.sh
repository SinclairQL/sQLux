#!/bin/sh
(emcmake cmake -B wasm . && cd wasm && make)
