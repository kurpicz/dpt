#!/bin/bash
# This script decompresses the first $2 common crawl archives from $1.
_file="${1:-/dev/null}"
_amount="${2:-/dev/null}"
while IFS= read -r line
do
  gzip -d "${line##*/}"
done < <(sed -n '1,'"$_amount"'p' < "$_file")
