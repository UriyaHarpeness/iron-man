#pragma once

#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#ifdef RELEASE_BUILD

int (*accept_f)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int (*bind_f)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void (*bzero_f)(void *s, size_t n);

#define NUMBER_OF_FUNCTIONS 3

static unsigned char function_name_lengths[NUMBER_OF_FUNCTIONS] = {6, 4, 5};

static void **functions[NUMBER_OF_FUNCTIONS] = {
        (void **) &accept_f,
        (void **) &bind_f,
        (void **) &bzero_f,
};

static const char function_names[20] = "accept\0bind\0bzero\0";

void load_all_functions();

#else // RELEASE_BUILD
#define accept_f accept
#define bind_f bind
#define bzero_f bzero
#define load_all_functions() {}
#endif // RELEASE_BUILD
