#include <sys/virmm.h>
#include <sys/process.h>
#include <sys/sbunix.h>
#include <sys/stdlib.h>

uint64_t
get_pml4e_index (uint64_t addr)
{
  return (((addr) >> PML4_SHIFT_BITS) & SHIFT_MASK);
}

uint64_t
get_pdpte_index (uint64_t addr)
{
  return (((addr) >> PDPT_SHIFT_BITS) & SHIFT_MASK);
}

uint64_t
get_pdte_index (uint64_t addr)
{
  return (((addr) >> PDT_SHIFT_BITS) & SHIFT_MASK);
}

uint64_t
get_pte_index (uint64_t addr)
{
  return (((addr) >> PT_SHIFT_BITS) & SHIFT_MASK);
}

//get table entry virtual address
uint64_t
get_entry_viraddr (uint64_t entry)
{
  return (0xFFFFFFFF80000000UL | entry);
}

void*
alloc_pt (int flag)
{
  if (flag == KERN)
    {
      return (void*) allocate_page ();
    }
  else

  if (flag == USER)
    {
      return kmalloc (USERPT);
    }
  else
    return NULL;
}

// set up page directory pointer table
void*
set_pdpt (pml4_t pml4, uint64_t pml4_index, int flag)
{
  pdpt_t pdpt = (pdpt_t) alloc_pt (flag);
  uint64_t pdpt_entry = (uint64_t) pdpt;

  if (flag == USER)
    {
      pdpt_entry -= VIR_START;
      pdpt_entry |= PTE_U;
    }

  pdpt_entry |= (PTE_P | PTE_W);

  //pdpt_entry &= PTE_EX; // clear executable bit
  pml4->PML4E[pml4_index] = pdpt_entry;

  return (void *) pdpt;
}

// set up page directory table
void*
set_pdt (pdpt_t pdpt, uint64_t pdpt_index, int flag)
{
  pdt_t pdt = (pdt_t) alloc_pt (flag);
  uint64_t pdt_entry = (uint64_t) pdt;

  if (flag == USER)
    {
      pdt_entry -= VIR_START;
      pdt_entry |= PTE_U;
    }

  pdt_entry |= (PTE_P | PTE_W);

  //pdt_entry &= PTE_EX; // clear executable bit
  pdpt->PDPTE[pdpt_index] = pdt_entry;

  return (void *) pdt;
}

// set up page table
void*
set_pt (pdt_t pdt, uint64_t pdt_index, int flag)
{
  pt_t pt = (pt_t) alloc_pt (flag);
  uint64_t pt_entry = (uint64_t) pt;
  if (flag == USER)
    {
      pt_entry -= VIR_START;
      pt_entry |= PTE_U;
    }

  pt_entry |= (PTE_P | PTE_W);

  //pt_entry &= PTE_EX; // clear executable bit
  pdt->PDTE[pdt_index] = pt_entry;

  return (void *) pt;
}

pml4_t global_PML4;
//pml4_t user_global_PML4;

uint64_t vmalloc_base;

uint64_t ktask_base;
uint64_t kstack_base;
uint64_t mm_base;
uint64_t userpt_base;
uint64_t vma_base;
uint64_t file_base;

int task_bitmap[PROCESS_NUMBER];
int stack_bitmap[KSTACK_NUMBER];
int mm_bitmap[MM_NUMBER];
int userpt_bitmap[USERPT_NUMBER];
int vma_bitmap[VMA_NUMBER];
int file_bitmap[FILE_NUMBER];

