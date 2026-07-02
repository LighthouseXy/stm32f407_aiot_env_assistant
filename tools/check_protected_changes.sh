#!/usr/bin/env bash
set -euo pipefail

# Files and directories that should stay under CubeMX/tool ownership.
# If these change, review carefully before keeping the change.
protected_patterns=(
  "stm32f407_aiot_env_assistant.ioc"
  "Core/Src/stm32f4xx_hal_msp.c"
  "Core/Src/system_stm32f4xx.c"
  "Core/Inc/stm32f4xx_hal_conf.h"
  "startup_stm32f407xx.s"
  "STM32F407XX_FLASH.ld"
  "Drivers/"
  "Middlewares/"
  "cmake/stm32cubemx/"
)

# These CubeMX files are allowed to change only inside USER CODE blocks.
user_code_guard_files=(
  "Core/Src/main.c"
  "Core/Inc/main.h"
  "Core/Src/stm32f4xx_it.c"
  "Core/Inc/stm32f4xx_it.h"
)

changed_files="$(git diff --name-only HEAD)"

if [[ -z "${changed_files}" ]]; then
  echo "OK: no working-tree changes."
  exit 0
fi

violations=()

while IFS= read -r file; do
  for pattern in "${protected_patterns[@]}"; do
    if [[ "${pattern}" == */ ]]; then
      if [[ "${file}" == "${pattern}"* ]]; then
        violations+=("${file}")
      fi
    elif [[ "${file}" == "${pattern}" ]]; then
      violations+=("${file}")
    fi
  done

done <<< "${changed_files}"

user_code_violations="$(python3 - "$PWD" "${user_code_guard_files[@]}" <<'PY_USER_CHECK'
import re
import subprocess
import sys
from pathlib import Path

repo = Path(sys.argv[1])
guarded = sys.argv[2:]
changed = set(subprocess.check_output(
    ["git", "diff", "--name-only", "HEAD"], cwd=repo, text=True
).splitlines())

def user_ranges(text: str):
    ranges = []
    start = None
    for lineno, line in enumerate(text.splitlines(), 1):
        if "USER CODE BEGIN" in line:
            start = lineno
        if "USER CODE END" in line and start is not None:
            ranges.append((start, lineno))
            start = None
    return ranges

def in_ranges(lineno: int, ranges):
    return any(start <= lineno <= end for start, end in ranges)

def changed_lines(diff_text: str):
    old_line = None
    new_line = None
    old_changed = []
    new_changed = []
    hunk_re = re.compile(r"@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")
    for line in diff_text.splitlines():
        match = hunk_re.match(line)
        if match:
            old_line = int(match.group(1))
            new_line = int(match.group(3))
            continue
        if old_line is None or new_line is None:
            continue
        if line.startswith("---") or line.startswith("+++"):
            continue
        if line.startswith("+"):
            new_changed.append(new_line)
            new_line += 1
        elif line.startswith("-"):
            old_changed.append(old_line)
            old_line += 1
        else:
            old_line += 1
            new_line += 1
    return old_changed, new_changed

violations = []
for file in guarded:
    if file not in changed:
        continue

    current_path = repo / file
    current_text = current_path.read_text(errors="ignore") if current_path.exists() else ""
    current_ranges = user_ranges(current_text)

    try:
        head_text = subprocess.check_output(
            ["git", "show", f"HEAD:{file}"], cwd=repo, text=True, stderr=subprocess.DEVNULL
        )
    except subprocess.CalledProcessError:
        head_text = ""
    head_ranges = user_ranges(head_text)

    diff_text = subprocess.check_output(
        ["git", "diff", "--unified=0", "HEAD", "--", file], cwd=repo, text=True
    )
    old_changed, new_changed = changed_lines(diff_text)

    old_bad = any(not in_ranges(line, head_ranges) for line in old_changed)
    new_bad = any(not in_ranges(line, current_ranges) for line in new_changed)
    if old_bad or new_bad:
        violations.append(file)

print("\n".join(violations))
PY_USER_CHECK
)"

if (( ${#violations[@]} > 0 )); then
  echo "WARNING: protected CubeMX/generated files changed:"
  printf '  %s\n' "${violations[@]}" | sort -u
  echo
  echo "Review these changes before keeping them. Peripheral and clock changes should usually be made in STM32CubeMX."
  exit 1
fi

if [[ -n "${user_code_violations}" ]]; then
  echo "WARNING: CubeMX files changed outside USER CODE blocks:"
  printf '%s\n' "${user_code_violations}" | sort -u | sed 's/^/  /'
  echo
  echo "Keep application edits inside USER CODE blocks, or move code into App/."
  exit 1
fi

echo "OK: no protected CubeMX/generated files changed."
