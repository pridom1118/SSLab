#ifndef KU_MMU_H
#define KU_MMU_H

#include <stdlib.h>
#define PFN_MASK 0xFC

struct ku_pte {
    char pte;
};
/* can be used as node for queue */
struct ku_pcb {
    char pid;
    struct ku_pte *ptba; /* page table base address */
    struct ku_pcb *next;
};

struct ku_pcbList {
    struct ku_pcb *front, *rear;
    int size;
};

struct ku_allocNode {
    unsigned char pid, PFN, VPN;
    struct ku_pte PTE;
    struct ku_allocNode *next;
};

struct ku_allocList {
    struct ku_allocNode *front, *rear;
    int size;
};

/* Global Variables */
// Consists of physical memory, swap space, process list (PCB LIST), free list, and alloc list
unsigned int ku_mmu_mem_size;
unsigned int ku_mmu_swap_size;

struct ku_pte *ku_mmu_pmem;
unsigned char *ku_mmu_sspace;
struct ku_pcbList ku_mmu_pcbList; // Process list. each has the base address of the page table for the current process
struct ku_allocList ku_mmu_allocList; //Has the mapping history of virtual page to page frames. Implemented using a queue.
unsigned char *ku_mmu_freeList; //Free list, each element indicates whether the page frame is free (0) or not (1)

void ku_pcb_list_init(struct ku_pcbList *L) {
    L->front = L->rear = NULL;
    L->size = 0;
}

void ku_pcb_list_insert(struct ku_pcbList *L, struct ku_pcb P) {
    struct ku_pcb *temp = malloc(sizeof(struct ku_pcb));
    temp->pid = P.pid;
    temp->ptba = P.ptba;
    temp->next = NULL;

    if(L->size == 0) L->front = L->rear = temp;
    else L->rear->next = temp;
    L->rear = temp;
    L->size++;
}

struct ku_pcb *ku_pcb_list_search(struct ku_pcbList *L, char pid) {
    struct ku_pcb* target = L->front;
    int isFound = 0, size = L->size;

    for(int i = 0; i < size; i++) {
        if(target->pid == pid) {
            isFound = 1; break;
        }
        target = target->next;
    }

    if(isFound) return target;
    else return 0;
}

void ku_allocList_init(struct ku_allocList *L) {
    L->front = L->rear = NULL;
    L->size = 0;
}

void ku_allocList_enqueue(struct ku_allocList *L, struct ku_allocNode target) {
    struct ku_allocNode *temp = malloc(sizeof(struct ku_allocNode));
    temp->pid = target.pid;
    temp->PFN = target.PFN;
    temp->VPN = target.VPN;
    temp->PTE = target.PTE;
    temp->next = NULL;

    if(L->size == 0) L->front = L->rear = temp;
    else {
        L->rear->next = temp;
    }
    L->rear = temp;
    L->size++;
    ku_mmu_freeList[temp->PFN] = 1; //do not forget to mark the page that the space is free
}

struct ku_allocNode ku_allocList_dequeue(struct ku_allocList *L) {
    struct ku_allocNode front;
    front.pid = front.PFN = front.VPN = 0;
    if(L->size == 0) return front;

    front.pid = L->front->pid;
    front.PFN = L->front->PFN;
    front.PTE = L->front->PTE;
    front.VPN = L->front->VPN;
    struct ku_allocNode *temp = L->front;
    L->front = temp->next;
    free(temp); temp = NULL;
    L->size--;
    ku_mmu_freeList[front.PFN] = 0; //do not forget to mark that the space is free
    return front;
}

unsigned char ku_freeList_firstFit() {
    unsigned char PFN = 0;
    unsigned int size = ku_mmu_mem_size / 4;
    // PFN 0 is reserved for the OS
    for(int i = 1; i < size; i++) {
        if (ku_mmu_freeList[i] == 0) {
            PFN = i;
            ku_mmu_freeList[i] = 1;
            break;
        }
    }
    return PFN;
}

void *ku_mmu_init(unsigned int mem_size, unsigned int swap_size) {
    ku_mmu_mem_size = mem_size;
    ku_mmu_swap_size = swap_size;
    ku_mmu_pmem = (struct ku_pte*) calloc(mem_size, sizeof(struct ku_pte));
    ku_mmu_sspace = (struct ku_pte*) calloc(swap_size / 4, sizeof(struct ku_pte));
    if (!ku_mmu_pmem || !ku_mmu_sspace) return 0; //calloc failed

    /* PFN 0 reserved for OS */
    for(int i = 0; i < 4; i++) ku_mmu_pmem[i].pte = 1; // 00000001
    /* 0th page in swap space is not used */
    ku_mmu_sspace[0] = 1;
    ku_pcb_list_init(&ku_mmu_pcbList);
    ku_mmu_freeList = calloc(mem_size / 4, sizeof(char));
    ku_mmu_freeList[0] = (char)1; //reserved for OS
    ku_allocList_init(&ku_mmu_allocList);
    return ku_mmu_pmem;
}

