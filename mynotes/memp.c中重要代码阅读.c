//分析的前提条件：MEMP_STATS = 0, MEMP_OVERFLOW_CHECK = 0
1.//memp.c刚上来就有一个宏：
    #define LWIP_MEMPOOL(name,num,size,desc) LWIP_MEMPOOL_DECLARE(name,num,size,desc)
    #include "lwip/priv/memp_std.h"
//继续追LWIP_MEMPOOL_DECLARE的定义如下，看完继续懵逼中。。。。但是不能慌一个个宏替换出来：
  #define LWIP_MEMPOOL_DECLARE(name,num,size,desc) \
  LWIP_DECLARE_MEMORY_ALIGNED(memp_memory_ ## name ## _base, ((num) * (MEMP_SIZE + MEMP_ALIGN_SIZE(size)))); \
    \
  LWIP_MEMPOOL_DECLARE_STATS_INSTANCE(memp_stats_ ## name) \
    \
  static struct memp *memp_tab_ ## name; \
    \
  const struct memp_desc memp_ ## name = { \
    DECLARE_LWIP_MEMPOOL_DESC(desc) \
    LWIP_MEMPOOL_DECLARE_STATS_REFERENCE(memp_stats_ ## name) \
    LWIP_MEM_ALIGN_SIZE(size), \
    (num), \
    memp_memory_ ## name ## _base, \
    &memp_tab_ ## name \
  };
//里面用到的相关宏的实现汇总如下：
    #define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) u8_t variable_name[LWIP_MEM_ALIGN_BUFFER(size)]
    #define LWIP_MEMPOOL_DECLARE_STATS_INSTANCE(name)
    #define DECLARE_LWIP_MEMPOOL_DESC(desc)
    #define LWIP_MEMPOOL_DECLARE_STATS_REFERENCE(name)
    #define LWIP_MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1U) & ~(MEM_ALIGNMENT-1U))
//然后把里面的宏一个个代入，最后就是这样一个过程(以LWIP_RAW为例子，下面展开得到)：
                |
                |
                |
                v
#define LWIP_MEMPOOL_DECLARE(name,num,size,desc) \
  memp_memory_RAW_PCB_base[(num) * (MEMP_SIZE + MEMP_ALIGN_SIZE(size)))]; \
  static struct memp *memp_tab_RAW_PCB; \
  const struct memp_desc memp_RAW_PCB = {\
    LWIP_MEM_ALIGN_SIZE(size), \
    (num), \
    memp_memory_RAW_PCB_base,\
    &memp_tab_RAW_PCB\ 
  };

//然后就是这样子的宏替换，此处未全部列举，以LWIP_UDP使能为例，并且是使能了LWIP_DEBUG
#define LWIP_MEMPOOL(name,num,size,desc) LWIP_MEMPOOL_DECLARE(name,num,size,desc)
#include "lwip/priv/memp_std.h"  
                |
                |
                |
                v
  memp_memory_UDP_PCB_base[(num) * (MEMP_SIZE + MEMP_ALIGN_SIZE(size)))]; \
  static struct memp *memp_tab_UDP_PCB; 
  const struct memp_desc memp_UDP_PCB = {
    “UDP_PCB”                      
    LWIP_MEM_ALIGN_SIZE(size),     
    (num),                         
    memp_memory_UDP_PCB_base, 
    &memp_tab_UDP_PCB
  };  



2.//memp.c中接着会看到第二个奇怪的宏
const struct memp_desc *const memp_pools[MEMP_MAX] = {
#define LWIP_MEMPOOL(name,num,size,desc) &memp_ ## name,
#include "lwip/priv/memp_std.h"
};
                |
                |
                |
                v
//这样memp_pools就将整个mempool的内存串到了一个结构体数组中。
const struct memp_desc *const memp_pools[MEMP_MAX] = {
  &memp_RAW_PCB,
  &memp_UDP_PCB,
  .
  .
  .
}
//然后这里还还需要了解一个结构体的定义如下:
struct memp_desc {
#if defined(LWIP_DEBUG) || MEMP_OVERFLOW_CHECK || LWIP_STATS_DISPLAY
  /** Textual description */
  const char *desc;
#endif /* LWIP_DEBUG || MEMP_OVERFLOW_CHECK || LWIP_STATS_DISPLAY */
  /** Element size */
  u16_t size;
  /*MEMP_MEM_MALLOC=1时,使用mem_malloc/mem_free代替分配池*/
#if !MEMP_MEM_MALLOC 
  /** Number of elements */
  u16_t num;
  /** Base address */
  u8_t *base;
  /** First free element of each pool. Elements form a linked list. */
  struct memp **tab;
#endif /* MEMP_MEM_MALLOC */
};
//要注意此时每个memp_pools中的memp_desc结构体中的memp_tab_UDP_PCB还只是一个指针的指针(二级指针)，
//并未有具体的实际意义。然后memp_init会进行这一工作

3.//去掉宏不编译的部分memp_init如下:
void memp_init(void)
{
  u16_t i;
  /* for every pool: */
  for (i = 0; i < LWIP_ARRAYSIZE(memp_pools); i++) {
    memp_init_pool(memp_pools[i]);
  }
}
//就是循环调用memp_init_pool，接着看去掉宏简化后的memp_init_pool
void memp_init_pool(const struct memp_desc *desc)
{
  int i;
  struct memp *memp;

  *desc->tab = NULL;
  memp = (struct memp *)LWIP_MEM_ALIGN(desc->base);

  /* create a linked list of memp elements */
  for (i = 0; i < desc->num; ++i) {
    memp->next = *desc->tab;
    *desc->tab = memp; //注意这里的->的优先级高
    memp = (struct memp *)(void *)((u8_t *)memp + MEMP_SIZE + desc->size
  }
}
//到这里所有内存池的定义和初始化已经完成了,可以看图："lwip初始化后的pool结构by野火"。