import argparse
import pathlib
import re

from building.randoms_generator import generate_from_template
from building.strings_encryption import encrypt_strings


def main():
    # Arguments.
    parser = argparse.ArgumentParser(description='Generate functions loading script for Iron Man.')

    parser.add_argument('--template', dest='template', type=pathlib.Path, required=True,
                        help='Path to the file template.')

    args = parser.parse_args()

    print(f'Generating dynamic functions from template: {args.template}')

    # Read the template file.
    with args.template.open() as f:
        template = f.read()

    # Find the function names to encrypt.
    functions = []
    for function_name in re.findall(r'/\*\* ENCRYPT([\s\S]*?)\*/\s*', template, re.MULTILINE)[0].splitlines()[3:-1]:
        function_name = re.findall(r'\*\s+(\w+)', function_name)[0]
        functions.append(function_name)

    # Generate values for the template and get the filled template and generated values.
    template, generated = generate_from_template(template)

    # Encrypt the function names.
    function_names_string, function_names_length = encrypt_strings(functions, generated['function_names_key'],
                                                                   generated['function_names_iv'])

    # Replace placeholder values with actual values.
    template = re.sub(r'0 ///< Functions number.', str(len(functions)), template)
    template = re.sub(r'0 ///< Function names length.', str(function_names_length), template)
    template = re.sub(r'{}; ///< Functions.', '{' + ', '.join(f'(void **) &{f}_f' for f in functions) + '};', template)
    template = re.sub(r'{}; ///< Function name lengths.', '{' + ', '.join(str(len(f)) for f in functions) + '};',
                      template)
    template = re.sub(r'""; ///< Function names.', f'"{function_names_string}";', template)
    template = re.sub(r'/// Define new function names.',
                      '\n'.join(f'#define {function}_f {function}' for function in functions), template)

    # Write the generated file from the template.
    generated_file = args.template.with_suffix('')
    with generated_file.open('w+') as f:
        f.write(template)

    print(f'Generated file: {generated_file}')


if __name__ == '__main__':
    main()
