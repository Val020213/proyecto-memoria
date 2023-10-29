#include "seg_manager.h"

#include "stdio.h"

#include "../memory.h"
#define MAX_PROGRAM_COUNT 20 // this value must be equal to the number of programs in test.c

// Global Variables
addr_t *base;
size_t *heap;
size_t *stack;

int direction_size = -1;

// Utiles
int binary()
{
  
}

int is_stack(addr_t addr) // 1 bit, 1 -> stack or 0 -> heap
{

}

int get_addr(addr_t addr)
{

}

int how_many_bits_needed(size_t memory_size) // buscar si C tiene un log?
{
  int bits = 0;
  while (memory_size > 1)
  {
    memory_size /= 2;
    bits++;
  }
  return bits;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv)
{
  int direction_bits = how_many_bits_needed(m_size());
  printf("Tamanno de la memoria %ld, Se necesitan %d bits para direccionar toda la memoria\n", m_size(), direction_bits);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Quita un elemento del stack
int m_seg_pop(byte *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
