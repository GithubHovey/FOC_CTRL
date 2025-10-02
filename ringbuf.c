#include "ringbuf.h"
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
// 辅助函数：检查是否为2的幂
static uint8_t is_power_of_two(uint16_t size)
{
    return (size != 0) && ((size & (size - 1)) == 0);
}

ringbuf_t *ringbuf_alloc(uint16_t size)
{
    // 检查size是否为2的幂且不为0
    if (size == 0 || !is_power_of_two(size)) {
        return NULL;
    }

    ringbuf_t *rb = (ringbuf_t *)malloc(sizeof(ringbuf_t));
    if (!rb) {
        return NULL;
    }

    rb->buffer = (uint8_t *)malloc(size);
    if (!rb->buffer) {
        free(rb);
        return NULL;
    }

    rb->size = size;
    rb->in = 0;
    rb->out = 0;

#ifdef ringbuf_USE_MUTEX
    if (mutex_init(&rb->lock) != 0) {
        free(rb->buffer);
        free(rb);
        return NULL;
    }
#endif

    return rb;
}

void ringbuf_free(ringbuf_t *rb)
{
    if (rb) {
        if (rb->buffer) {
            free(rb->buffer);
        }
        
#ifdef ringbuf_USE_MUTEX
        mutex_destroy(&rb->lock);
#endif
        
        free(rb);
    }
}

uint16_t ringbuf_push(ringbuf_t *rb, const uint8_t *from, uint16_t len)
{
#ifdef ringbuf_USE_MUTEX
    mutex_lock(&rb->lock);
#endif
    len = min(len, rb->size - rb->in + rb->out);
    uint16_t l = min(len, rb->size - (rb->in & (rb->size - 1)));
    memcpy(rb->buffer + (rb->in & (rb->size - 1)), from, l);
    memcpy(rb->buffer, from + l, len - l);
    rb->in += len;

#ifdef ringbuf_USE_MUTEX
    mutex_unlock(&rb->lock);
#endif
    return len;
}

uint16_t ringbuf_pop(ringbuf_t *rb, uint8_t *dest, uint16_t len)
{
#ifdef ringbuf_USE_MUTEX
    mutex_lock(&rb->lock);
#endif
    unsigned int l;
    len = min(len, rb->in - rb->out);
    l = min(len, rb->size - (rb->out & (rb->size - 1)));
    memcpy(dest, rb->buffer + (rb->out & (rb->size - 1)), l);
    memcpy(dest + l, rb->buffer, len - l);
    rb->out += len;

#ifdef ringbuf_USE_MUTEX
    mutex_unlock(&rb->lock);
#endif

    return len;
}

/**
 * @brief 从环形缓冲区拷贝数据到目标数组（不修改读指针，不加锁）
 * @param rb 环形缓冲区指针
 * @param start 起始位置（基于有效数据，0表示第一个可用数据）
 * @param len 要拷贝的长度
 * @param dest 目标数组（需确保空间足够）
 * @return 实际拷贝的长度（若参数非法或数据不足返回0）
 */
uint16_t ringbuf_peek(ringbuf_t *rb, uint16_t start, uint16_t len, uint8_t *dest) {
    // 参数检查
    if (!rb || !dest || len == 0) {
        return 0;
    }

    // 计算有效数据范围
    uint16_t avail = rb->in - rb->out;
    if (start >= avail || (start + len) > avail) {
        return 0; // 起始位置或长度越界
    }

    // 计算物理位置和分段拷贝
    uint16_t out_pos = rb->out; // 当前读位置
    uint16_t first_len = min(len, rb->size - (out_pos & rb->size - 1));

    memcpy(dest, rb->buffer + (out_pos & (rb->size - 1)), first_len);
    if (len > first_len) {
        memcpy(dest + first_len, rb->buffer, len - first_len);
    }

    return len;
}

uint16_t ringbuf_remove(ringbuf_t *rb, uint16_t len) {
    unsigned int avail = rb->in - rb->out; 
    len = min(len, avail); 
    rb->out += len;
    return len;
}

/**
 * @brief 计算环形缓冲区中有效数据[n..m]的校验和（16位累加和）
 * @param rb 环形缓冲区指针
 * @param n 起始索引（基于有效数据，0表示第一个可用数据）
 * @param m 结束索引（包含在内）
 * @return 计算出的校验和，若参数非法返回0
 */
uint16_t ringbuf_checksum(ringbuf_t *rb, uint16_t n, uint16_t m) {
    if (!rb || n > m || m >= rb->in - rb->out) {
        return 0; // 参数非法或越界
    }

    uint16_t sum = 0;
    uint16_t out_pos = rb->out; // 当前读位置
    uint16_t size_mask = rb->size - 1;

    for (uint16_t i = n; i <= m; i++) {
        uint16_t pos = (out_pos + i) & size_mask; // 环形索引计算
        sum += rb->buffer[pos]; // 累加每个字节
    }

    return sum;
}

uint16_t ringbuf_len(ringbuf_t *rb)
{
    return (rb->in - rb->out);
}