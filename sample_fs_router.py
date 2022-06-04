import pathlib
import sys


def enum_dir(fs_root: pathlib.Path, dir: pathlib.Path):
    print(f'{fs_root}, {dir}, {dir == fs_root}', file=sys.stderr)
    return [pathlib.Path("C:\\Users\\saama\\Applications\\mipsit.zip")], ['a_folder'] if dir == fs_root else []
