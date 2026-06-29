#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "config.h"
#include "page_table.h"
#include "tlb.h"

static signed char physical_memory[NUM_FRAMES][FRAME_SIZE];

/*
 * Indica qual página está carregada em cada quadro.
 * Valor -1 indica quadro livre.
 */
static int frame_to_page[NUM_FRAMES];

static FILE *backing = NULL;

void memory_init(FILE *backing_store)
{
    backing = backing_store;

    for (int i = 0; i < NUM_FRAMES; i++) {
        frame_to_page[i] = -1;

        for (int j = 0; j < FRAME_SIZE; j++) {
            physical_memory[i][j] = 0;
        }
    }
}

static int find_free_frame(void)
{
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (frame_to_page[i] == -1) {
            return i;
        }
    }

    return -1;
}

int handle_page_fault(int page)
{
    int frame = find_free_frame();

    if (frame == -1) {
        /*
         * MODIFICADO (Passo 4):
         * Seleciona a página vítima usando o algoritmo de Aging.
         * Encontra em qual quadro ela estava e limpa as estruturas de mapeamento.
         */
        int victim_page = select_victim_page();

        // Obtém o quadro físico associado à página vítima antes de invalidá-la
        frame = page_table_get_frame(victim_page);

        // Invalida a página vítima na tabela de páginas para que futuros acessos gerem Page Fault
        page_table_invalidate(victim_page);

        // Remove a página vítima do TLB (caso ela esteja lá dentro)
        tlb_invalidate(victim_page);
    }

    if (backing == NULL) {
        fprintf(stderr, "Erro interno: BACKING_STORE nao inicializado.\n");
        exit(1);
    }

    // Calcula a posição do byte inicial da página no arquivo binário
    long offset_arquivo = page * FRAME_SIZE;

    // Reposiciona o ponteiro de leitura do arquivo BACKING_STORE
    if (fseek(backing, offset_arquivo, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: fseek falhou para a pagina %d\n", page);
        exit(1);
    }

    // Lê os 256 bytes da página diretamente para o quadro (frame) correspondente na memória física
    size_t bytes_lidos = fread(physical_memory[frame], sizeof(signed char), FRAME_SIZE, backing);
    
    if (bytes_lidos != FRAME_SIZE) {
        fprintf(stderr, "Erro: fread leu apenas %zu bytes da pagina %d\n", bytes_lidos, page);
        exit(1);
    }

    // Atualiza o mapeamento indicando qual página está residindo neste quadro físico
    frame_to_page[frame] = page;

    // Atualiza a tabela de páginas mapeando o novo par (página, quadro)
    page_table_update(page, frame);

    return frame;
}

int select_victim_page(void)
{
    /*
     * MODIFICADO (Passo 4):
     * Selecionar a página VÁLIDA com menor aging_counter.
     */
    int victim = -1;
    unsigned char min_aging = 0xFF; // Inicializa com o maior valor possível para fins de comparação

    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        // Só avalia páginas que estão atualmente mapeadas na memória física
        if (page_table_is_valid(i)) {
            unsigned char current_aging = page_table_get_aging_counter(i);
            
            // Busca pelo menor valor numérico do contador
            if (current_aging < min_aging) {
                min_aging = current_aging;
                victim = i;
            }
        }
    }

    // Caso de salvaguarda (se nenhuma página for válida por algum erro de estado, o que não deve ocorrer)
    if (victim == -1) {
        fprintf(stderr, "Erro fatal: Nenhuma pagina valida encontrada para substituicao.\n");
        exit(1);
    }

    return victim;
}

signed char read_memory(int frame, int offset)
{
    /*
     * MODIFICADO (Passo 2):
     * Retorna o byte armazenado na posição exata da memória física.
     */
    return physical_memory[frame][offset];
}

int get_page_loaded_in_frame(int frame)
{
    if (frame < 0 || frame >= NUM_FRAMES) {
        return -1;
    }

    return frame_to_page[frame];
}