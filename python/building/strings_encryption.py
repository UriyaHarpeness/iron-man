from typing import List, Tuple

from tiny_aes import AES_CTR_xcrypt_buffer, AES_init_ctx_iv


def encrypt_strings(strings: List[str], key: List[int], iv: List[int]) -> Tuple[str, int]:
    """
    Encrypt a list of strings using a given AES key and IV.

    Args:
        strings: The strings to encrypt.
        key: The AES key to use.
        iv: The AES IV to use.

    Returns:
        The concatenated encrypted strings and the length of the generated string.
    """
    ctx = AES_init_ctx_iv(key, iv)

    joined_strings = ''.join((string + '\0') for string in strings)
    encrypted_strings = ''.join('\\\\x' + hex(ord(byte))[2:].zfill(2) for byte in
                                AES_CTR_xcrypt_buffer(ctx, joined_strings, len(joined_strings)))
    encrypted_strings_length = int(len(encrypted_strings) / 5)

    return encrypted_strings, encrypted_strings_length
