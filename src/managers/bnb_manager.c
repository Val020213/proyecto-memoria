#include "bnb_manager.h"

#include "stdio.h"

int current_pid = -1;
int *pids;
int valid_count = 0;
addr_t *bases;
int bound;

int *heap_pointer;
int *stack_pointer;

addr_t **virtual_mem;

#define SIZEOFVALUES 4

// Utiles

// PID
int find_pid(int pid)
{
  for (int i = 0; i < valid_count; i++)
    if (pids[i] == pid)
      return i;
  return -1;
}

int add_pid(int pid)
{
  int index = find_pid(pid);

  if (index >= 0)
    return 1;

  pids[valid_count] = pid;
  valid_count += 1;
  return 0;
}

// STACK
int sim_stack_op(int is_del)
{
  int stack_pointer_temp = stack_pointer[current_pid] + (is_del ? SIZEOFVALUES : -SIZEOFVALUES);

  if (stack_pointer_temp > bound || stack_pointer_temp < heap_pointer[current_pid])
    return 1;
  return 0;
}
int valid_stack_op(int is_del)
{
  if (is_del && sim_stack_op(is_del))
    return 1;
  return 0;
}

// HEAP
int find_addr(addr_t addr)
{
  for (int i = 0; i < heap_pointer[current_pid]; i++)
    if (virtual_mem[current_pid][i] == addr)
      return i;
  return -1;
}

int delete_addr(addr_t addr)
{
  int index = find_addr(addr);
  if (index < 0)
    return 1;

  while (index <= heap_pointer[current_pid])
  {
    virtual_mem[current_pid][index] = virtual_mem[current_pid][index + 1];
    index += 1;
  }
  heap_pointer -= 1;
  return 0;
}

int valid_free_heap_op(addr_t addr)
{
  int index = find_addr(addr);
  if (index < 0)
    return 1;
  return 0;
}

int valid_malloc_heap_op(size_t size)
{
  int spaces = size / SIZEOFVALUES;
  if (heap_pointer[current_pid] + spaces > heap_pointer[current_pid])
    return 1;
  return 0;
}

int in_addr(addr_t addr)
{
  if (bases[current_pid] > addr || addr > bases[current_pid] + bound)
    return 1;
  return 0;
}

int add_addr(addr_t addr)
{
  if (find_addr(addr) >= 0 || in_addr(addr) || valid_malloc_heap_op(SIZEOFVALUES))
    return 1;
  virtual_mem[current_pid][stack_pointer[current_pid]] = addr;
  stack_pointer[current_pid] += 1;
  return 0;
}


// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  fprintf(stderr, "Not Implemented\n");
  exit(1);
}
