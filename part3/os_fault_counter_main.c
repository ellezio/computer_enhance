#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "metrics.c"

int main(int argc, char **argv) {
  if (argc == 2) {
    uint64_t page_size = 4096;
    uint64_t page_count = atol(argv[1]);
    uint64_t total_size = page_size * page_count;

    printf("Page Count, Touch Count, Fault Count, Extra Faults\n");

    for (uint64_t touch_count = 0; touch_count < page_count; ++touch_count) {
      uint64_t touch_size = touch_count * page_size;
      uint8_t *data = mmap(NULL, touch_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (data) {
        uint64_t start_fault_count = read_os_page_fault_count();
        for (uint64_t idx = 0; idx < touch_size; ++idx) {
          data[idx] = (uint8_t)idx;
        }
        uint64_t end_fault_count = read_os_page_fault_count();

        uint64_t fault_count = end_fault_count - start_fault_count;

        printf("%lu, %lu, %lu, %lu\n", page_count, touch_count, fault_count,
               (fault_count - touch_count));
      } else {
        fprintf(stderr, "ERROR: unable to allocate memory\n");
      }

      munmap(data, touch_size);
    }
  } else {
    fprintf(stderr, "Usage: %s [page count]\n", argv[0]);
  }
}
