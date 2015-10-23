#include <stdio.h>
#include <stdlib.h>
#include <sys/physical.h>

extern char physfree,kernofs;
static uint32_t page_inuse_num=0;
extern uint32_t page_index;
extern uint32_t page_num;
//page_sp* page_struct_start=(page_sp*)(uint32_t)(&physfree);
//page_sp* page_struct_start=(page_sp*)(0xffffffff80350000);
 //int page_num=((page_length)>>12);
extern page_sp* page_struct_start;

//each page is 4kb


int init_phy_page(uint32_t num, uint32_t page_num, uint32_t page_index)
{
	if(num>page_num)
	{
		printf("ERROR: number is too big to init page");
	}

	//printf("physfree=%x\n",physfree);

	page_sp* page_tmp;
	uint32_t i=0;
	uint32_t index_=page_index;

	while(i<page_num)
	{
		
		page_tmp=page_struct_start+i;

	if(i<num)
	{
		//printf("%d\n",i);
		page_tmp->info=PAGE_OCP;
		//printf("%x\n",page_tmp->info);
	}
	else if(i>=num)
	{
		page_tmp->info=PAGE_FREE;
	}

	page_tmp->index=index_;

	page_tmp->next=page_index+1;

	i++;
	index_++;
	//(page_index)=(page_index)+1;
	page_inuse_num+=num;

    }

    printf("lalalala");
    return 0;
}

void set_used(uint32_t index)
{
	page_sp* tmp=(page_sp*)(page_struct_start+index);
	tmp->info&=PAGE_OCP;
}


void set_free(uint32_t index)
{
	page_sp* tmp=(page_sp*)(page_struct_start+index);
	tmp->info|=PAGE_FREE;
}



uint32_t find_first_free()
{
	uint32_t i=0;
	//uint32_t start=0;

	//printf("struct start :%x\n", page_struct_start);



	for(i=0;i<page_num;i++)
	{
		//printf("page_num:%x",page_num);
		page_sp* tmp=(page_struct_start+i);
		if(tmp->info&PAGE_OCP)
		{
			//printf("find one! %x",i);
			continue;
		}
		else
		{
			//tmp->info=PAGE_OCP;
			//printf("find one! %x",i);
			return i;
		}
		//start=i;
	}
	
	//return start;
	return 0;
}

uint32_t find_free_pages(uint32_t num)
{
	uint32_t i=0,j=0;
	//int number=0;
	//int start=0;

	for(i=0;i<page_num-num+1;i++)
	{
		if((i+num)>page_num)
		{
			printf("ERROR: num overflows");
			break;
		}
        
		page_sp* tmp=(page_struct_start+i);

		if(!(tmp->info & PAGE_OCP))
		{
			int number=0;
			for(j=0;j<num;j++)
			{
				if(!(tmp->info & PAGE_OCP))
				{
					number++;

				}
			}

			if(number==num)
			{
				return i;
			}

		}

	}

			printf("we don't have enough physical memory to allocate");
			return 0;

	//return start;
}

uint32_t allocate_page()
{
	uint32_t start=find_first_free();
	//return (uint64_t)(0xffffffff80200000+start);
	page_sp* tmp=(page_sp*)(page_struct_start+start);
	tmp->info=PAGE_OCP;
	//printf("see1: %x\n",tmp->index);
	return (uint32_t)((tmp->index)<<12);

}

uint32_t allocate_pages(uint32_t num)
{
	uint32_t i;
	uint32_t start=find_free_pages(num);
	//return (uint64_t)(0xffffffff80200000+start);
	page_sp* tmp=(page_struct_start+start);
	//page_sp* temp=(page_struct_start+start);
	for(i=0;i<num;i++)
	{
		page_sp* temp=(page_struct_start+i);
		temp->info=PAGE_OCP;
	}
	//return (uint64_t)(start<<12)
	return (uint32_t)((tmp->index)<<12);
}






