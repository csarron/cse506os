#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/tarfs.h>
#include <sys/virmm.h>
#include <sys/process.h>
#include <sys/stdlib.h>
#include <sys/test_threads.h>

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE]; //stakc used by boot
uint32_t* loader_stack; //points to top of the OS loader stack. Seems loader is 32-bit instead of 64-bit
extern char kernmem, physbase; //only symbols-only declared here. Defined in linker script
struct tss_t tss;

size_t console_row;
size_t console_column;
uint32_t page_index = 0;
uint32_t page_num = 0;
uint64_t length = 0;
uint32_t first = 0;
page_sp* page_struct_start;

void
start (uint32_t* modulep, void* physbase, void* physfree)
{

  struct smap_t
  {
    uint64_t base, length;
    uint32_t type;
  }__attribute__((packed)) *smap;
  while (modulep[0] != 0x9001)
    modulep += modulep[1] + 2;
  for (smap = (struct smap_t*) (modulep + 2);
      smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4); ++smap)
    {
      if (smap->type == 1 /* mem
       ory */&& smap->length != 0)
	{
	  length = smap->length;
	  page_num = length >> 12;
	  page_index = (uint32_t) ((smap->base) >> 12);

	  printf ("smaplength is %x, smapbase is %x\n", smap->length,
		  smap->base);

	  printf ("Available Physical Memory [%x-%x]\n", smap->base,
		  smap->base + smap->length);
	}
    }

  page_struct_start = (page_sp*) (0xffffffff80000000UL + physfree);

  printf ("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
  dprintf ("page_num=%x\n", page_num);
  dprintf ("page index=%x\n", page_index);
  init_phy_page (get_num_init ((uint64_t) physfree), page_num, page_index);

  first = allocate_page ();
  dprintf ("first free is %x\n", first);

  dprintf ("kernmem starts in %p\n", &kernmem);

  init_mm ();  //initialize kernel memory

  initial_mapping ();	// map 32MB physical memory to virtual memory

  init_phy_page (8192, page_num, page_index); //init first 32mb as used, kmalloc take over

  task_struct* idle = create_idle_thread ();

  create_thread ((uint64_t) & func_a, "a thread");
  create_thread ((uint64_t) & func_b, "b thread");
//  create_thread ((uint64_t)&func_c, "c thread");
  print_threads (idle);

  dprintf ("begin scheduling\n");
  int i = 0;
  while (1)
    {
      dprintf ("I'm idle %d \n", i++);
      schedule ();
//      __asm__ __volatile__ ("hlt");
      if (i == 10)
	{
	  create_thread ((uint64_t) & func_c, "c thread");
	}
    }
  while (1)
    ;

}

void
boot (void)
{
// note: function changes rsp, local stack variables can't be practically used
//	register char *s, *v;
  __asm__(
      "movq %%rsp, %0;" //loader_stack points sto old rsp, namely, to the top of OS loader stack
      "movq %1, %%rsp;"//now rsp points to the top of stack[INITIAL_STACK_SIZE];
      :"=g"(loader_stack)
      :"r"(&stack[INITIAL_STACK_SIZE])
  );

  reload_gdt ();
  setup_tss ();

  console_initialize ();

  reload_idt ();
  timer_init ();

  start (
      (uint32_t*) ((char*) (uint64_t) loader_stack[3] + (uint64_t)
	  & kernmem - (uint64_t) & physbase),
      &physbase, (void*) (uint64_t) loader_stack[4]);

//	s = "!!!!! start() returned !!!!!";
//	for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;

  while (1)
    ;
}
