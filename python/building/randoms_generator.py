import random
import re

from functools import partial
from typing import List, Mapping, Tuple

# Random value generators by data type.
TYPE_GENERATORS = {
    'uint8': partial(random.randint, 0, 2 ** 8 - 1),
    'uint64': partial(random.randint, 0, 2 ** 64 - 1)
}

# Regex for a random value definition.
RANDOM_REGEX = re.compile(r'/\*(\w+): (\d+) random (\w+)\*/')


def generate_random_list(data_type: str, number: int) -> List[int]:
    """
    Generate a list of random values for the data type.

    Args:
        data_type: The data type to generate values for.
        number: How many value to generate.

    Returns:
        The generated list of values for the data type.
    """
    return [TYPE_GENERATORS[data_type]() for _ in range(int(number))]


def generate_from_template(template: str) -> Tuple[str, Mapping[str, List[int]]]:
    """
    Generate values for a template file.

    Args:
        template: The template to generate value for.

    Returns:
        The template filled with generated values and the generated values.
    """
    generated = {}

    def generate_randoms(match: re.Match) -> str:
        """
        Generate random values for a matched random value regex.

        Args:
            match: The random value regex match.

        Returns:
            The generated random values to replace the match with.
        """
        name, number, data_type = match.groups()
        generated[name] = generate_random_list(data_type, number)
        return ', '.join([hex(value) for value in generated[name]])

    return RANDOM_REGEX.sub(generate_randoms, template), generated
