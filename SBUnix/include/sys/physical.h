#ifndef _PHYSICAL_H
#define _PHYSICAL_H

#define PAGE_OCP 0x0001
#define PAGE_FREE 0x0000
//#define page_num 10000 //to be changed
//number of total pages which is memory_length >>12    which is smap->length>>12
//#define page_index 10000//to be changed
//start of page index which is smap->base >>12

struct page
{
  uint64_t info;
  uint64_t next;
  uint64_t index;
  int64_t ref_count; // -1: free, 0: used by one process, n: used by n+1 process
}__attribute__((packed));

typedef struct page page_sp;

int
init_phy_page (uint32_t, uint32_t, uint32_t);
uint32_t
find_first_free ();
uint32_t
find_free_pages (uint32_t);
uint64_t
allocate_page ();
uint64_t
allocate_pages (uint32_t);
//get initial page numbers
uint32_t
get_num_init (uint64_t physfree);
//alloacte single for vmalloc usage
uint64_t
allocate_page_user ();

uint64_t
get_kmalloc_base ();

void
phy_free (uint64_t addr);


/*
 * input: a physical address
 * output: the page frame descriptor of physical page which phys_addr belongs to
 */
page_sp *get_page_frame_descriptor (uint64_t phys_addr);

#endif