/* performs context switch */
int ku_run_proc(char pid, struct ku_pte **ku_cr3) {
    struct ku_pcb newProc;
    newProc.pid = pid;

    /* traverse freelist and find */
    struct ku_pcb* cur = ku_mmu_pcbList.front;
    int size = ku_mmu_pcbList.size;
    int inList = 0;
    for(int i = 0; i < size; i++) {
        if(cur->pid == pid) {
            inList = 1; break;
        }
        cur = cur->next;
    }

    //if pcb already exists just pass the base of the page table to ku_cr3
    if(inList) *(struct ku_pte**)ku_cr3 = cur->ptba;
        //if not then create new pcb, allocate new page table then insert pcb into the list
    else {
        struct ku_pcb tmp;
        tmp.pid = pid;
        tmp.ptba = (struct ku_pte *)calloc(256, sizeof(struct ku_pte));
        if(!tmp.ptba) return -1; //if calloc fails
        tmp.next = NULL;
        ku_pcb_list_insert(&ku_mmu_pcbList, tmp);
        *(struct ku_pte**)ku_cr3 = tmp.ptba;
    }
    return 0;
}

/* handles page fault */
int ku_page_fault(char pid, char va) {
    /* case 0: va = 0 -> defined as null pointer in this assignment */
    if(va == 0) return -1;
    unsigned char PFN = 0;
    struct ku_pcb *target = ku_pcb_list_search(&ku_mmu_pcbList, pid);
    unsigned char VPN = (va & PFN_MASK) >> 2;

    /* case 1: unmapped page */
    if(target->ptba[VPN].pte == 0) {
        PFN = ku_freeList_firstFit();
        /* case 1-1: not enough physical memory */
        if(PFN == 0) {
            struct ku_allocNode swapOut;
            int isFound = 0;

            /* check whether the swap space has some free space */
            for(int i = 1; i < ku_mmu_swap_size / 4; i++) {
                if(ku_mmu_sspace[i] == 0) {
                    isFound = i;
                    break;
                }
            }

            if(!isFound) return -1; //Swap space is also full
            else {
                swapOut = ku_allocList_dequeue(&ku_mmu_allocList);
                PFN = swapOut.PFN;
                struct ku_pcb *node = ku_mmu_pcbList.front;
                for(int i = 0; i < ku_mmu_pcbList.size; i++) {
                    if(node->pid == swapOut.pid) {
                        node->ptba[swapOut.VPN].pte = 0;
                        node->ptba[swapOut.VPN].pte |= (unsigned char)isFound << 1;
                        break;
                    }
                    node = node->next;
                }
                ku_mmu_sspace[isFound] = 1;

                struct ku_allocNode cur;
                target->ptba[VPN].pte |= PFN << 2;
                target->ptba[VPN].pte |= 1; //set the present bit to 1
                cur.pid = pid; cur.PFN = PFN; cur.VPN = VPN; cur.PTE = target->ptba[VPN];
                cur.next = NULL;
                ku_allocList_enqueue(&ku_mmu_allocList, cur);
            }
        } else {
            struct ku_allocNode cur;
            target->ptba[VPN].pte |= PFN << 2;
            target->ptba[VPN].pte |= 1; //set the present bit to 1
            cur.pid = pid; cur.PFN = PFN; cur.VPN = VPN; cur.PTE = target->ptba[VPN];
            cur.next = NULL;
            ku_allocList_enqueue(&ku_mmu_allocList, cur);
        }
        /* case 2. currently not present in physical memory */
    } else if (!(target->ptba[VPN].pte & 0x01) || (target->ptba[VPN].pte & 0x02)) {
        /* first free the page in the swap space using 7-bit offset from PTE */
        unsigned char offset = target->ptba[VPN].pte;
        ku_mmu_sspace[(offset & 0xFE) >> 1] = 0;
        PFN = ku_freeList_firstFit();

        /* physical memory is full, evict a page frame into swap space */
        if(PFN == 0) {
            struct ku_allocNode swapOut;
            unsigned char isFound = 0;

            /* remove the page for swap in from the swap space */
            swapOut = ku_allocList_dequeue(&ku_mmu_allocList);
            PFN = swapOut.PFN;

            /* swap space for swapping out the page in the physical memory  */
            for(int i = 1; i < ku_mmu_swap_size / 4; i++) {
                if(ku_mmu_sspace[i] == 0) {
                    isFound = i;
                    break;
                }
            }

            /* then map the virtual page with the page frame */
            struct ku_pcb *node = ku_mmu_pcbList.front;
            for(int i = 0; i < ku_mmu_pcbList.size; i++) {
                if(node->pid == swapOut.pid) {
                    node->ptba[swapOut.VPN].pte = 0;
                    node->ptba[swapOut.VPN].pte |= isFound << 1;
                    break;
                }
                node = node->next;
            }
            ku_mmu_sspace[isFound] = 1;
        }

        /* if there are enough physical memory then just swap in the page */
        struct ku_allocNode cur;
        PFN = ku_freeList_firstFit();
        target->ptba[VPN].pte = 0;
        target->ptba[VPN].pte |= PFN << 2;
        target->ptba[VPN].pte |= 1; //set the present bit to 1
        cur.pid = pid; cur.PFN = PFN; cur.VPN = VPN; cur.PTE = target->ptba[VPN];
        cur.next = NULL;
        ku_allocList_enqueue(&ku_mmu_allocList, cur);
    }
    return 0;
}

#endif //KU_MMU_H
