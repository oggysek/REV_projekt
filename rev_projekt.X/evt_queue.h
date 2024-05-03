#ifndef EVT_QUEUE_H_
#define EVT_QUEUE_H_

#include <stdint.h>
#include <xc.h>

#define ENTER_CRITICAL() char tmp = (GIE & 0b1); GIE=0;
#define LEAVE_CRITICAL() GIE = (tmp & 0b1);

typedef struct{
    uint8_t fifo[32];
    uint8_t head :5;
    uint8_t tail :5;
}evt_queue_t;

void write_evt(volatile evt_queue_t *queue, uint8_t in);
uint8_t get_evt(volatile evt_queue_t *queue, uint8_t *out);

#endif 
