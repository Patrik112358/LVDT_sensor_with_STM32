find . -type f -iname "*.svg" -exec $SHELL -c '
  echo Processing file: "$0"
  inkscape "$0" --export-area-drawing \
                --export-plain-svg \
                --export-type=svg \
                --export-filename="$0" \
                --export-overwrite
' {} \;

