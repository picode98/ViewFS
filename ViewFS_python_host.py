import argparse
import pathlib
import platform
import sys
from os import chdir
import importlib
from typing import List

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Host for ViewFS Python router scripts')
    parser.add_argument('--fs-root', type=str, required=True, help='Path to the root of the virtual filesystem')
    parser.add_argument('--command-pipe', type=str, required=True, help='Path to a pipe from which script commands can'
                                                                        'be read and results can be written')
    parser.add_argument('--router-file', type=str, required=True, help='Router Python module to be loaded')
    args = parser.parse_args()

    fs_root = pathlib.Path(args.fs_root).absolute().resolve()
    module_path = pathlib.Path(args.router_file).absolute().resolve()
    chdir(module_path.parent)
    sys.path.append(str(pathlib.Path(__file__).parent))
    router = importlib.import_module(module_path.name.split('.', maxsplit=1)[0])

    if platform.system() == 'Windows':
        with open(args.command_pipe, 'rb+') as pipe_ref:
            for command_bytes in pipe_ref:
                command_parts = [this_chunk.decode('utf8') for this_chunk in command_bytes[:-1].split(b'\x1f')]
                if len(command_parts) == 2 and command_parts[0] == 'list_dir':
                    ref_paths: List[pathlib.Path]
                    subdirs: List[str]
                    ref_paths, subdirs = router.enum_dir(fs_root, pathlib.Path(command_parts[1]))
                    result_bytes = b'\x1f'.join(str(this_path.absolute().resolve()).encode('utf8') for this_path in ref_paths) + b'\n' \
                        + b'\x1f'.join(subdir.encode('utf8') for subdir in subdirs) + b'\n'
                    pipe_ref.write(result_bytes)
                    pipe_ref.flush()
