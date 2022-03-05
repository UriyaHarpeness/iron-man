import re
from functools import partial
from typing import Any, List

# Bytes constructor.
bytes_ = partial(bytes, encoding='utf-8')

# A mapping of struct format characters to their Python types.
SIGN_TO_BASIC_TYPE = {
    'x': None,
    'b': int, 'B': int, '?': bool, 'h': int, 'H': int, 'i': int, 'I': int, 'l': int, 'L': int, 'q': int, 'Q': int,
    'P': int, 'n': int, 'N': int, 'e': int, 'f': int, 'd': int,
    's': bytes_, 'c': bytes_, 'p': bytes_,
}


def convert_to_types_by_format(fmt: str = '', *args: Any) -> List[Any]:
    """
    Convert arguments to the types specified in the format.

    Args:
        fmt: The struct format of the arguments.
        *args: The arguments to convert types of.

    Returns:
        The arguments after type conversion.
    """
    # Simplify the format.
    fmt = re.sub(r'(\d+)s', 's', fmt)
    fmt = re.sub(r'(\d+)(.)', lambda x: x.group(2) * int(x.group(1)), fmt)

    correct_typed_args = []
    index = 0
    # Iterate the format and convert each argument to its type.
    for sign in fmt:
        # Get the matching type.
        sign_type = SIGN_TO_BASIC_TYPE[sign]
        if sign_type is None:
            continue

        # Convert the type and continue to the next type.
        correct_typed_args.append(sign_type(args[index]))
        index += 1

    return correct_typed_args
