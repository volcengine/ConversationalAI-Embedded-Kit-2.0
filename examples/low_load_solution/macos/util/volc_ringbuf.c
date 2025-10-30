#include "volc_ringbuf.h"

#include <string.h>
#include <stdlib.h>

#define RINGBBUF_FILL_BYTE_SIZE (1)
#define RINGBBUF_ATTR_READ (1 << 0)
#define RINGBBUF_ATTR_SYNC (1 << 1)
#define RINGBBUF_ATTR_WRITE (1 << 2)

typedef int volc_ringbuf_attr_t;

typedef struct {
  int size;
  char *buf;
  int head, tail;
} ringbuf_impl_t;

static ringbuf_impl_t *_create_ring_buf(void)
{
  ringbuf_impl_t *ring_buf = (ringbuf_impl_t *)malloc(sizeof(ringbuf_impl_t));
  ring_buf->head = ring_buf->tail = 0;
  return ring_buf;
}

static int _get_data_size(ringbuf_impl_t *ring_buf)
{
  return (ring_buf->head + ring_buf->size - ring_buf->tail) % ring_buf->size;
}

static int _get_free_size(ringbuf_impl_t *ring_buf)
{
  return ring_buf->size - _get_data_size(ring_buf) - RINGBBUF_FILL_BYTE_SIZE;
}

static void _init_ring_buf(void *ring_buf, int size)
{
  ringbuf_impl_t *buf = (ringbuf_impl_t *)ring_buf;
  buf->size = size + RINGBBUF_FILL_BYTE_SIZE;
  buf->buf = (char *)malloc(size + RINGBBUF_FILL_BYTE_SIZE);
  buf->head = buf->tail = 0;
}

static int _read_attr_setted(volc_ringbuf_attr_t attr)
{
  return ((attr & RINGBBUF_ATTR_READ) == 0 ? 0 : 1);
}

static int _write_attr_setted(volc_ringbuf_attr_t attr)
{
  return ((attr & RINGBBUF_ATTR_WRITE) == 0 ? 0 : 1);
}

static int _sync_attr_setted(volc_ringbuf_attr_t attr)
{
  return ((attr & RINGBBUF_ATTR_SYNC) == 0 ? 0 : 1);
}

static int _read_length_overflow(ringbuf_impl_t *ring_buf, int readlen)
{
  return _get_data_size(ring_buf) < readlen;
}

static int _write_length_overflow(ringbuf_impl_t *ring_buf, int writelen)
{
  return _get_free_size(ring_buf) < writelen;
}

static int _read_internal(volc_ringbuf_t handle, char *buf, int size, volc_ringbuf_attr_t attr)
{
  ringbuf_impl_t *ring_buf = (ringbuf_impl_t *)handle;
  int pos, remain;
  pos = (ring_buf->tail + size) % ring_buf->size;
  if (_read_attr_setted(attr)) {
    if (pos >= ring_buf->tail) {
      memcpy(buf, ring_buf->buf + ring_buf->tail, size);
    } else {
      remain = (ring_buf->size - ring_buf->tail);
      memcpy(buf, ring_buf->buf + ring_buf->tail, remain);
      memcpy(buf + remain, ring_buf->buf, size - remain);
    }
  }

  if (_sync_attr_setted(attr)) {
    ring_buf->tail = pos;
  }

  return size;
}

static int _write_internal(volc_ringbuf_t handle, char *buf, int size, volc_ringbuf_attr_t attr)
{
  ringbuf_impl_t *ring_buf = (ringbuf_impl_t *)handle;
  int pos, remain;
  pos = (ring_buf->head + size) % ring_buf->size;
  if (_write_attr_setted(attr)) {
    if (pos >= ring_buf->head) {
      memcpy(ring_buf->buf + ring_buf->head, buf, size);
    } else {
      remain = (ring_buf->size - ring_buf->head);
      memcpy(ring_buf->buf + ring_buf->head, buf, remain);
      memcpy(ring_buf->buf, buf + remain, size - remain);
    }
  }

  if (_sync_attr_setted(attr)) {
    ring_buf->head = pos;
  }

  return size;
}

volc_ringbuf_t volc_ringbuf_create(int size)
{
  ringbuf_impl_t *ring_buf;
  if (size <= 0) {
    return (volc_ringbuf_t)NULL;
  }

  ring_buf = _create_ring_buf();
  _init_ring_buf(ring_buf, size);
  return (volc_ringbuf_t)ring_buf;
}

int volc_ringbuf_destroy(volc_ringbuf_t handle)
{
  ringbuf_impl_t *ring_buf = (ringbuf_impl_t *)handle;
  free(ring_buf->buf);
  free(ring_buf);
  return 0;
}

static int _ring_buffer_read(char *dst, int readlen, volc_ringbuf_t handle, volc_ringbuf_attr_t attr)
{
  if (_read_length_overflow((ringbuf_impl_t *)handle, readlen)) {
    return -1;
  }

  return _read_internal(handle, dst, readlen, attr);
}

static int _ring_buffer_write(volc_ringbuf_t handle, char *src, int writelen, volc_ringbuf_attr_t attr)
{
  if (_write_length_overflow((ringbuf_impl_t *)handle, writelen)) {
    return -1;
  }

  return _write_internal(handle, src, writelen, attr);
}

int volc_ringbuf_clear(volc_ringbuf_t handle)
{
  ringbuf_impl_t *ring_buf = (ringbuf_impl_t *)handle;
  ring_buf->head = ring_buf->tail = 0;
  return 0;
}

int volc_ringbuf_getfreesize(volc_ringbuf_t handle)
{
  ringbuf_impl_t *ring_buf = (ringbuf_impl_t *)handle;
  return _get_free_size(ring_buf);
}

int volc_ringbuf_getdatasize(volc_ringbuf_t handle)
{
  ringbuf_impl_t *ring_buf = (ringbuf_impl_t *)handle;
  return _get_data_size(ring_buf);
}

int volc_ringbuf_peek(volc_ringbuf_t handle, char *dst, int readlen)
{
  return _ring_buffer_read(dst, readlen, handle, RINGBBUF_ATTR_READ);
}

int volc_ringbuf_write(volc_ringbuf_t handle, char *src, int writelen)
{
  return _ring_buffer_write(handle, src, writelen, RINGBBUF_ATTR_WRITE | RINGBBUF_ATTR_SYNC);
}

int volc_ringbuf_read(volc_ringbuf_t handle, char *dst, int readlen)
{
  return _ring_buffer_read(dst, readlen, handle, RINGBBUF_ATTR_READ | RINGBBUF_ATTR_SYNC);
}
