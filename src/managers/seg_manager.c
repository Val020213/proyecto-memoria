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

size_t seg_default_stack_bound;
// size_t seg_default_heap_bound;

int *seg_pids;
int seg_valid_count = MAX_PROGRAM_COUNT;

int *seg_virtual_mem; // -1 if is free, else pid
size_t seg_virtual_mem_size;

int seg_current_pid_index = -1;
int is_new = 0;
int seg_bits = 1;
int seg_direction_size = -1;
/*
ARREGLAR LAS DIRECCIONES DE MEMORIA
*/

// Utiles

void seg_update_heap()
{
  for (size_t i = 0; i < seg_bound_seg_heap[seg_current_pid_index]; i++)
  {
    if (seg_virtual_mem[seg_base[seg_current_pid_index] + i] == seg_pids[seg_current_pid_index])
    {
      seg_heap[seg_current_pid_index] = i;
    }
  }
}

size_t get_from_binary(int off)
{
  size_t value = 1;
  for (int i = 0; i < seg_direction_size; i++)
    value *= 2;
  return value + off;
}

size_t get_va_stack(int index)
{
  return get_from_binary(index);
}

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
    offsetv += power * repr[i];
    power *= 2;
  }
  return offsetv;
}

int read_va(int *repr, size_t va) // return 1 if from stack
{
  binary_representation(repr, va);
  return repr[seg_direction_size - seg_bits] == 1;
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

int seg_find_pid(int pid)
{
  for (int i = 0; i < MAX_PROGRAM_COUNT; i++)
    if (seg_pids[i] == pid)
      return i;
  return -1;
}

int find_free_pid()
{
  for (int i = 0; i < MAX_PROGRAM_COUNT; i++)
    if (seg_pids[i] == -1)
      return i;
  return -1;
}

int seg_add_pid(int pid)
{
  int index = seg_find_pid(pid);
  int free = find_free_pid();

  if (index >= 0 || free < 0)
  {
    printf("Error: pid %d already exists or there is no free space\n", pid);
    return 1;
  }

  seg_pids[free] = pid;
  return free;
}

int seg_del_pid(int pid)
{
  int index = seg_find_pid(pid);

  if (index < 0)
  {
    printf("Error: pid %d does not exists\n", pid);
    return 1;
  }

  seg_pids[index] = -1;
  return 0;
}

size_t seg_min(size_t x, size_t y)
{
  return x < y ? x : y;
}

// free list

int seg_resb(int l, int r)
{
  for (int i = l; i < r; i++)
  {
    if (seg_virtual_mem[i] != -1)
    {
      printf("Error: Addr %d is not free, Owner: %d  Currentpid : %d  \n", i, seg_virtual_mem[i], seg_pids[seg_current_pid_index]);
      return 1;
    }
    seg_virtual_mem[i] = seg_pids[seg_current_pid_index];
  }
  return 0;
}

void free_v_memo(int l, int r)
{
  for (int i = l; i < r; i++)
  {
    seg_virtual_mem[i] = -1;
  }
  m_unset_owner(l, r - 1);
}

int first_fit(size_t need)
{
  for (size_t i = 0; i < seg_virtual_mem_size; i++)
    if (seg_virtual_mem[i] == -1)
    {
      size_t j = i;
      while (j < seg_virtual_mem_size && seg_virtual_mem[j] == -1)
        j++;
      if (j - i > need)
        return i;
      i = j;
    }
  return -1;
}

int fusion_fit()
{
  for (int i = 0; i < seg_valid_count; i++)
  {
    if (seg_pids[i] >= 0)
    {
      if (seg_stack[i] < seg_bound_stack[i])
      {
        free_v_memo(seg_stack[i], seg_bound_stack[i]);
        seg_bound_stack[i] = seg_stack[i] + 1;
      }

      if (seg_heap[i] < seg_bound_seg_heap[i] - 1)
      {
        free_v_memo(seg_heap[i], seg_bound_seg_heap[i]);
        seg_bound_seg_heap[i] = seg_heap[i] + 1;
      }
    }
  }
  return 1;
}

int extend_proc(int need_stack, int need_heap)
{
  fusion_fit();
  int addr_stack = seg_base[seg_current_pid_index] - seg_bound_stack[seg_current_pid_index];

  while (need_stack > 0)
  {
    if (seg_virtual_mem[addr_stack] != seg_pids[seg_current_pid_index] && seg_virtual_mem[addr_stack] != -1)
    {
      printf("Error: Stack is not free\n");
      return 1;
    }
    free_v_memo(addr_stack, addr_stack - 1);
    addr_stack--;
    need_stack--;
  }

  seg_resb(addr_stack, seg_base[seg_current_pid_index] - seg_bound_stack[seg_current_pid_index]);
  m_set_owner(addr_stack, seg_base[seg_current_pid_index] - seg_bound_stack[seg_current_pid_index]);
  seg_bound_stack[seg_current_pid_index] += need_stack;

  int addr_heap = seg_base[seg_current_pid_index] + seg_bound_seg_heap[seg_current_pid_index];

  while (need_heap > 0)
  {
    if (seg_virtual_mem[addr_heap] != seg_pids[seg_current_pid_index] && seg_virtual_mem[addr_heap] != -1)
    {
      printf("Error: Heap is not free\n");
      return 1;
    }
    free_v_memo(addr_heap, addr_heap + 1);
    addr_heap++;
    need_heap--;
  }
  seg_resb(seg_base[seg_current_pid_index] + seg_bound_seg_heap[seg_current_pid_index], addr_heap + 1);
  m_set_owner(seg_base[seg_current_pid_index] + seg_bound_seg_heap[seg_current_pid_index], addr_heap);
  seg_bound_seg_heap[seg_current_pid_index] += need_heap;
  return 0;
}

int seg_find_malloc_size(size_t size, size_t *slots)
{
  int spaces = size;
  int count = 0;
  for (size_t i = seg_base[seg_current_pid_index]; i < seg_base[seg_current_pid_index] + seg_bound_seg_heap[seg_current_pid_index]; i++)
  {
    if (count == spaces)
      return 0;

    if (seg_virtual_mem[i] == -1)
    {
      slots[count] = i;
      count++;
    }
    else
      count = 0;
  }
  return 1;
}

int create_new_space(size_t need)
{
  int free_space = first_fit(need + seg_default_stack_bound);

  if (free_space < 0 && fusion_fit() && (free_space = first_fit(need + seg_default_stack_bound)) < 0)
  {
    printf("Error: No hay espacio para crear un nuevo proceso\n");
    return 1;
  }

  seg_stack[seg_current_pid_index] = 0;
  seg_bound_stack[seg_current_pid_index] = seg_default_stack_bound;

  seg_base[seg_current_pid_index] = free_space + seg_default_stack_bound;

  seg_bound_seg_heap[seg_current_pid_index] = need;
  seg_heap[seg_current_pid_index] = need - 1;

  seg_resb(free_space, free_space + seg_default_stack_bound + need);
  m_set_owner(free_space, free_space + seg_default_stack_bound + need - 1);
  return 0;
}

// Esta función se llama cuando se inicializa un caso de prueba
void m_seg_init(int argc, char **argv)
{
  seg_direction_size = how_many_bits_needed(m_size());

  printf("Tamanno de la memoria %ld, Se necesitan %d bits para direccionar toda la memoria\n", m_size(), seg_direction_size);

  seg_direction_size += seg_bits;

  printf("Agg el bit de seg: %d\n", seg_direction_size);

  seg_virtual_mem_size = m_size();
  seg_virtual_mem = malloc(seg_virtual_mem_size * sizeof(int));

  printf("Iniciando la memoria virtual\n");
  for (size_t i = 0; i < seg_virtual_mem_size; i++)
    seg_virtual_mem[i] = -1;

  seg_pids = malloc(MAX_PROGRAM_COUNT * sizeof(int));
  seg_base = malloc(MAX_PROGRAM_COUNT * sizeof(addr_t));
  seg_heap = malloc(MAX_PROGRAM_COUNT * sizeof(size_t));
  seg_stack = malloc(MAX_PROGRAM_COUNT * sizeof(size_t));
  seg_bound_seg_heap = malloc(MAX_PROGRAM_COUNT * sizeof(size_t));
  seg_bound_stack = malloc(MAX_PROGRAM_COUNT * sizeof(size_t));

  seg_default_stack_bound = 10;
  // seg_default_heap_bound = 10;
  printf("Iniciando pidds\n");

  for (int i = 0; i < MAX_PROGRAM_COUNT; i++)
    seg_pids[i] = -1;
}

// Reserva un espacio en el seg_heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_seg_malloc(size_t size, ptr_t *out)
{
  if (is_new)
  {
    is_new = 0;
    printf("Creando nuevo espacio de direccion\n");
    create_new_space(size);
    printf("Se creo un nuevo espacio de direccion\n Base: %ld Heap: %ld  Bound:%ld \n", seg_base[seg_current_pid_index], seg_heap[seg_current_pid_index], seg_bound_seg_heap[seg_current_pid_index]);
    out->addr = 0;
    return 0;
  }

  size_t slot[size];

  if (seg_find_malloc_size(size, slot))
  {
    printf("Error: No hay espacio para crear un nuevo proceso, hay que extender el heap Heap: %ld Bound %ld\n", seg_heap[seg_current_pid_index], seg_bound_seg_heap[seg_current_pid_index]);

    if (fusion_fit() && extend_proc(0, seg_bound_seg_heap[seg_current_pid_index] - seg_heap[seg_current_pid_index] + size))
    {
      printf("Error: No se pudo extender el heap\n");
      return 1;
    }
    printf("Se extendio el heap dentro de su bound Heap:%ld  Bound:%ld\n", seg_heap[seg_current_pid_index], seg_bound_seg_heap[seg_current_pid_index]);
    out->addr = seg_heap[seg_current_pid_index] + 1;
  }

  else
  {
    printf("Se encontro espacio para crear un nuevo proceso dentro del propio heap %ld -> %ld\n", slot[0], slot[size - 1]);
    seg_resb(slot[0], slot[size - 1] + 1);
    m_set_owner(slot[0], slot[size - 1]);
    out->addr = slot[0];
  }

  return 0;
}

// Libera un espacio de memoria dado un puntero.
int m_seg_free(ptr_t ptr)
{
  int repr[seg_direction_size];

  int is_stack = read_va(repr, ptr.addr);

  size_t off = offset(repr);

  if (is_stack)
  {
    printf("Error try to free stack : %ld\n", ptr.addr);
    return 1;
  }

  if (off > seg_heap[seg_current_pid_index])
  {
    printf("Not allocated memory, offset: %ld heap: %ld\n", off, seg_heap[seg_current_pid_index]);
    return 1;
  }

  if (off > seg_bound_seg_heap[seg_current_pid_index])
  {
    printf("Try to access to memory out of bound, offset: %ld bound: %ld\n", off, seg_bound_seg_heap[seg_current_pid_index]);
    return 1;
  }

  if (seg_virtual_mem[off] != seg_pids[seg_current_pid_index])
  {
    printf("Error: Try to free memory that is not of current proc: %d  Owner: %d\n", seg_pids[seg_current_pid_index], seg_virtual_mem[off]);
    return 1;
  }

  seg_virtual_mem[seg_base[seg_current_pid_index] + off] = -1;
  seg_update_heap();
  return 0;
}

// Agrega un elemento al stack
int m_seg_push(byte val, ptr_t *out)
{
  if (seg_stack[seg_current_pid_index] + 1 > seg_bound_stack[seg_current_pid_index])
  {
    printf("Se lleno el stack, intentando expandirlo");
    if (extend_proc(1, 0))
    {
      printf("Stack Overflow Stack %ld  BoundStack: %ld", seg_stack[seg_current_pid_index], seg_bound_stack[seg_current_pid_index]);
      return 1;
    }
  }

  seg_stack[seg_current_pid_index]++;
  m_write(seg_base[seg_current_pid_index] - seg_stack[seg_current_pid_index], val);
  out->addr = get_va_stack(seg_stack[seg_current_pid_index]);
  return 0;
}

// Quita un elemento del stack
int m_seg_pop(byte *out)
{
  if (seg_stack[seg_current_pid_index] == 0)
  {
    printf("Stack Underflow Stack %ld  BoundStack: %ld", seg_stack[seg_current_pid_index], seg_bound_stack[seg_current_pid_index]);
    return 1;
  }

  *out = m_read(seg_base[seg_current_pid_index] - seg_stack[seg_current_pid_index]);
  seg_stack[seg_current_pid_index]--;
  return 0;
}

// Carga el valor en una dirección determinada
int m_seg_load(addr_t addr, byte *out)
{
  int repr[seg_direction_size];
  int is_stack = read_va(repr, addr);

  if (is_stack)
  {
    size_t off = offset(repr);
    if (off > seg_stack[seg_current_pid_index])
    {
      printf("Try read out of Stack %ld  BoundStack: %ld", seg_stack[seg_current_pid_index], seg_bound_stack[seg_current_pid_index]);
      return 1;
    }
    *out = m_read(seg_base[seg_current_pid_index] - off);
    return 0;
  }

  size_t off = offset(repr);
  if (off > seg_heap[seg_current_pid_index])
  {
    printf("Try read out of alloc, Heap: %ld  BoundHeap: %ld", seg_heap[seg_current_pid_index], seg_bound_seg_heap[seg_current_pid_index]);
    return 1;
  }

  if (off >= seg_bound_seg_heap[seg_current_pid_index])
  {
    printf("Try read out of Boundheap %ld  BoundH: %ld", seg_heap[seg_current_pid_index], seg_bound_seg_heap[seg_current_pid_index]);
    return 1;
  }

  if (seg_virtual_mem[seg_base[seg_current_pid_index] + off] != seg_pids[seg_current_pid_index])
  {
    printf("Memory in free space, Owner: %d  Currentpid : %d  \n", seg_virtual_mem[seg_base[seg_current_pid_index] + off], seg_pids[seg_current_pid_index]);
    return 1;
  }
  *out = m_read(seg_base[seg_current_pid_index] + off);
  return 0;
}

// Almacena un valor en una dirección determinada
int m_seg_store(addr_t addr, byte val)
{
  int repr[seg_direction_size];
  int is_stack = read_va(repr, addr);
  if (is_stack)
  {
    printf("Try to store in stack \n");
    return 1;
  }

  size_t off = offset(repr);

  if (off > seg_heap[seg_current_pid_index])
  {
    printf("Try write out of alloc, Heap: %ld  BoundHeap: %ld", seg_heap[seg_current_pid_index], seg_bound_seg_heap[seg_current_pid_index]);
    return 1;
  }

  if (off >= seg_bound_seg_heap[seg_current_pid_index])
  {
    printf("Try write out of Boundheap %ld  BoundH: %ld", seg_heap[seg_current_pid_index], seg_bound_seg_heap[seg_current_pid_index]);
    return 1;
  }

  if (seg_virtual_mem[seg_base[seg_current_pid_index] + off] != seg_pids[seg_current_pid_index])
  {
    printf("Memory in free space, Owner: %d  Currentpid : %d  \n", seg_virtual_mem[seg_base[seg_current_pid_index] + off], seg_pids[seg_current_pid_index]);
    return 1;
  }

  m_write(seg_base[seg_current_pid_index] + off, val);
  return 0;
}

// Notifica un cambio de contexto al proceso 'next_pid'
void m_seg_on_ctx_switch(process_t process)
{
  set_curr_owner(process.pid);

  seg_current_pid_index = seg_find_pid(process.pid);

  if (seg_current_pid_index < 0)
  {
    is_new = 1;
    seg_current_pid_index = seg_add_pid(process.pid);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_seg_on_end_process(process_t process)
{
  seg_del_pid(process.pid);

  for (size_t i = 0; i < seg_virtual_mem_size; i++)
    if (seg_virtual_mem[i] == process.pid)
    {
      seg_virtual_mem[i] = -1;
      m_unset_owner(i, i);
    }
}
