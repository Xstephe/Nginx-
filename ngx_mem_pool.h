#pragma once
//类型重定义
using u_char = unsigned char;
using ngx_uint_t = unsigned int;
//using uintptr_t= unsigned int ;
#include<stdlib.h>
#include<memory.h>

//类型前置说明
struct ngx_pool_s;

/*
清理函数（回调函数）的类型
*/
typedef void (*ngx_pool_cleanup_pt)(void* data);
struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;//定义了一个函数指针，保存清理操作的回调函数
    void*                 data;//传递给回调函数的参数
    ngx_pool_cleanup_s*   next;//所有的cleanup 清理操作都被串在一条链表上
};

/*/
大块内存的头部信息
*/
struct ngx_pool_large_s {
    ngx_pool_large_s*   next;//所有的大块内存分配也是被串在一条链表上
    void*               alloc;//保存分配出去的大块内存的起始地址
};

/*
分配小块内存的内存池头部数据信息
*/
 struct ngx_pool_data_t {
    u_char*               last;//小块内存池可用内存的起始地址
    u_char*               end;//小块内存池可用内存的末尾地址
    ngx_pool_s*           next;//所有小块内存池都被串在了一条链表上
    ngx_uint_t            failed;//记录了当前小块内存池分配内存失败的次数
};


/*
ngx内存池的头部信息和管理成员信息
*/
struct ngx_pool_s {
    ngx_pool_data_t       d;//存储的是当前小块内存池的使用情况
    size_t                max;//存储的是小块内存和大块内存的分界线
    ngx_pool_s*           current;//指向第一个提供小块内存分配的小块内存池
    ngx_pool_large_s*     large;//指向大块内存（链表）的入口地址
    ngx_pool_cleanup_s*   cleanup;//指向所有预置的清理操作回调函数（链表）
};

#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)//buf缓冲区清0

#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))//把数值d调整为邻近的a的倍数
#define ngx_align_ptr(p, a)                                                  \
(u_char *) (((size_t) (p) + ((size_t) a - 1)) & ~((size_t) a - 1)) //把指针p调整到a的临近的倍数
#define NGX_ALIGNMENT   sizeof(unsigned long) //小块内存分配考虑字节对齐时的单位

const int ngx_pagesize = 4096;//默认一个物理界面的大小4K

const int NGX_MAX_ALLOC_FROM_POOL = ngx_pagesize - 1;//ngx小块内存池可分配的最大空间

const int NGX_DEFAULT_POOL_SIZE = 16 * 1024;//定义常量，表示一个默认的ngx内存池开辟的大小 16k

const int NGX_POOL_ALIGNMENT = 16;         //内存池按照16字节进行对齐

const int  NGX_MIN_POOL_SIZE =                                                    
ngx_align((sizeof(ngx_pool_s) + 2 * sizeof(ngx_pool_large_s)),NGX_POOL_ALIGNMENT);  //ngx小块内存池最小的size调整成NGX_POOL_ALIGNMENT的临近的倍数

//移植nginx内存池的代码，用OOP来实现
class ngx_mem_pool
{
public:
    void* ngx_create_pool(size_t size);//创建指定size大小的内存池，但是小块内存池不超过一个页面的大小

    void* ngx_palloc(size_t size);//考虑内存字节对齐，从内存池申请size大小的内存

    void* ngx_pnalloc(size_t size);//和上面的函数一样，不考虑内存对齐

    void* ngx_pcalloc(size_t size);//调用的是ngx_palloc实现内存分配，但会初始化0

    void ngx_pfree(void* p);//释放大块内存

    void ngx_reset_pool();//内存重置函数

    void ngx_destroy_pool();//内存池的销毁函数

    ngx_pool_cleanup_s* ngx_pool_cleanup_add( size_t size);//添加回调清理操作函数
private:
    ngx_pool_s* pool; //指向ngx内存池的入口指针

    void* ngx_palloc_small(size_t size, ngx_uint_t align);//小块内存分配

    void* ngx_palloc_large(size_t size);//大块内存分配

    void* ngx_palloc_block(size_t size);//分配新的小块内存池
};