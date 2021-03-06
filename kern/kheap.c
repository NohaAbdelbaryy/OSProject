#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
void* ptr_end = (void*)KERNEL_HEAP_MAX;
void* ptr_last = (void*)KERNEL_HEAP_START;
int CountCall = 0;
uint32 numbPages= 0;
#define Ksize (KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE

struct AddData{
		void* address;
		int numPages;
	};

struct AddData addData[Ksize];


/////////////////////////////////////////////////
void* Nextfit(unsigned int size,int count)
{
	size=ROUNDUP(size,PAGE_SIZE);
	numbPages = size/PAGE_SIZE;
	uint32 * ptrPageTable = NULL;


	//use current to search for free frames not ptr_last so as not to change ptr_last in case
	//the pages available were not enough
	void* current= ptr_last;
	void* ptr_start= NULL;

	//use current to search for your start
	while(current!=(ptr_last-PAGE_SIZE)&&count<numbPages){


		get_page_table(ptr_page_directory,current,&ptrPageTable);

		if(ptrPageTable!=NULL){

			struct Frame_Info* frame = get_frame_info(ptr_page_directory,current,&ptrPageTable);
			if(frame==NULL)
			{
				if(count==0){
					ptr_start=current;
				}
				count++;

			}else
			{
				count = 0;
			}
		}
		if(ROUNDUP(current,PAGE_SIZE)==ptr_end){
					current=(void*)KERNEL_HEAP_START;
					count=0;
			}

		current+=PAGE_SIZE;
	}
	if(count==numbPages)
		return ptr_start;
	else
		return NULL;



}
//////////////////////////
void* Bestfit(unsigned int size, int count)
{


	size=ROUNDUP(size,PAGE_SIZE);
	numbPages = size/PAGE_SIZE;
	uint32 * ptrPageTable = NULL;
	uint32 difference = Ksize+1 ;
	int diff;

	//use current to search for free frames not ptr_last so as not to change ptr_last in case
	//the pages available were not enough
	void* current= (void*)KERNEL_HEAP_START;
	void* ptr_start= NULL;
	void* ptr_temp= NULL;

	//use current to search for your start
	while(current!=ptr_end)
	{


		get_page_table(ptr_page_directory,current,&ptrPageTable);

		if(ptrPageTable!=NULL){

			struct Frame_Info* frame = get_frame_info(ptr_page_directory,current,&ptrPageTable);
			if(frame==NULL)
			{
				if(count==0){
					ptr_temp=current;
				}
				count++;
				if(current==ptr_end-PAGE_SIZE)
				{
					diff=count - numbPages;
					if(diff<difference&&diff>=0){

					difference=diff;
					ptr_start=ptr_temp;

					}

				}

			}else if(count !=0)
			{
				//cprintf("x\n");
				diff=count - numbPages;
				if(diff<difference&&diff>=0){

					difference=diff;
					ptr_start=ptr_temp;
				}
				count = 0;
			}
		}


		current+=PAGE_SIZE;
	}
	return ptr_start;



}
////////////////////////////
void* kmalloc(unsigned int size)
{
	uint32 count = 0;
	void* ptr_final;
	void* ptr_start;
	if(isKHeapPlacementStrategyBESTFIT()){
		ptr_start= Bestfit( size, count);
	}else{
		ptr_start= Nextfit( size, count);
	}



	if(ptr_start!=NULL){
		//the ptr_final will be the one returned so we need to save the star address for allocation
		ptr_final=ptr_start;

		for(int i = 0;i<numbPages;i++){
			struct Frame_Info* newFrame;
			if(allocate_frame(&newFrame)==E_NO_MEM){
				return NULL;
			}
			if( map_frame(ptr_page_directory,newFrame,ptr_start,PERM_PRESENT|PERM_WRITEABLE)==E_NO_MEM){
				return NULL;
			}
		ptr_start+=PAGE_SIZE;
		}
		//after allocation we change the value of the ptr_last for the next allocation to happen correctly
		ptr_last=ptr_start;
		addData[CountCall].address=ptr_final;
		addData[CountCall].numPages=numbPages;
		CountCall++;
		numbPages=0;
		return ptr_final;

	}

		return NULL;

	//TODO: [PROJECT 2022 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");


	//NOTE: Allocation using NEXTFIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details


	//TODO: [PROJECT 2022 - BONUS1] Implement a Kernel allocation strategy
	// Instead of the Next allocation/deallocation, implement
	// BEST FIT strategy
	// use "isKHeapPlacementStrategyBESTFIT() ..."
	// and "isKHeapPlacementStrategyNEXTFIT() ..."
	//functions to check the current strategy
	//change this "return" according to your answer


}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2022 - [2] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	void* VAadd= virtual_address;
	int numPages=0;
	for (int i = 0; i < CountCall; i++)
		{

			if ((uint32)addData[i].address == (uint32)VAadd )
			{
				 numPages=addData[i].numPages;
				 addData[i].address=NULL;


				break;
			}
		}
	for(int i=0;i<numPages;i++)
	{
		unmap_frame(ptr_page_directory,VAadd);
		VAadd+=PAGE_SIZE;
	}

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2022 - [3] Kernel Heap] kheap_virtual_address()

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	void* VA;
       for(VA=(void*)KERNEL_HEAP_START;VA<ptr_last;VA+=PAGE_SIZE)
         {
          uint32* page_table_PTR=NULL;
          struct Frame_Info *fVA_info =NULL;
          fVA_info=get_frame_info(ptr_page_directory, VA, &page_table_PTR);
          uint32 physical_add=to_physical_address(fVA_info);
          if(fVA_info!=NULL&&(physical_add==physical_address))
           {
           return (uint32)VA;
           };
          }

return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{

	uint32* ptr_page_table = NULL;
	get_page_table(ptr_page_directory, (void*)virtual_address, &ptr_page_table);

	uint32 KPA=ptr_page_table[PTX(virtual_address)]>>12;

	return KPA * PAGE_SIZE;
}
