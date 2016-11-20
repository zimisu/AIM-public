#include <proc.h>

struct cpu *get_cpu() {
  int ret;
  asm("mov %%gs:0, %0":"=r"(ret));
  return (struct cpu*)ret;
}