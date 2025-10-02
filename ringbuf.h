#ifndef __RINGBUF_H_
#define __RINGBUF_H_

#include <stdint.h>
#include <stdlib.h>  

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buffer;      // 数据缓冲区指针
    uint16_t size;        // 缓冲区大小(必须是2的幂)
    uint16_t in;        // 写入指针
    uint16_t out;        // 读取指针
#ifdef ringbuf_USE_MUTEX
    mutex_t lock;         // 互斥锁
#endif
} ringbuf_t;

// 函数声明
ringbuf_t *ringbuf_alloc(uint16_t size);
void ringbuf_free(ringbuf_t *rb);
uint16_t ringbuf_push(ringbuf_t *rb, const uint8_t *from, uint16_t len);
uint16_t ringbuf_pop(ringbuf_t *rb, uint8_t *dest, uint16_t len);
uint16_t ringbuf_peek(ringbuf_t *rb, uint16_t start, uint16_t len, uint8_t *dest);
uint16_t ringbuf_remove(ringbuf_t *rb, uint16_t len);
uint16_t ringbuf_checksum(ringbuf_t *rb, uint16_t n, uint16_t m);
uint16_t ringbuf_len(ringbuf_t *rb);

// 宏定义
#define ringbuf_clear(rb) do { (rb)->in = (rb)->out = 0; } while(0)
#define ringbuf_get_read_ptr(rb) ((rb)->buffer + ((rb)->out & ((rb)->size - 1)))
#define ringbuf_get_at(rb, n) ((rb)->buffer[((rb)->out + (n)) & ((rb)->size - 1)])
#define ringbuf_is_empty(rb) ((rb)->in == (rb)->out ? 1 : 0)
#define ringbuf_get_write_ptr(rb) ((rb)->buffer + ((rb)->in & ((rb)->size - 1)))

#ifdef __cplusplus
}
#endif

#endif
