#!/usr/bin/env python3

from __future__ import annotations

import argparse
from datetime import datetime
from pathlib import Path

LANGUAGE_BY_EXTENSION = {
    ".c": "c", ".cc": "cpp", ".cpp": "cpp", ".cxx": "cpp",
    ".h": "cpp", ".hh": "cpp", ".hpp": "cpp", ".hxx": "cpp",
    ".ipp": "cpp", ".tpp": "cpp", ".cmake": "cmake", ".sh": "bash", ".bash": "bash",
    ".json": "json", ".yaml": "yaml", ".yml": "yaml",
    ".toml": "toml", ".md": "markdown", ".txt": "text",
}

SPECIAL_FILENAMES = {
    "CMakeLists.txt": "cmake",
    "CMakePresets.json": "json",
    "CTestConfig.cmake": "cmake",
    "Dockerfile": "dockerfile",
    "Makefile": "makefile",
    ".clang-format": "yaml",
    ".clang-tidy": "yaml",
    ".gitignore": "gitignore",
}

IGNORE_DIRS = {
    ".git", ".hg", ".svn", ".idea", ".vscode", ".venv", "venv", "env",
    "node_modules", "build", "build-debug", "build-release",
    "cmake-build-debug", "cmake-build-release", "dist", "out", "bin", "obj",
    "coverage", "__pycache__", ".pytest_cache", ".mypy_cache", ".ruff_cache", ".cache",
}

DEFAULT_OUTPUT = "codebase_snapshot.md"


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Export a source-code repository into one Markdown snapshot."
    )
    parser.add_argument(
        "-o", "--output", default=DEFAULT_OUTPUT,
        help=f"Output Markdown file. Default: {DEFAULT_OUTPUT}",
    )
    parser.add_argument(
        "--max-file-size", type=int, default=1_000_000,
        help="Maximum included file size in bytes. Default: 1,000,000",
    )
    return parser.parse_args()


def should_ignore(path: Path, root: Path) -> bool:
    relative_path = path.relative_to(root)
    return any(part in IGNORE_DIRS for part in relative_path.parts)


def language_for(path: Path) -> str | None:
    if path.name in SPECIAL_FILENAMES:
        return SPECIAL_FILENAMES[path.name]
    return LANGUAGE_BY_EXTENSION.get(path.suffix.lower())


def markdown_fence(content: str) -> str:
    longest_sequence = 0
    current_sequence = 0
    for character in content:
        if character == "`":
            current_sequence += 1
            longest_sequence = max(longest_sequence, current_sequence)
        else:
            current_sequence = 0
    return "`" * max(3, longest_sequence + 1)


def collect_files(
    root: Path,
    script_path: Path,
    output_path: Path,
    max_file_size: int,
) -> tuple[list[Path], list[tuple[Path, str]]]:
    included: list[Path] = []
    skipped: list[tuple[Path, str]] = []

    for path in root.rglob("*"):
        if not path.is_file() or should_ignore(path, root):
            continue

        resolved_path = path.resolve()
        if resolved_path in {script_path, output_path}:
            continue
        if path.name.endswith("_snapshot.md"):
            continue
        if language_for(path) is None:
            continue

        try:
            size = path.stat().st_size
        except OSError as error:
            skipped.append((path, f"could not inspect file: {error}"))
            continue

        if size > max_file_size:
            skipped.append((path, f"file is larger than {max_file_size} bytes"))
            continue

        included.append(path)

    included.sort(key=lambda item: item.relative_to(root).as_posix().lower())
    skipped.sort(key=lambda item: item[0].relative_to(root).as_posix().lower())
    return included, skipped


def build_project_tree(files: list[Path], root: Path) -> str:
    lines = [root.name + "/"]
    for path in files:
        relative_path = path.relative_to(root)
        depth = len(relative_path.parts) - 1
        indentation = "    " * depth
        lines.append(f"{indentation}└── {relative_path.name}")
    return "\n".join(lines)


def write_snapshot(
    root: Path,
    output_path: Path,
    source_files: list[Path],
    skipped_files: list[tuple[Path, str]],
) -> None:
    with output_path.open("w", encoding="utf-8", newline="\n") as output:
        output.write("# Codebase Snapshot\n\n")
        output.write(f"- Generated: `{datetime.now().isoformat(timespec='seconds')}`\n")
        output.write(f"- Project root: `{root}`\n")
        output.write(f"- Files included: `{len(source_files)}`\n")
        output.write(f"- Files skipped: `{len(skipped_files)}`\n\n")

        output.write("## Project Tree\n\n```text\n")
        output.write(build_project_tree(source_files, root))
        output.write("\n```\n\n## Source Files\n\n")

        for file_path in source_files:
            relative_path = file_path.relative_to(root)
            language = language_for(file_path) or "text"
            output.write(f"### `{relative_path.as_posix()}`\n\n")

            try:
                content = file_path.read_text(encoding="utf-8", errors="replace")
            except OSError as error:
                output.write(f"Could not read file: `{error}`\n\n")
                continue

            fence = markdown_fence(content)
            output.write(f"{fence}{language}\n")
            output.write(content)
            if content and not content.endswith("\n"):
                output.write("\n")
            output.write(f"{fence}\n\n")

        if skipped_files:
            output.write("## Skipped Files\n\n")
            for file_path, reason in skipped_files:
                relative_path = file_path.relative_to(root)
                output.write(f"- `{relative_path.as_posix()}`: {reason}\n")


def main() -> None:
    arguments = parse_arguments()
    root = Path.cwd().resolve()
    script_path = Path(__file__).resolve()
    output_path = (root / arguments.output).resolve()

    source_files, skipped_files = collect_files(
        root, script_path, output_path, arguments.max_file_size
    )
    write_snapshot(root, output_path, source_files, skipped_files)

    print(f"Done. Wrote {len(source_files)} files to {output_path.relative_to(root)}")
    if skipped_files:
        print(f"Skipped {len(skipped_files)} files. See the snapshot for details.")


if __name__ == "__main__":
    main()
