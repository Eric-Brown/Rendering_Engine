#!/bin/bash

SHADERS="./*"

for file in $SHADERS; do
  if [[ $file == *.vert || $file == *.frag ]]; then
    filename="$(basename -- "$file")"
    extension="${file##*.}"
    filename="${filename%.*}"
    out_filename="${filename}_${extension}.spv"
    expr="glslangValidator -V ${file} -o sprvs/${out_filename}"
    echo "Executing: ${expr}"
    eval "$expr"
  fi
done
