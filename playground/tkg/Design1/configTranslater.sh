#!/bin/bash

cur_dir=$(pwd)

directory_path="${cur_dir}/Config"
escaped_cur_dir=$(echo "$cur_dir" | sed 's/\//\\\//g')
echo "$escaped_cur_dir"

# ディレクトリ内のファイルをイテレート
for file_path in "$directory_path"/*.template; do
    if [ -f "$file_path" ]; then
        new_file="${file_path%.*}.conf"
        cp "$file_path" "$new_file"
        sed "s/pwd/${escaped_cur_dir}/g" "$new_file" > "tmp.txt"
		cat "tmp.txt" > "$new_file"
    fi
done
rm tmp.txt