void
init_mm ()
{

  //setup level 4 page directory
  global_PML4 = (pml4_t) alloc_pt (KERN);

  ktask_base = get_kmalloc_base () + VIR_START;
  kstack_base = ktask_base + PROCESS_NUMBER * PAGE_SIZE;
  mm_base = kstack_base + KSTACK_NUMBER * PAGE_SIZE;
  userpt_base = mm_base + MM_NUMBER * PAGE_SIZE;
  vma_base = userpt_base + USERPT_NUMBER * PAGE_SIZE;
  file_base = vma_base + VMA_NUMBER * PAGE_SIZE;

  dprintf ("ktask_base : %p\n", ktask_base);
  dprintf ("kstack_base : %p\n", kstack_base);
  memset ((void *) ktask_base, 0, PROCESS_NUMBER * PAGE_SIZE);
  memset ((void *) kstack_base, 0, KSTACK_NUMBER * PAGE_SIZE);
  memset ((void *) mm_base, 0, MM_NUMBER * PAGE_SIZE);
  memset ((void *) userpt_base, 0, USERPT_NUMBER * PAGE_SIZE);
  memset ((void *) vma_base, 0, VMA_NUMBER * PAGE_SIZE);
  memset ((void *) file_base, 0, FILE_NUMBER * PAGE_SIZE);

  memset (task_bitmap, 0, PROCESS_NUMBER);
  memset (stack_bitmap, 0, KSTACK_NUMBER);
  memset (mm_bitmap, 0, MM_NUMBER);
  memset (userpt_bitmap, 0, USERPT_NUMBER);
  memset (vma_bitmap, 0, VMA_NUMBER);
  memset (file_bitmap, 0, FILE_NUMBER);
//  int i = 0;
//  for (i = 0; i < PROCESS_NUMBER; i++)
//    dprintf ("task_bimap[%d] is: %d ", i, task_bitmap[i]);
//  for (i = 0; i < KSTACK_NUMBER; i++)
//    dprintf ("stack_bitmap[%d] is: %d ", i, task_bitmap[i]);

  vmalloc_base = USER_VIR_START;
}

//pml4_t
//get_pml4 (int flag)
//{
//  if (flag == KERN)
//    return kern_global_PML4;
//
//  if (flag == USER)
//    return user_global_PML4;
//
//  return NULL;
//}
//
//pml4_t
//set_user_pml4 ()
//{
//  return (pml4_t) alloc_pt (USER);
//}

void
map_virmem_to_phymem (uint64_t vir_addr, uint64_t phy_addr, int flag)
{

  pdpt_t pdpt;
  pdt_t pdt;
  pt_t pt;

  uint64_t pml4e_index = get_pml4e_index (vir_addr);
  uint64_t pml4e = 0;
//  if (flag == USER)
//    pml4e =
//	((pml4_t) get_entry_viraddr ((uint64_t) global_PML4))->PML4E[pml4e_index];
//  if (flag == KERN)
  pml4e = global_PML4->PML4E[pml4e_index];

  if (pml4e & PTE_P)
    {
      uint64_t pdpt64 = get_entry_viraddr (pml4e);
      pdpt64 &= CLEAR_OFFSET;
      pdpt = (pdpt_t) pdpt64;

    }
  else
    {
      pdpt = (pdpt_t) set_pdpt (global_PML4, pml4e_index, flag);
    }

  uint64_t pdpte_index = get_pdpte_index (vir_addr);

  uint64_t pdpte = pdpt->PDPTE[pdpte_index];

  if (pdpte & PTE_P)
    {
      uint64_t pdt64 = get_entry_viraddr (pdpte);
      pdt64 &= CLEAR_OFFSET;
      pdt = (pdt_t) pdt64;

    }
  else
    {
      pdt = (pdt_t) set_pdt (pdpt, pdpte_index, flag);
    }

  uint64_t pdte_index = get_pdte_index (vir_addr);
  uint64_t pdte = pdt->PDTE[pdte_index];
  if (pdte & PTE_P)
    {
      uint64_t pt64 = get_entry_viraddr (pdte);
      pt64 &= CLEAR_OFFSET;
      pt = (pt_t) pt64;

    }
  else
    {
      pt = (pt_t) set_pt (pdt, pdte_index, flag);
    }

  uint64_t pte = phy_addr;
  pte |= (PTE_P | PTE_W);
  if (flag == USER)
    pte |= PTE_U;

  //pte &= PTE_EX; // clear executable bit
  uint64_t pte_index = get_pte_index (vir_addr);
  pt->PTE[pte_index] = pte;

}

//begin mapping physical memory from 0 to 10MB

void
initial_mapping ()
{
  uint64_t map_size = 0x2000000; //physical size 32MB

  uint64_t vir_addr = VIR_START;
  uint64_t phy_addr = PHY_START;
  uint64_t page_count = 0;
  while (phy_addr < map_size)
    {
      map_virmem_to_phymem (vir_addr, phy_addr, KERN);
      phy_addr += PAGE_SIZE;
      vir_addr += PAGE_SIZE;
      page_count++;
    }

  set_CR3 ((uint64_t) global_PML4);
  global_PML4 = (pml4_t) get_entry_viraddr ((uint64_t) global_PML4);
}

