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
    /*
     * MODIFICADO (Passo 2):
     * 1. Procurar quadro livre.
     * 2. Se não houver quadro livre, selecionar página vítima (Será completado no Passo 4).
     * 5. Ler a página correta do BACKING_STORE.bin.
     * 6. Atualizar frame_to_page.
     */

    int frame = find_free_frame();

    if (frame == -1) {
        // Lógica de substituição que faremos no Passo 4.
        // Por enquanto, apenas um placeholder para evitar travamentos nos primeiros testes.
        int victim_page = select_victim_page();

        /*
         * TODO (Passo 4):
         * Obter o quadro da página vítima.
         * Invalidar tabela e TLB.
         */

        (void) victim_page;

        frame = 0; // Temporário até implementarmos o Aging
    }

    if (backing == NULL) {
        fprintf(stderr, "Erro interno: BACKING_STORE nao inicializado.\n");
        exit(1);
    }

    /*
     * MODIFICADO (Passo 2):
     * Fazer fseek para page * FRAME_SIZE (ou PAGE_SIZE).
     * Fazer fread de FRAME_SIZE bytes para physical_memory[frame].
     */
    
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

    return frame;
}

int select_victim_page(void)
{
    /*
     * TODO (Passo 4):
     * Selecionar a página válida com menor aging_counter.
     */

    return 0;
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