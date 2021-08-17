#pragma once

static const uint8_t KEY[32] = {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
                                0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                                0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
                                0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4};
static const uint8_t IV[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                               0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

static const uint64_t HANDSHAKE = 0x424021fca0537ac5;

static const int PORT = 8080;
