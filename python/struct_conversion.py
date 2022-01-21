import re
from functools import partial
from typing import Any, List

SIGN_TO_BASIC_TYPE = {
    'x': None,
    'c': partial(bytes, encoding='utf-8'),
    'b': int,
    'B': int,
    '?': bool,
    'h': int,
    'H': int,
    'i': int,
    'I': int,
    'l': int,
    'L': int,
    'q': int,
    'Q': int,
    'n': int,
    'N': int,
    'e': int,
    'f': int,
    'd': int,
    's': partial(bytes, encoding='utf-8'),
    'p': partial(bytes, encoding='utf-8'),
    'P': int
}


def convert_to_types_by_format(fmt: str = '', *args: Any) -> List[Any]:
    fmt = re.sub(r'(\d+)s', 's', fmt)
    fmt = re.sub(r'(\d+)(.)', lambda x: x.group(2) * int(x.group(1)), fmt)

    correct_typed_args = []
    index = 0
    for sign in fmt:
        sign_type = SIGN_TO_BASIC_TYPE[sign]
        if sign_type is None:
            continue

        correct_typed_args.append(sign_type(args[index]))
        index += 1

    return correct_typed_args
