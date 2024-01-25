/*
 *
 * Copyright (C) 2023-2024 Intel Corporation
 *
 * Under the Apache License v2.0 with LLVM Exceptions. See LICENSE.TXT.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 */

#ifndef UMF_MEMORY_TRACKER_INTERNAL_H
#define UMF_MEMORY_TRACKER_INTERNAL_H 1

#include <assert.h>
#include <stdlib.h>

#include <umf/base.h>
#include <umf/memory_pool.h>
#include <umf/memory_provider.h>

#include "critnib.h"
#include "utils_concurrency.h"

#ifdef __cplusplus
extern "C" {
#endif

extern umf_memory_provider_ops_t UMF_TRACKING_MEMORY_PROVIDER_OPS;

struct umf_memory_tracker_t {
    critnib *map;
    os_mutex_t *splitMergeMutex;
};

typedef struct umf_memory_tracker_t *umf_memory_tracker_handle_t;

static inline umf_memory_tracker_handle_t umfMemoryTrackerCreate(void) {
    umf_memory_tracker_handle_t handle = (umf_memory_tracker_handle_t)malloc(
        sizeof(struct umf_memory_tracker_t));
    if (!handle) {
        return NULL;
    }

    handle->map = critnib_new();
    if (!handle->map) {
        free(handle);
        return NULL;
    }

    handle->splitMergeMutex = util_mutex_create();
    if (!handle->splitMergeMutex) {
        critnib_delete(handle->map);
        free(handle);
        return NULL;
    }

    return handle;
}

static inline void umfMemoryTrackerDestroy(umf_memory_tracker_handle_t handle) {
    if (!handle) {
        return;
    }

    critnib_delete(handle->map);
    util_mutex_destroy(handle->splitMergeMutex);
    free(handle);
}

umf_memory_tracker_handle_t umfMemoryTrackerGet(void);
umf_memory_pool_handle_t
umfMemoryTrackerGetPool(umf_memory_tracker_handle_t hTracker, const void *ptr);

// Creates a memory provider that tracks each allocation/deallocation through umf_memory_tracker_handle_t and
// forwards all requests to hUpstream memory Provider. hUpstream lifetime should be managed by the user of this function.
umf_result_t umfTrackingMemoryProviderCreate(
    umf_memory_provider_handle_t hUpstream, umf_memory_pool_handle_t hPool,
    umf_memory_provider_handle_t *hTrackingProvider);

// Initialize critnib for a UMF static library build on Windows
void umfTrackingMemoryProviderInit(void);

void umfTrackingMemoryProviderFini(void *tracker);

void umfTrackingMemoryProviderGetUpstreamProvider(
    umf_memory_provider_handle_t hTrackingProvider,
    umf_memory_provider_handle_t *hUpstream);

#ifdef __cplusplus
}
#endif

#endif /* UMF_MEMORY_TRACKER_INTERNAL_H */
