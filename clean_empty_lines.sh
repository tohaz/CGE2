#!/bin/bash
# Usage: ./eclipse-strip-lines.sh <file_path>
awk '
{
    # Detect if this line opens or closes a namespace
    if (/^[[:space:]]*namespace([[:space:]]+[_a-zA-Z0-9:]+)?([[:space:]]*{)?/) {
        in_namespace_line = 1
    } else {
        in_namespace_line = 0
    }

    # Count braces on the current line
    open_braces = gsub(/{/, "{")
    close_braces = gsub(/}/, "}")

    # If this line is a namespace declaration line, ignore its opening brace
    if (in_namespace_line && open_braces > 0) {
        open_braces--
    }

    # If a line contains ONLY a closing brace for a namespace, we need to skip it.
    # Standard C++ namespace ends are often just "}" or "} // namespace"
    if (current_depth == 0 && close_braces > 0 && /^[[:space:]]*}[[:space:]]*(\/\/.*)?$/) {
        close_braces--
    }

    # Check if we should skip the empty line
    if (current_depth > 0 && /^[[:space:]]*$/) {
        current_depth += open_braces - close_braces
        next
    }

    # Update the function nesting depth
    current_depth += open_braces - close_braces

    # Ensure depth never drops below zero due to mismatched namespace braces
    if (current_depth < 0) current_depth = 0

    print
}
' "$1" > "$1.tmp" && mv "$1.tmp" "$1"
