#include "page_table.h"
#include "config.h"

static page_table_entry_t page_table[PAGE_TABLE_SIZE];

void page_table_init(void)
{
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i].frame = -1;
        page_table[i].valid = 0;
        page_table[i].reference_bit = 0;
        page_table[i].aging_counter = 0;
    }
}

int page_table_lookup(int page)
{
    /*
     * MODIFICADO (Passo 3):
     * Se a página for válida, retorna o quadro correspondente.
     * Caso contrário, retorna -1 para disparar o Page Fault.
     */
    if (page >= 0 && page < PAGE_TABLE_SIZE && page_table[page].valid) {
        return page_table[page].frame;
    }

    return -1;
}

void page_table_update(int page, int frame)
{
    /*
     * MODIFICADO (Passo 3):
     * Atualiza a entrada da tabela de páginas mapeando-a para o frame e tornando-a válida.
     * Também inicializa os controles de envelhecimento com segurança.
     */
    if (page >= 0 && page < PAGE_TABLE_SIZE) {
        page_table[page].frame = frame;
        page_table[page].valid = 1;
        page_table[page].reference_bit = 1; // Já entra com bit de referência ativo pelo acesso
        page_table[page].aging_counter = 0x80; // Inicializa com o MSB ativo (página recém-carregada)
    }
}

void page_table_invalidate(int page)
{
    /*
     * MODIFICADO (Passo 3/4):
     * Invalida a entrada da página (chamado quando a página vira vítima no algoritmo de substituição).
     */
    if (page >= 0 && page < PAGE_TABLE_SIZE) {
        page_table[page].valid = 0;
        page_table[page].frame = -1;
        page_table[page].reference_bit = 0;
        page_table[page].aging_counter = 0;
    }
}

void page_table_set_reference(int page)
{
    /*
     * MODIFICADO (Passo 3):
     * Marcar o bit de referência da página como 1.
     * Esta função é chamada pelo main.c a cada acesso a endereço lógico.
     */
    if (page >= 0 && page < PAGE_TABLE_SIZE) {
        page_table[page].reference_bit = 1;
    }
}

void page_table_update_aging(void)
{
    /*
     * MODIFICADO (Passo 3/4):
     * Implementada a atualização do LRU aproximado (Aging).
     * * Como o main.c chama essa função a cada instrução/acesso, ela varre a tabela
     * atualizando os contadores históricos de todas as páginas válidas.
     */
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        if (page_table[i].valid) {
            // 1. Desloca o contador de envelhecimento 1 bit para a direita
            page_table[i].aging_counter >>= 1;

            // 2. Se o bit de referência for 1, insere-o no Bit Mais Significativo (MSB -> 0x80)
            if (page_table[i].reference_bit) {
                page_table[i].aging_counter |= 0x80;
            }

            // 3. Zera o bit de referência para o próximo período de amostragem
            page_table[i].reference_bit = 0;
        }
    }
}

int page_table_get_frame(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return -1;
    }

    return page_table[page].frame;
}

int page_table_is_valid(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return 0;
    }

    return page_table[page].valid;
}

unsigned char page_table_get_aging_counter(int page)
{
    if (page < 0 || page >= PAGE_TABLE_SIZE) {
        return 0;
    }

    return page_table[page].aging_counter;
}