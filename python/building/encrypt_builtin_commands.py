import argparse
import json
import pathlib
import subprocess

from building.randoms_generator import generate_random_list
from iron_man import IronMan
from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


def main():
    # Arguments.
    parser = argparse.ArgumentParser(description='Encrypt builtin commands script for Iron Man.')

    parser.add_argument('--executable', dest='executable', type=pathlib.Path, required=True,
                        help='Path to built Iron Man executable.')
    parser.add_argument('--function', dest='function', type=str, required=True, action='append',
                        help='Name of the function to encrypt.')
    parser.add_argument('--config', dest='config', type=pathlib.Path, required=True,
                        help='Path to save the configuration of the Iron Man executable.')

    args = parser.parse_args()

    print(f'Encrypting builtin commands: {", ".join(args.function)}, for: {args.executable}')

    # Find the start and end addresses of the sections containing the specified functions.
    nm_output = subprocess.check_output(f'nm -S {args.executable}', shell=True).decode('utf-8').splitlines()
    nm_output = {line.split()[-1]: line.split() for line in nm_output}
    commands = {command: (int(nm_output[f'__{command}_start'][0], 16), int(nm_output[f'__{command}_end'][0], 16))
                for command in args.function}

    # Read the Iron Man's executable.
    with args.executable.open('rb') as f:
        elf = bytearray(f.read())

    # Encrypt the sections and save the AES key and IV used.
    generated_encryption = {}
    for command, addresses in commands.items():
        key = generate_random_list('uint8', 32)
        iv = generate_random_list('uint8', 16)
        ctx = AES_init_ctx_iv(key, iv)
        elf[addresses[0]:addresses[1]] = IronMan.to_bytes(AES_CTR_xcrypt_buffer(ctx, elf[addresses[0]:addresses[1]],
                                                                                addresses[1] - addresses[0]))
        generated_encryption[command] = [key, iv]

    # Write the modified Iron Man's executable.
    with args.executable.open('wb') as f:
        f.write(elf)

    # Add the AES key and IV of the sections to the Iron Man config file.
    with args.config.open() as f:
        config = json.load(f)
    config['commands'] = generated_encryption
    with args.config.open('w+') as f:
        json.dump(config, f, indent=2)

    print(f'Saved configurations in: {args.config}')


if __name__ == '__main__':
    main()
