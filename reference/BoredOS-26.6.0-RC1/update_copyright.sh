#!/bin/bash

# This script replaces the old copyright header with the new one throughout the BoredOS codebase.
# It handles both C-style (//) and Assembly-style (;) comments.

OLD_C="// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)"
NEW_C="// Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)"

OLD_ASM="; Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)"
NEW_ASM="; Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)"

OLD_SH="# Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)"
NEW_SH="# Copyright (c) 2023-2026 Christiaan (chris@boreddev.nl)"

echo "Updating copyright headers in BoredOS..."

# Find all files and perform replacement
# Note: Using sed -i '' for macOS compatibility, and LC_ALL=C to avoid illegal byte sequence errors
find . -type f -not -path '*/.*' -print0 | LC_ALL=C xargs -0 sed -i '' "s|$OLD_C|$NEW_C|g"
find . -type f -not -path '*/.*' -print0 | LC_ALL=C xargs -0 sed -i '' "s|$OLD_ASM|$NEW_ASM|g"
find . -type f -not -path '*/.*' -print0 | LC_ALL=C xargs -0 sed -i '' "s|$OLD_SH|$NEW_SH|g"

echo "Copyright update complete."
