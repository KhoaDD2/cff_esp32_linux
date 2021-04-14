#include "logfile.h"

#include "ringbuffer.h"

/**
 * @file
 * Implementation of ring buffer functions.
 */

struct ringbuffer_data_t {
    int i_rb_r;
    int i_rb_w;
} g_dd = {
    .i_rb_r = 0,
    .i_rb_w = 0
};

void ring_buffer_init(ring_buffer_t  *buffer) {
  buffer->tail_index = 0;
  buffer->head_index = 0;
  buffer->min_remain = RING_BUFFER_SIZE;
}

void ring_buffer_queue(ring_buffer_t *buffer, char data) 
{
    uint32_t remain_slot=RING_BUFFER_SIZE-ring_buffer_num_items(buffer);
    
    /* Is buffer full? */
    if(ring_buffer_is_full(buffer)) 
    {
        /* Is going to overwrite the oldest byte */
        /* Increase tail index */
        LOGI("Bad ring buffer, never leave it happend");
        buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK);
    }

    if(buffer->min_remain>remain_slot)
        buffer->min_remain=remain_slot;
    
    /* Place data in buffer */
    buffer->buffer[buffer->head_index] = data;
    buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK);
    g_dd.i_rb_w++;
}

void ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size) {
  /* Add bytes; one by one */
  ring_buffer_size_t i;
  for(i = 0; i < size; i++) {
    ring_buffer_queue(buffer, data[i]);
  }
}

ring_buffer_size_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data) {
  if(ring_buffer_is_empty(buffer)) {
    /* No items */
    return 0;
  }

  *data = buffer->buffer[buffer->tail_index];
  buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK);
  g_dd.i_rb_r++;
  return 1;
}

ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len) {
  if(ring_buffer_is_empty(buffer)) {
    /* No items */
    return 0;
  }

  char *data_ptr = data;
  ring_buffer_size_t cnt = 0;
  while((cnt < len) && ring_buffer_dequeue(buffer, data_ptr)) {
    cnt++;
    data_ptr++;
  }
  return cnt;
}

ring_buffer_size_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index) {
  if(index >= ring_buffer_num_items(buffer)) {
    /* No items at index */
    return 0;
  }

  /* Add index to pointer */
  ring_buffer_size_t data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK);
  *data = buffer->buffer[data_index];
  return 1;
}

extern inline uint8_t ring_buffer_is_empty(ring_buffer_t *buffer);
extern inline uint8_t ring_buffer_is_full(ring_buffer_t *buffer);
extern inline ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer);

