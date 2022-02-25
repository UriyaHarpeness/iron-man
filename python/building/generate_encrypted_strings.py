import argparse
import pathlib
import re

from building.randoms_generator import generate_from_template
from building.strings_encryption import encrypt_strings


def main():
    # Arguments.
    parser = argparse.ArgumentParser(description='Generate encrypted strings script for Iron Man.')

    parser.add_argument('--template', dest='template', type=pathlib.Path, required=True,
                        help='Path to the file template.')

    args = parser.parse_args()

    print(f'Generating encrypted strings from template: {args.template}')

    # Read the template file.
    with args.template.open() as f:
        template = f.read()

    # Find the string to encrypt.
    strings = []
    for string in re.findall(r'/\*\* ENCRYPT([\s\S]*?)\*/\s*', template, re.MULTILINE)[0].splitlines()[3:-1]:
        string = re.match('\s+\*\s+"(.*?)"', string).group(1)
        strings.append(string)

    # Generate values for the template and get the filled template and generated values.
    template, generated = generate_from_template(template)

    # Encrypt the strings.
    encrypted_strings, encrypted_strings_length = encrypt_strings(strings, generated['strings_key'],
                                                                  generated['strings_iv'])

    # Replace placeholder values with actual values.
    variable_name_strings = [re.sub(r"\W", "_", string) for string in strings]
    template = re.sub(r'/// Individual strings.',
                      '\n'.join(f'const char *string_{string};' for string in variable_name_strings), template)
    template = re.sub(r'0 ///< Strings number.', str(len(strings)), template)
    template = re.sub(r'0 ///< Encrypted strings length.', str(encrypted_strings_length), template)
    template = re.sub(r'{}; ///< Strings lengths.', '{' + ', '.join(str(len(string)) for string in strings) + '};',
                      template)
    template = re.sub(r'{}; ///< Strings.',
                      '{' + ', '.join(f'&string_{string}' for string in variable_name_strings) + '};',
                      template)
    template = re.sub(r'""; ///< Encrypted strings.', f'"{encrypted_strings}";', template)

    # Write the generated file from the template.
    generated_file = args.template.with_suffix('')
    with generated_file.open('w+') as f:
        f.write(template)

    print(f'Generated file: {generated_file}')


if __name__ == '__main__':
    main()
