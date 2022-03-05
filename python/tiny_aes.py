"""
This is a Python equivalent for the https://github.com/kokke/tiny-AES-c AES_CTR.
"""

from typing import List, Union

Nb = 4
Nk = 8
Nr = 14
AES_KEYLEN = 32
AES_keyExpSize = 240
AES_BLOCKLEN = 16

sbox = [
    # 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16]

Rcon = [0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36]

UINT8_T = 256


class AES_ctx:
    def __init__(self, RoundKey: List[int], Iv: List[int]):
        self.RoundKey = RoundKey
        self.Iv = Iv


def KeyExpansion(Key: List[int]) -> List[int]:
    RoundKey = [0] * AES_keyExpSize

    RoundKey[:Nk * 4] = Key[:Nk * 4]

    for i in range(Nk, Nb * (Nr + 1)):
        k = (i - 1) * Nb
        tmp = [RoundKey[k + j] for j in range(Nb)]

        if i % Nk == 0:
            tmp = [sbox[t] for t in tmp[1:] + tmp[:1]]
            tmp[0] = tmp[0] ^ Rcon[i // Nk]
        elif i % Nk == 4:
            tmp = [sbox[t] for t in tmp]

        k = (i - Nk) * Nb
        for j in range(Nb):
            RoundKey[i * Nb + j] = RoundKey[k + j] ^ tmp[j]

    return RoundKey


def AES_init_ctx_iv(key: List[int], iv: List[int]) -> AES_ctx:
    return AES_ctx(KeyExpansion(key), iv.copy())


def AddRoundKey(rnd: int, state: List[int], RoundKey: List[int]):
    for i in range(Nb):
        for j in range(Nb):
            state[i * 4 + j] ^= RoundKey[(rnd * Nb * 4) + (i * Nb) + j]


def SubBytes(state: List[int]):
    for i in range(Nb):
        for j in range(Nb):
            state[i * 4 + j] = sbox[state[i * 4 + j]]


def ShiftRows(state: List[int]) -> List[int]:
    temp = state[0 * 4 + 1]
    state[0 * 4 + 1] = state[1 * 4 + 1]
    state[1 * 4 + 1] = state[2 * 4 + 1]
    state[2 * 4 + 1] = state[3 * 4 + 1]
    state[3 * 4 + 1] = temp

    temp = state[0 * 4 + 2]
    state[0 * 4 + 2] = state[2 * 4 + 2]
    state[2 * 4 + 2] = temp

    temp = state[1 * 4 + 2]
    state[1 * 4 + 2] = state[3 * 4 + 2]
    state[3 * 4 + 2] = temp

    temp = state[0 * 4 + 3]
    state[0 * 4 + 3] = state[3 * 4 + 3]
    state[3 * 4 + 3] = state[2 * 4 + 3]
    state[2 * 4 + 3] = state[1 * 4 + 3]
    state[1 * 4 + 3] = temp
    return state


def xtime(x: int) -> int:
    return ((x << 1) % UINT8_T) ^ (((x >> 7) & 1) * 0x1b)


def MixColumns(state: List[int]):
    for i in range(4):
        t = state[i * 4 + 0]
        Tmp = state[i * 4 + 0] ^ state[i * 4 + 1] ^ state[i * 4 + 2] ^ state[i * 4 + 3]
        Tm = state[i * 4 + 0] ^ state[i * 4 + 1]
        Tm = xtime(Tm)
        state[i * 4 + 0] ^= Tm ^ Tmp
        Tm = state[i * 4 + 1] ^ state[i * 4 + 2]
        Tm = xtime(Tm)
        state[i * 4 + 1] ^= Tm ^ Tmp
        Tm = state[i * 4 + 2] ^ state[i * 4 + 3]
        Tm = xtime(Tm)
        state[i * 4 + 2] ^= Tm ^ Tmp
        Tm = state[i * 4 + 3] ^ t
        Tm = xtime(Tm)
        state[i * 4 + 3] ^= Tm ^ Tmp


def Cipher(state: List[int], RoundKey: List[int]):
    AddRoundKey(0, state, RoundKey)
    rnd = 1
    while True:
        SubBytes(state)
        state = ShiftRows(state)
        if rnd == Nr:
            break
        MixColumns(state)
        AddRoundKey(rnd, state, RoundKey)
        rnd += 1

    AddRoundKey(Nr, state, RoundKey)


def AES_CTR_xcrypt_buffer(ctx: AES_ctx, text: Union[str, bytes], length: int) -> str:
    text = [ord(c) for c in text] if isinstance(text, str) else list(text)

    buffer = [0] * AES_BLOCKLEN
    bi = AES_BLOCKLEN
    for i in range(length):
        if bi == AES_BLOCKLEN:
            buffer = ctx.Iv.copy()

            Cipher(buffer, ctx.RoundKey)

            for bi in range(AES_BLOCKLEN - 1, -1, -1):
                if ctx.Iv[bi] == 255:
                    ctx.Iv[bi] = 0
                    continue

                ctx.Iv[bi] = (ctx.Iv[bi] + 1) % 256
                break

            bi = 0

        text[i] = text[i] ^ buffer[bi]
        bi += 1

    return ''.join(chr(c) for c in text)
