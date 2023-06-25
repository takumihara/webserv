#!/bin/bash

cur_dir=$(pwd)

directory_path="${cur_dir}/Config"
escaped_cur_dir=$(echo "$cur_dir" | sed 's/\//\\\//g')
echo "$escaped_cur_dir"

# ディレクトリ内のファイルをイテレート
for file_path in "$directory_path"/*; do
    if [ -f "$file_path" ]; then
        sed "s/pwd/${escaped_cur_dir}/g" "$file_path" > "tmp.txt"
		cat "tmp.txt" > "$file_path"
    fi
done
rm tmp.txt