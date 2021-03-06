//
// Created by yiran on 18-7-6.
//

#include "buffer_pool.h"
#include "malloc.h"
#include <iostream>
#include <cstring>
#include "unistd.h"

using namespace std;

//#define REGION_SIZE 1024 * 1024 * 1024L

void *buffer_monitor(void* arg) {
    BufferPool *buffer_pool = static_cast<BufferPool *>(arg);
    while (true) {
        sleep(1);
        cout << "-----Remain buffers num:" << buffer_pool->get_remain_buffers_num() << endl;
    }

}

BufferPool::BufferPool(u_int64_t pool_size, u_int64_t buffer_size) {
    this->buffer_size = buffer_size;
    this->max_queue_length = pool_size + 1;
    this->head = 0;
    this->tail = pool_size;
    this->page_get_index = 0;
    pthread_spin_init(&spinlock, 0);

    this->memory = malloc(pool_size * buffer_size);
    if (this->memory == NULL) {
        std::cerr << "Buffer内存分配失败！！！！" <<std::endl;
        exit(1);
    }

    this->buffers = (void **) malloc(max_queue_length * sizeof(void *));
    for (int i = 0;i < pool_size;i++) {
        buffers[i] = this->memory + buffer_size * i;
        memset(buffers[i], 0, buffer_size);
    }

    /**因为去掉了自旋锁,为了保证插入和取出时数据不冲突,留出部分buffer**/
    sem_init(&remain_buffer_num, 0, pool_size - 30);

    cout << ((pool_size * (u_int64_t) buffer_size) >> 20) << "M buffer have been allocated!" << endl;

//    pthread_t tid;
//    pthread_create(&tid, NULL, buffer_monitor, this);

}


void *BufferPool::borrow_buffer() {
    sem_wait(&this->remain_buffer_num);
//    pthread_spin_lock(&spinlock);
    void* buffer = buffers[head++%max_queue_length];
//    pthread_spin_unlock(&spinlock);
//    memset(buffer, 0, buffer_size);
    return buffer;
}

void BufferPool::return_buffer(void *buffer) {
//    pthread_spin_lock(&spinlock);
    buffers[tail++%max_queue_length] = buffer;
//    pthread_spin_unlock(&spinlock);
    sem_post(&this->remain_buffer_num);
}

u_int32_t BufferPool::get_buffer_size() {
    return buffer_size;
}

int BufferPool::get_remain_buffers_num() {
    int num;
    sem_getvalue(&remain_buffer_num, &num);
    return num;
}

void BufferPool::release_all() {
    free(memory);
}

void *BufferPool::borrow_page() {
    return this->memory + (4096 * page_get_index.fetch_add(1));
}
