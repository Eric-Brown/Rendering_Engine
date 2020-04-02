#!/bin/bash

HEADERS="../../vulk_src/*"

for file in $HEADERS; do
  if [[ $file == *.h || $file == *.hpp ]]; then
    filename="$(basename -- "$file")"
    extension="${file##*.}"
    filename="${filename%.*}"
    out_filename="${filename}.puml"
    expr="hpp2plantuml -i ${file} -o ${out_filename} -t template.puml"
    echo "Executing: ${expr}"
    eval "$expr"
  fi
done