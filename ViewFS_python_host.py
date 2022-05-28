import pathlib
import platform
from typing import List

if __name__ == '__main__':
    import sample_fs_router

    pipe_path = '\\\\.\\pipe\\aPipe'  # TODO: Generate a better name for this pipe

    if platform.system() == 'Windows':
        with open(pipe_path, 'rb+') as pipe_ref:
            for command_bytes in pipe_ref:
                command_parts = [this_chunk.decode('utf8') for this_chunk in command_bytes.split(b'\x1f')]
                if len(command_parts) == 2 and command_parts[0] == 'list_dir':
                    result_paths: List[pathlib.Path] = sample_fs_router.enum_dir(pathlib.Path(command_parts[1]))
                    result_bytes = b'\x1f'.join(str(this_path.resolve()).encode('utf8') for this_path in result_paths)
                    pipe_ref.write(result_bytes + b'\n')
                    pipe_ref.flush()
