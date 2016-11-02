#!/bin/bash
# This scripy download the first $2 common crawl corpus archives from $1.
_file="${1:-/dev/null}"
_amount="${2:-/dev/null}"
_url=https://commoncrawl.s3.amazonaws.com/
while IFS= read -r line
do
  wget "${_url}""$line"
done < <(sed -n '1,'"$_amount"'p' < "$_file")