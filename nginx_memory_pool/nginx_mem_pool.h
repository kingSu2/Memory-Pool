#pragma once
#include <stdlib.h>
#include <memory>
#include <string.h>

using u_char = unsigned char;
using ngx_uint_t = unsigned int;
struct ngx_pool_s;

//清理函数（回调函数）的类型
typedef void (*ngx_pool_cleanup_pt)(void* data);
struct ngx_pool_cleanup_s {
	ngx_pool_cleanup_pt   handler; //保存清理操作的回调函数
	void				  *data;           //传递给回调函数的参数
	ngx_pool_cleanup_s    *next;//所有的cleanup操作都被串在一条链表上
};

// 大块内存类型定义
struct ngx_pool_large_s {
	ngx_pool_large_s *next;
	void			 *alloc;
};

//分配小块内存的内存池的头部数据信息
struct ngx_pool_data_t {
	u_char			 *last;//可用内存的起始地址
	u_char			 *end;//可用内存的末尾地址
	ngx_pool_s		 *next;
	ngx_uint_t		 failed;//分配内存失败的次数
};

struct ngx_pool_s {
	ngx_pool_data_t		d;
	size_t			max;//小块内存和大块内存的分界线
	ngx_pool_s* current;//指向第一个可以分配内存的小块内存池
	ngx_pool_large_s* large;//指向大块内存的入口
	ngx_pool_cleanup_s* cleanup;//指向所有预制的清理操作回调函数的入口
};
//把数值d调整到临近的a的倍数
#define ngx_align(d,a) (((d)+(a - 1)) & ~(a - 1))

#define ngx_align_ptr(p,a) \
    (u_char*)(((uintptr_t)(p)+((uintptr_t) a - 1))& ~(( uintptr_t ) a - 1))

#define ngx_memzero(buf,n)  (void)memset(buf,0,n)

#define NGX_ALIGNMENT sizeof(unsigned long)
//默认一个页面的大小是4K
const int ngx_pagesize = 4096;
//ngx小块内存池可分配的最大空间
const int NGX_MAX_ALLOC_FROM_POOL = ngx_pagesize - 1;
//默认的ngx的内存池开辟的大小
const int NGX_DEFAULT_POOL_SIZE = 16 * 1024; //16k
//内存池大小按照16字节进行对齐
const int NGX_POOL_ALIGNMENT = 16;
//ngx小块内存池最小的size调整成NGX_POOL_ALIMENT的临近倍数
//为什么最小的size是这个？至少要能放下内存头
const int NGX_MIN_POOL_SIZE =
ngx_align((sizeof(ngx_pool_s) + 2 * sizeof(ngx_pool_large_s)),
	NGX_POOL_ALIGNMENT);

//移植nginx内存池代码
class ngx_mem_pool
{
public:
	void* ngx_create_pool(size_t size);
	//考虑内存字节对齐，从内存池申请内存大小
	void* ngx_palloc(size_t size);
	//不考虑字节对齐
	void* ngx_pnalloc(size_t size);
	//调用ngx_palloc ，但是会初始化为0
	void* ngx_pcalloc(size_t size);
	//释放大块内存
	void ngx_pfree(void* p);
	//内存重置
	void ngx_reset_pool();
	//内存池的销毁函数
	void ngx_destory_pool();
	//添加回调清理函数
	ngx_pool_cleanup_s* ngx_pool_cleanup_add(size_t size);

private:
	ngx_pool_s* pool_;//指向ngx内存池的入口指针
	void* ngx_palloc_small(size_t size, ngx_uint_t align);
	void* ngx_palloc_large(size_t size);
	void* ngx_palloc_block(size_t size);

};