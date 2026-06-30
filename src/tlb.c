#include "tlb.h"
#include "config.h"

static tlb_entry_t tlb[TLB_SIZE];

/*
 * Índice da próxima posição a ser substituída.
 * Essa variável implementa FIFO no TLB.
 */
static int fifo_next = 0;

void tlb_init(void)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page = -1;
        tlb[i].frame = -1;
        tlb[i].valid = 0;
    }

    fifo_next = 0;
}

int tlb_lookup(int page)
{
    /*
     * MODIFICADO (Passo 5):
     * Procura a página no TLB.
     * Se encontrar uma entrada válida, retorna o quadro correspondente (TLB Hit).
     * Caso contrário, retorna -1 (TLB Miss).
     */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            return tlb[i].frame;
        }
    }

    return -1;
}

void tlb_insert(int page, int frame)
{
    /*
     * MODIFICADO (Passo 5):
     * Inserir uma entrada no TLB.
     */

    // 1. Política: Se a página já estiver no TLB, atualiza o frame e encerra.
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            tlb[i].frame = frame;
            return;
        }
    }

    // 2. Política: Se existir uma entrada inválida antes de encher, usamos ela.
    // Como o fifo_next avança sequencialmente, as entradas vazias naturais do início 
    // do programa serão preenchidas na ordem (0, 1, 2...). 
    // Quando tlb[fifo_next] já possuir valid == 1 (TLB cheio), a substituição FIFO 
    // acontecerá naturalmente sobrescrevendo a entrada mais antiga.
    tlb[fifo_next].page = page;
    tlb[fifo_next].frame = frame;
    tlb[fifo_next].valid = 1;

    // Incremento circular do ponteiro FIFO (vai de 0 a 15 e volta para 0)
    fifo_next = (fifo_next + 1) % TLB_SIZE;
}

void tlb_remove(int page)
{
    /*
     * MODIFICADO (Passo 5):
     * Invalida uma entrada do TLB associada à página informada.
     * Chamada no handle_page_fault do memory.c quando uma página vira vítima do Aging.
     */
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            tlb[i].valid = 0;
            tlb[i].page = -1;
            tlb[i].frame = -1;
            // Paramos no primeiro pois não deve haver páginas duplicadas no TLB
            break; 
        }
    }
}

void tlb_clear(void)
{
    tlb_init();
}