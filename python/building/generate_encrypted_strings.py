import argparse
import re

import pathlib

from building import generate_consts
from tiny_aes import AES_init_ctx_iv, AES_CTR_xcrypt_buffer


def main():
    parser = argparse.ArgumentParser(description='Post build script for Iron Man.')

    parser.add_argument('--template', dest='template', type=pathlib.Path, required=True,
                        help='Path to the file template.')
    args = parser.parse_args()

    print(f'Generating encrypted strings from template: {args.template.resolve().absolute()}')

    with args.template.open() as f:
        template = f.read()

    strings = []
    for string in re.findall(r'/\*([\s\S]*?)\*/\s*', template, re.MULTILINE)[0].splitlines()[3:-1]:
        string = re.findall(r'\*\s+"(.*?)"', string)[0]
        strings.append(string)

    joined_strings = ''.join((f + '\0') for f in strings)

    template = generate_consts.generate_from_template(template)

    ctx = AES_init_ctx_iv(generate_consts.GENERATED['strings_key'],
                          generate_consts.GENERATED['strings_iv'])

    encrypted_strings = ''.join('\\\\x' + hex(ord(f))[2:].zfill(2) for f in
                                AES_CTR_xcrypt_buffer(ctx, joined_strings, len(joined_strings)))
    encrypted_strings_length = int(len(encrypted_strings) / 5)

    variable_name_strings = [re.sub(r"\W", "_", string) for string in strings]
    template = re.sub(r'// Individual strings.',
                      '\n'.join(f'const char *string_{string};' for string in variable_name_strings), template)
    template = re.sub(r'0 // Strings number.', str(len(strings)), template)
    template = re.sub(r'0 // Encrypted strings length.', str(encrypted_strings_length), template)
    template = re.sub(r'{}; // Strings lengths.', '{' + ', '.join(str(len(s)) for s in strings) + '};', template)
    template = re.sub(r'{}; // Strings.', '{' + ', '.join(f'&string_{s}' for s in variable_name_strings) + '};',
                      template)
    template = re.sub(r'""; // Encrypted strings.', f'"' + encrypted_strings + '";', template)

    with args.template.resolve().absolute().with_suffix("").open('w+') as f:
        f.write(template)

    print(f'Generated file: {args.template.resolve().absolute().with_suffix("")}')


if __name__ == '__main__':
    main()
