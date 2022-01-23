import argparse
import json
import random
import subprocess

import pathlib

from iron_man import IronMan
from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


def main():
    parser = argparse.ArgumentParser(description='Encrypt module commands script for Iron Man.')

    parser.add_argument('--module', dest='module', type=pathlib.Path, required=True,
                        help='Path to built module shared object.')
    parser.add_argument('--function', dest='function', type=str, required=True, action='append',
                        help='Name of the function to encrypt.')
    parser.add_argument('--config', dest='config', type=pathlib.Path, required=True,
                        help='Path to save the configuration of the module.')

    args = parser.parse_args()

    print(f'Encrypting module commands: {", ".join(args.function)}, for: {args.module.resolve().absolute()}')
    nm_output = subprocess.check_output(f'nm -S {args.module}', shell=True).decode('utf-8').splitlines()
    nm_output = {x.split()[-1]: x.split() for x in nm_output}
    commands = {
        command: (int(nm_output[command][0], 16), int(nm_output[command][0], 16) + int(nm_output[command][1], 16)) for
        command in args.function}

    with args.module.open('rb') as f:
        elf = bytearray(f.read())

    generated_encryption = {}

    for command, addresses in commands.items():
        key = [random.randint(0, 255) for _ in range(32)]
        iv = [random.randint(0, 255) for _ in range(16)]
        ctx = AES_init_ctx_iv(key, iv)
        elf[addresses[0]:addresses[1]] = IronMan.to_bytes(AES_CTR_xcrypt_buffer(ctx, elf[addresses[0]:addresses[1]],
                                                                                addresses[1] - addresses[0]))
        generated_encryption[command] = {'key': key, 'iv': iv, 'size': addresses[1] - addresses[0]}

    with args.config.open('w+') as f:
        json.dump(generated_encryption, f, indent=2)

    with args.module.open('wb') as f:
        f.write(elf)

    print(f'Saved configurations in: {args.config.resolve().absolute()}')


if __name__ == '__main__':
    main()
