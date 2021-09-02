import argparse
import json
import random
import re

import pathlib
from functools import partial

TYPE_GENERATORS = {
    'uint8': partial(random.randint, 0, 2 ** 8 - 1),
    'uint64': partial(random.randint, 0, 2 ** 64 - 1)
}

GENERATED = {}


def generate_randoms(match) -> str:
    name, number, type_ = match.groups()
    generated = [TYPE_GENERATORS[type_]() for _ in range(int(number))]
    GENERATED[name] = generated
    return ', '.join([hex(value) for value in generated])


def main():
    parser = argparse.ArgumentParser(description='Post build script for Iron Man.')

    parser.add_argument('--template', dest='template', type=pathlib.Path, required=True,
                        help='Path to the file template.')
    parser.add_argument('--config', dest='config', type=pathlib.Path, required=True,
                        help='Path to save the values generated for Iron Man.')

    args = parser.parse_args()

    print(f'Generating from template: {args.template.resolve().absolute()}')

    global GENERATED
    GENERATED = {}

    with args.template.open() as f:
        template = f.read()

    generated_file = re.sub(r'/\*(\w+): (\d+) random (\w+)\*/', generate_randoms, template)

    with args.config.open('w+') as f:
        json.dump({'generated_consts': GENERATED}, f, indent=2)

    with args.template.resolve().absolute().with_suffix("").open('w+') as f:
        f.write(generated_file)

    print(f'Generated file: {args.template.resolve().absolute().with_suffix("")}, '
          f'saved values in {args.config.resolve().absolute()}')


if __name__ == '__main__':
    main()
