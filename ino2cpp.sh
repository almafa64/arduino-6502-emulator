#!/bin/bash
path="$(/bin/pwd)/*.ino"
ino=( $path )
mkdir -p "$(dirname $ino)/src" 
cpp="$(dirname $ino)/src/$(basename "$ino" .ino).cpp"
cp $ino $cpp
sed -i '1s/^/#include <Arduino.h>\n /' $cpp
