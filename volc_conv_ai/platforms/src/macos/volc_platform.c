// Copyright (2025) Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

#include "volc_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if_dl.h>

void* hal_malloc(size_t size) {
    return malloc(size);
}

void* hal_calloc(size_t num, size_t size) {
    return calloc(num, size);
}

void* hal_realloc(void* ptr, size_t new_size) {
    return realloc(ptr, new_size);
}

void hal_free(void* ptr) {
    free(ptr);
}

hal_mutex_t hal_mutex_create(void) {
    pthread_mutex_t* p_mutex = NULL;
    pthread_mutexattr_t attr;

    p_mutex = (pthread_mutex_t *)hal_calloc(1, sizeof(pthread_mutex_t));
    if (NULL == p_mutex) {
        return NULL;
    }

    if (0 != pthread_mutexattr_init(&attr) ||
        0 != pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL) ||
        0 != pthread_mutex_init(p_mutex, &attr)) {
        hal_free(p_mutex);
        return NULL;
    }
    
    return (hal_mutex_t)p_mutex;
}
void hal_mutex_lock(hal_mutex_t mutex) {
    pthread_mutex_lock((pthread_mutex_t *)mutex);
}

void hal_mutex_unlock(hal_mutex_t mutex) {
    pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

void hal_mutex_destroy(hal_mutex_t mutex) {
    pthread_mutex_t* p_mutex = (pthread_mutex_t *)mutex;
    if (NULL == p_mutex) {
        return;
    }
    pthread_mutex_destroy(p_mutex);
    hal_free(p_mutex);
}

uint64_t hal_get_time_ms(void) {
    struct timespec now_time;
    clock_gettime(CLOCK_REALTIME, &now_time);
    return (uint64_t)now_time.tv_sec * 1000 + (uint64_t)now_time.tv_nsec / 1000000;
}

int hal_get_uuid(char* uuid, size_t size) {
    int ret = 0;
    struct ifaddrs* ifa = NULL;
    struct ifaddrs* ifa_temp = NULL;

    getifaddrs(&ifa);
    for (ifa_temp = ifa; ifa_temp != NULL; ifa_temp = ifa_temp->ifa_next) {
        if (ifa_temp->ifa_addr != NULL && ifa_temp->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa_temp->ifa_addr;
            if (sdl->sdl_alen == 6) {
                unsigned char* mac = (unsigned char*)LLADDR(sdl);
                if (strncmp(ifa_temp->ifa_name, "lo", 2) != 0) {
                    snprintf(uuid, size, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    break;
                }
            }
        }
    }
    freeifaddrs(ifa);
err_out_label:  
    return ret;
}
int hal_thread_create(hal_tid_t* thread, const hal_thread_param_t* param, void* (*start_routine)(void *), void* args) {
    int ret = 0;
    pthread_t pthread;
    if (NULL == thread || NULL == start_routine) {
        return -1;
    }
    ret = pthread_create(&pthread, NULL, start_routine, args);
    if (0 != ret) {
        goto err_out_label;
    }
    *thread = (hal_tid_t)pthread;
    return 0;
err_out_label:
    return -1;
}
int hal_thread_detach(hal_tid_t thread) {
    return pthread_detach((pthread_t)thread);
}

void hal_thread_exit(hal_tid_t thread) {
    pthread_exit((pthread_t)thread);
}

void hal_thread_sleep(int time_ms) {
    usleep(time_ms * 1000);
}

void hal_thread_destroy(hal_tid_t thread) {
    (void)thread;
}

int hal_get_platform_info(char* info, size_t size) {
    if (NULL == info || size <= 0) {
        return -1;
    }
    snprintf(info, size, "macos");
    return 0;
}

int hal_fill_random(uint8_t* data, size_t size) {
    if (NULL == data || size <= 0) {
        return -1;
    }
    while (size > 0) {
        uint32_t word = (uint32_t) hal_get_time_ms();
        uint32_t to_copy = sizeof(word) < size ? sizeof(word) : size;
        memcpy(data, &word, to_copy);
        data += to_copy;
        size -= to_copy;
    }
    return 0;
}
