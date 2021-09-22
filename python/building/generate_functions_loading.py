import argparse
import json
import random
import re

import pathlib
from functools import partial

from building import generate_consts
from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


def main():
    parser = argparse.ArgumentParser(description='Post build script for Iron Man.')

    parser.add_argument('--template', dest='template', type=pathlib.Path, required=True,
                        help='Path to the file template.')
    args = parser.parse_args()

    print(f'Generating dynamic functions from template: {args.template.resolve().absolute()}')

    with args.template.open() as f:
        template = f.read()

    functions = []
    for function_name in re.findall(r'/\*([\s\S]*?)\*/\s*', template, re.MULTILINE)[0].splitlines()[3:-1]:
        function_name = re.findall(r'\*\s+(\w+)', function_name)[0]
        functions.append(function_name)

    function_names_string = ''.join((f + '\0') for f in functions)

    template = generate_consts.generate_from_template(template)

    ctx = AES_init_ctx_iv(generate_consts.GENERATED['function_names_key'],
                          generate_consts.GENERATED['function_names_iv'])

    libc_name = re.findall(r'{}; // Libc library name - "(.*?)".', template)[0]
    libc_name_string = ''.join('\\\\x' + hex(ord(f))[2:].zfill(2) for f in
                               AES_CTR_xcrypt_buffer(ctx, libc_name, len(libc_name)))

    template = re.sub(r'0 // Libc library name length.', str(len(libc_name)), template)
    template = re.sub(r'{}; // Libc library name .*.', '"' + libc_name_string + '";', template)

    function_names_string = ''.join('\\\\x' + hex(ord(f))[2:].zfill(2) for f in
                                    AES_CTR_xcrypt_buffer(ctx, function_names_string, len(function_names_string)))
    function_names_length = int(len(function_names_string) / 5)

    template = re.sub(r'0 // Functions number.', str(len(functions)), template)
    template = re.sub(r'0 // Function names length.', str(function_names_length), template)
    template = re.sub(r'{}; // Functions.', '{' + ', '.join(f'(void **) &{f}_f' for f in functions) + '};', template)
    template = re.sub(r'{}; // Function name lengths.', '{' + ', '.join(str(len(f)) for f in functions) + '};',
                      template)
    template = re.sub(r'""; // Function names.', f'"' + function_names_string + '";',
                      template)
    template = re.sub(r'// Define new function names.',
                      '\n'.join(f'#define {function}_f {function}' for function in functions), template)

    with args.template.resolve().absolute().with_suffix("").open('w+') as f:
        f.write(template)

    print(f'Generated file: {args.template.resolve().absolute().with_suffix("")}')


if __name__ == '__main__':
    main()
