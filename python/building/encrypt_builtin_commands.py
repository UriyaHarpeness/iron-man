import argparse
import json
import random
import subprocess

import pathlib

from iron_man import IronMan
from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


def main():
    parser = argparse.ArgumentParser(description='Post build script for Iron Man.')

    parser.add_argument('--executable', dest='executable', type=pathlib.Path, required=True,
                        help='Path to built Iron Man executable.')
    parser.add_argument('--config', dest='config', type=pathlib.Path, required=True,
                        help='Path to save the configuration of the Iron Man executable.')

    args = parser.parse_args()

    print(f'Encrypting builtin commands for: {args.executable.resolve().absolute()}')
    nm_output = subprocess.check_output(f'nm -S {args.executable}', shell=True).decode('utf-8').splitlines()
    nm_output = {x.split()[-1]: x.split() for x in nm_output}
    commands = {command: (int(nm_output[f'__{command}_start'][0], 16), int(nm_output[f'__{command}_end'][0], 16))
                for command in ['get_file', 'put_file', 'run_shell']}

    with open(args.executable, 'rb') as f:
        elf = bytearray(f.read())

    generated_encryption = {}

    for command, addresses in commands.items():
        key = [random.randint(0, 255) for _ in range(32)]
        iv = [random.randint(0, 255) for _ in range(16)]
        ctx = AES_init_ctx_iv(key, iv)
        elf[addresses[0]:addresses[1]] = IronMan.to_bytes(AES_CTR_xcrypt_buffer(ctx, elf[addresses[0]:addresses[1]],
                                                                                addresses[1] - addresses[0]))
        generated_encryption[command] = [key, iv]

    with args.config.open() as f:
        config = json.load(f)

    config['commands'] = generated_encryption
    with args.config.open('w+') as f:
        json.dump(config, f, indent=2)

    with open(args.executable, 'wb') as f:
        f.write(elf)

    print(f'Saved configurations in: {args.config.resolve().absolute()}')


if __name__ == '__main__':
    main()
