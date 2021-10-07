#include "strings.h"

void decrypt_strings() {
    struct AES_ctx strings_ctx;
    AES_init_ctx_iv(&strings_ctx, STRINGS_KEY, STRINGS_IV);
    AES_CTR_xcrypt_buffer(&strings_ctx, (uint8_t *) encrypted_strings, ENCRYPTED_STRINGS_LENGTH);
    const char *strings_pointer = encrypted_strings;
    for (size_t index = 0; index < NUMBER_OF_STRINGS; index++) {
        *strings[index] = strings_pointer;
        strings_pointer += strings_lengths[index] + 1;
    }
}
