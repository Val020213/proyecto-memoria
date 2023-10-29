#include "seg_manager.h"

#include "stdio.h"

#include "../memory.h"

#define MAX_PROGRAM_COUNT 20 // this value must be equal to the number of programs in test.c

// Global Variables
addr_t *seg_base;
size_t *seg_heap;
size_t *seg_stack;

size_t *seg_bound_stack;
size_t *seg_bound_seg_heap;

int *seg_pids;
int seg_valid_count = MAX_PROGRAM_COUNT;

size_t *seg_virtual_mem; // -1 if is free, else pid

int seg_current_pid_index = -1;
int seg_bits = 1;
int seg_direction_size = -1;

// Utiles
void binary_representation(int *repr, size_t value)
{
  for (int i = 0; i < seg_direction_size; i++)
  {
    repr[i] = value % 2;
    value /= 2;
  }
}
size_t full_offset()
{
  return m_size();
}

size_t offset(int *repr)
{
  size_t offsetv = 0;
  int power = 1;
  for (int i = 0; i < seg_direction_size - seg_bits; i++)
  {
    offsetv += power * i;
    power *= 2;
  }
  return offsetv;
}

int read_va(int *repr, size_t va) // return 1 if from stack
{
  binary_representation(repr, va);
  return repr[seg_direction_size - seg_bits] == 1;
}

int get_pa(addr_t va_addr, addr_t *pa)
{
  int repr[seg_direction_size];
  int is_stack = read_va(repr, va_addr);
  size_t off = offset(repr);

  if (is_stack && full_offset() - off >= seg_bound_stack[seg_current_pid_index])
  {
    printf("Out of bounds Offset: %ld  Bounds: %ld  IsStack: %d", off, seg_bound_stack[seg_current_pid_index], is_stack);
    return 1;
  }

  if (!is_stack && off >= seg_bound_seg_heap[seg_current_pid_index])
  {
    printf("Out of bounds Offset: %ld  Bounds: %ld  IsStack: %d", off, seg_bound_seg_heap[seg_current_pid_index], is_stack);
    return 1;
  }

  *pa = seg_base[seg_current_pid_index] + (is_stack) ? off - full_offset() : off;
  return 0;
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
  seg_direction_size = how_many_bits_needed(m_size());
  printf("Tamanno de la memoria %ld, Se necesitan %d bits para direccionar toda la memoria\n", m_size(), seg_direction_size);
  seg_direction_size += seg_bits;
  printf("Agg el bit de seg: %d\n", seg_direction_size);
}

// Reserva un espacio en el seg_heap de tamaño 'size' y establece un puntero al
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
