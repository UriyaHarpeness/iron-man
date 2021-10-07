#pragma once

#include "../consts.h"
#include "../tiny-aes/aes.h"

/**
 * Strings to encrypt:
 *
 * "module_constructor"
 * "run"
 * "module_destructor"
 */

const char *string_module_constructor;
const char *string_run;
const char *string_module_destructor;

// Keys.
static const uint8_t STRINGS_KEY[KEY_LENGTH] = {0xe6, 0x79, 0x89, 0xec, 0x17, 0xb9, 0x46, 0xd2, 0x73, 0xba, 0x10, 0x41, 0x4a, 0xcf, 0xb8, 0xed, 0xa1, 0xe8, 0xa9, 0xda, 0x97, 0x7d, 0x3c, 0x2d, 0x66, 0x2c, 0x2b, 0x72, 0x6b, 0x48, 0x30, 0x46};
static const uint8_t STRINGS_IV[IV_LENGTH] = {0xca, 0xa3, 0x4c, 0x51, 0x9b, 0xe4, 0xac, 0x81, 0xe9, 0xfc, 0x9f, 0xa3, 0xde, 0x1b, 0x45, 0xd3};

#define NUMBER_OF_STRINGS 3

#define ENCRYPTED_STRINGS_LENGTH 41

static const char **strings[NUMBER_OF_STRINGS] = {&string_module_constructor, &string_run, &string_module_destructor};

static unsigned char strings_lengths[NUMBER_OF_STRINGS] = {18, 3, 17};

static char encrypted_strings[ENCRYPTED_STRINGS_LENGTH] = "\xb1\x51\x0e\x65\x01\x86\xfb\xb9\x5f\x05\x2d\x9a\x4d\x9d\xfa\x4f\xae\x80\xb0\xe3\x3b\x17\x39\x6b\xf8\x4e\x24\x4d\x94\x97\x65\x54\xd4\xaf\xc7\x87\x30\x17\x87\xf2\x83";

void decrypt_strings();
