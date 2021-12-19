#!/usr/bin/env bash

sed 's/<script.*<\/script>//g;/<script/,/<\/script>/{/<script/!{/<\/script>/!d}};s/<script.*//g;s/.*<\/script>//g' < $1 | hxnormalize -x 