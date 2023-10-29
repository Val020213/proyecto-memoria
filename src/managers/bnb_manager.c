#include "bnb_manager.h"

#include "stdio.h"
#include "../memory.h"
#define MAX_PROGRAM_COUNT 20 // this value must be equal to the number of programs in test.c

int current_pid_index = -1;
int *pids;
size_t valid_count = MAX_PROGRAM_COUNT;
addr_t *bases;
size_t bound;

size_t *heap_pointer;
size_t *stack_pointer;

int **virtual_mem; // 1 if is reserved

#define SIZEOFVALUES 1

int find_pid(int pid)
{
  for (size_t i = 0; i < valid_count; i++)
    if (pids[i] == pid)
      return i;
  return -1;
}

int get_free_pid()
{
  for (size_t i = 0; i < valid_count; i++)
    if (pids[i] == -1)
      return i;
  return -1;
}

int add_pid(int pid)
{
  int index = find_pid(pid);
  int free = get_free_pid();

  if (index >= 0 || free < 0)
    return -1;

  pids[free] = pid;
  return free;
}

int sim_stack_op(int is_del)
{
  size_t stack_pointer_temp = stack_pointer[current_pid_index] + (is_del ? SIZEOFVALUES : -SIZEOFVALUES);

  if (stack_pointer_temp >= bound || stack_pointer_temp < heap_pointer[current_pid_index])
    return 1;
  return 0;
}

int valid_stack_op(int is_del)
{
  if (is_del && sim_stack_op(is_del))
    return 1;
  return 0;
}

int in_addr(addr_t addr)
{
  if (bases[current_pid_index] > addr || addr > bases[current_pid_index] + bound)
    return 1;
  return 0;
}

int find_addr(addr_t addr)
{
  if (in_addr(addr) || addr > stack_pointer[current_pid_index])
    return -1;
  return addr - bases[current_pid_index];
}

int update_heap_pointer() // TODO: check this function
{
  for (size_t i = 0; i < stack_pointer[current_pid_index]; i++)
    if (virtual_mem[current_pid_index][i])
      heap_pointer[current_pid_index] = i;
  return 0;
}

int delete_addr(addr_t addr)
{
  int index = find_addr(addr);
  if (index < 0 || virtual_mem[current_pid_index][index] == 0)
    return 1;

  virtual_mem[current_pid_index][index] = 0;
  update_heap_pointer();
  return 0;
}

size_t min(size_t x, size_t y)
{
  return x < y ? x : y;
}

int find_malloc_size(size_t size, size_t *slots)
{
  int spaces = size / SIZEOFVALUES;
  int count = 0;
  for (size_t i = 0; i < min(stack_pointer[current_pid_index], bound); i++)
  {
    if (count == spaces)
      return 0;

    if (virtual_mem[current_pid_index][i] == 0)
    {
      slots[count] = i;
      count++;
    }

    else
      count = 0;
  }
  return 1;
}

int resb_addr(size_t index_addr)
{
  if (in_addr(bases[current_pid_index] + index_addr) || virtual_mem[current_pid_index][index_addr])
    return 1;
  virtual_mem[current_pid_index][index_addr] = 1;
  update_heap_pointer();
  return 0;
}

int clean_pid(int pid)
{
  int index = find_pid(pid);

  if (index < 0)
    return -1;

  pids[index] = -1;

  stack_pointer[index] = bound - 1;
  heap_pointer[index] = 0;

  for (size_t i = 0; i < bound; i++)
    virtual_mem[index][i] = 0;
  return index;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_bnb_init(int argc, char **argv)
{
  // initialize global variables
  current_pid_index = -1;

  pids = (int *)malloc(MAX_PROGRAM_COUNT * sizeof(int));
  valid_count = MAX_PROGRAM_COUNT;

  bases = (addr_t *)malloc(MAX_PROGRAM_COUNT * sizeof(addr_t));
  bound = 0;

  heap_pointer = (size_t *)malloc(MAX_PROGRAM_COUNT * sizeof(size_t));
  stack_pointer = (size_t *)malloc(MAX_PROGRAM_COUNT * sizeof(size_t));

  virtual_mem = (int **)malloc(MAX_PROGRAM_COUNT * sizeof(int *));
  for (int i = 0; i < MAX_PROGRAM_COUNT; i++)
    virtual_mem[i] = (int *)malloc(bound * sizeof(int));

  // No valid pids
  for (int i = 0; i < MAX_PROGRAM_COUNT; i++)
    pids[i] = -1;

  // Reset to 0
  for (int i = 0; i < MAX_PROGRAM_COUNT; i++)
    for (size_t j = 0; j < bound; j++)
      virtual_mem[i][j] = 0;

  // initialize sectors
  bound = m_size() / MAX_PROGRAM_COUNT;

  for (int i = 0; i < MAX_PROGRAM_COUNT; i++)
  {
    bases[i] = i * bound;
    stack_pointer[i] = bound - 1;
    heap_pointer[i] = 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_bnb_malloc(size_t size, ptr_t *out)
{
  size_t slot[size];

  if (find_malloc_size(size, slot))
  {
    printf("ERROR: No hay espacio en el heap [bnb_malloc / find malloc size()] \n");
    return 1;
  }
  for (size_t i = 0; i < size; i++)
    resb_addr(slot[i]);

  out->addr = bases[current_pid_index] + slot[0];
  printf("Se resevo %ld para el proceso %d \n", size, pids[current_pid_index]);
  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_bnb_free(ptr_t ptr)
{
  return delete_addr(ptr.addr);
}

// Agrega un elemento al stack
int m_bnb_push(byte val, ptr_t *out)
{
  if (valid_stack_op(0))
    return 1;
  m_write(stack_pointer[current_pid_index] + bases[current_pid_index], val);
  out->addr = stack_pointer[current_pid_index] + bases[current_pid_index];
  stack_pointer[current_pid_index] -= SIZEOFVALUES;
  return 0;
}

// Quita un elemento del stack
int m_bnb_pop(byte *out)
{
  if (valid_stack_op(1))
    return 1;
  stack_pointer[current_pid_index] += SIZEOFVALUES;
  *out = m_read(stack_pointer[current_pid_index] + bases[current_pid_index]);
  return 0;
}

// Carga el valor en una dirección determinada
int m_bnb_load(addr_t addr, byte *out)
{
  if (in_addr(addr) || !virtual_mem[current_pid_index][find_addr(addr)])
    return 1;
  *out = m_read(addr);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_bnb_store(addr_t addr, byte val)
{
  if (in_addr(addr) || !virtual_mem[current_pid_index][find_addr(addr)])
    return 1;
  m_write(addr, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_bnb_on_ctx_switch(process_t process)
{
  set_curr_owner(process.pid);
  current_pid_index = find_pid(process.pid);

  if (current_pid_index < 0)
  {
    current_pid_index = get_free_pid();
    pids[current_pid_index] = process.pid;
    m_set_owner(bases[current_pid_index], bases[current_pid_index] + bound);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_bnb_on_end_process(process_t process)
{
  int index = clean_pid(process.pid);
  if (index >= 0)
    m_unset_owner(bases[index], bases[index] + bound);
}
