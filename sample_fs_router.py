import pathlib
import os
from os.path import getmtime
import time

src_dir = pathlib.Path("C:\\Users\\saama\\Documents\\VM Shares")


def enum_dir(fs_root: pathlib.Path, dir: pathlib.Path):
    # print(f'{fs_root}, {dir}, {dir == fs_root}', file=sys.stderr)
    if dir == fs_root:
        years = set()
        for this_dir, _, file_list in os.walk(src_dir):
            times = [getmtime(pathlib.Path(this_dir) / this_file) for this_file in file_list]
            years.update(time.localtime(this_time)[0] for this_time in times)
        return [], [str(year) for year in years]
    else:
        folder_year = int(dir.name)
        result_files = []
        for this_dir, _, file_list in os.walk(src_dir):
            full_paths = [pathlib.Path(this_dir) / this_file for this_file in file_list]
            times = [getmtime(this_file) for this_file in full_paths]
            result_files += [this_path for this_path, this_time in zip(full_paths, times) if time.localtime(this_time)[0] == folder_year]
        return result_files, []
