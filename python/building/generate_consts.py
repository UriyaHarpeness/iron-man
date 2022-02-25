import argparse
import json
import pathlib

from building.randoms_generator import generate_from_template


def main():
    # Arguments.
    parser = argparse.ArgumentParser(description='Generate consts script for Iron Man.')

    parser.add_argument('--template', dest='template', type=pathlib.Path, required=True,
                        help='Path to the file template.')
    parser.add_argument('--config', dest='config', type=pathlib.Path, required=True,
                        help='Path to save the values generated for Iron Man.')

    args = parser.parse_args()

    print(f'Generating consts from template: {args.template}')

    # Read the template file.
    with args.template.open() as f:
        template = f.read()

    # Generate values for the template and get the filled template and generated values.
    template, generated = generate_from_template(template)

    # Write the generated file from the template.
    generated_file = args.template.with_suffix('')
    with generated_file.open('w+') as f:
        f.write(template)

    # Write the generated values to the config file.
    with args.config.open('w+') as f:
        json.dump({'generated_consts': generated}, f, indent=2)

    print(f'Generated file: {generated_file}, saved values in {args.config}')


if __name__ == '__main__':
    main()