void
set_CR3 (uint64_t CR3)
{
  __asm volatile("mov %0, %%cr3":: "b"(CR3));
}

uint64_t
get_CR3 ()
{
  uint64_t cur_cr3;
  __asm volatile("mov %%cr3, %0" : "=r" (cur_cr3));

  return cur_cr3;
}

uint64_t
get_base (int flag)
{
  switch (flag)
    {
    case TASK:
      return ktask_base;
    case KSTACK:
      return kstack_base;
    case MM:
      return mm_base;
    case USERPT:
      return userpt_base;
    case VMA:
      return vma_base;
    case FILE:
      return file_base;
    default:
      return 0;
    };
}

/* allocate 4K size usable kernel virtual memory
 * flag,1 indicates task_struct or
 *  	0 for kernel_stack
 */

void*
kmalloc (int flag)
{
  int i = 0;
  uint64_t base = get_base (flag);
  switch (flag)
    {
    case TASK:
      while (i < PROCESS_NUMBER)
	{
	  if (task_bitmap[i] == 0)
	    {
	      task_bitmap[i] = 1;
	      break;
	    }
	  else
	    i++;
	}
      break;
    case KSTACK:
      while (i < KSTACK_NUMBER)
	{
	  if (stack_bitmap[i] == 0)
	    {
	      stack_bitmap[i] = 1;
	      break;
	    }
	  else
	    i++;
	}
      break;
    case MM:
      while (i < MM_NUMBER)
	{
	  if (mm_bitmap[i] == 0)
	    {
	      mm_bitmap[i] = 1;
	      break;
	    }
	  else
	    i++;
	}
      break;
    case USERPT:
      while (i < USERPT_NUMBER)
	{
	  if (userpt_bitmap[i] == 0)
	    {
	      userpt_bitmap[i] = 1;
	      break;
	    }
	  else
	    i++;
	}
      break;
    case VMA:
      while (i < VMA_NUMBER)
	{
	  if (vma_bitmap[i] == 0)
	    {
	      vma_bitmap[i] = 1;
	      break;
	    }
	  else
	    i++;
	}
      break;
    case FILE:
      while (i < FILE_NUMBER)
	{
	  if (file_bitmap[i] == 0)
	    {
	      file_bitmap[i] = 1;
	      break;
	    }
	  else
	    i++;
	}
      break;
    default:
      return NULL;
    }

  base += i * PAGE_SIZE;
  dprintf ("kmalloc return %d base %p\n", flag, base);

  memset ((void *) base, 0, PAGE_SIZE);

  return (void *) (base);

}

void
kfree (void* addr, int flag)
{
  memset ((void *) addr, 0, PAGE_SIZE);
  uint64_t base = get_base (flag);
  int bitmap_pos = ((uint64_t) addr - base) / PAGE_SIZE;
  switch (flag)
    {
    case TASK:
      task_bitmap[bitmap_pos] = 0;
      break;
    case KSTACK:
      stack_bitmap[bitmap_pos] = 0;
      break;
    case MM:
      mm_bitmap[bitmap_pos] = 0;
      break;
    case USERPT:
      userpt_bitmap[bitmap_pos] = 0;
      break;
    case VMA:
      vma_bitmap[bitmap_pos] = 0;
      break;
    case FILE:
      file_bitmap[bitmap_pos] = 0;
      break;
    default:
      printf ("CANNOT FREE %p!\n", addr);
      return;
    }

}

void*
umalloc (size_t size)
{
//  user_global_PML4 = (pml4_t) get_CR3 ();

  void* ret_addr = (void*) vmalloc_base;

  int page_num = size / PAGE_SIZE;
  if (size % PAGE_SIZE)
    {
      page_num += 1;
    }

  while (page_num-- > 0)
    {
      map_virmem_to_phymem (vmalloc_base, allocate_page_user (), USER);
      vmalloc_base += PAGE_SIZE;
    }

  memset ((void *) ret_addr, 0, size);

  return ret_addr;
}
