#ifndef __CONV_AI_SRC_UTIL_VOLC_RINGBUF_H__
#define __CONV_AI_SRC_UTIL_VOLC_RINGBUF_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void *volc_ringbuf_t;

volc_ringbuf_t volc_ringbuf_create(int size);
int volc_ringbuf_destroy(volc_ringbuf_t handle);
int volc_ringbuf_peek(volc_ringbuf_t handle, char *dst, int readlen);
int volc_ringbuf_clear(volc_ringbuf_t handle);
int volc_ringbuf_getfreesize(volc_ringbuf_t handle);
int volc_ringbuf_getdatasize(volc_ringbuf_t handle);
int volc_ringbuf_write(volc_ringbuf_t handle, char *src, int writelen);
int volc_ringbuf_read(volc_ringbuf_t handle, char *dst, int readlen);

#ifdef __cplusplus
}
#endif
#endif /* __CONV_AI_SRC_UTIL_VOLC_RINGBUF_H__ */
