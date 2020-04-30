struct mem
{
    /*
    可能很多人都会认为next与prev是一个指针，分别指向下一个内存块与上一个内存块，
    但是其实这两个字段表示的是目的地址的偏移量，基地址是整个内存堆的起始地址
    */
     /** index (-> ram[next]) of the next struct */
     mem_size_t next;				
     /** index (-> ram[prev]) of the previous struct */
     mem_size_t prev;				
     /** 1: this area is used; 0: this area is unused */
     u8_t used;				
 #if MEM_OVERFLOW_CHECK
     /** this keeps track of the user allocation size for guard checks */
     mem_size_t user_size;
 #endif
};

/*
申请的内存最小为12字节，因为一个内存块最起码需要保持mem结构体的信息，
以便于对内存块进行操作，而该结构体在对齐后的内存大小就是12字节
*/
#define MIN_SIZE    12	

/*
内存堆常见的实现方式是，通过申请一个大的内存空间，作为内存分配释放的总内存。
LWIP也是这样实现的先定义的一个数组，然后将这块内存进行管理，看代码:
*/
//实际的内存堆数据声明
#ifndef LWIP_RAM_HEAP_POINTER
LWIP_DECLARE_MEMORY_ALIGNED(ram_heap, MEM_SIZE_ALIGNED + (2U*SIZEOF_STRUCT_MEM));
#define LWIP_RAM_HEAP_POINTER ram_heap
#endif
//中间的宏的定义
#ifndef LWIP_DECLARE_MEMORY_ALIGNED
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) u8_t variable_name[LWIP_MEM_ALIGN_BUFFER(size)]
#endif
