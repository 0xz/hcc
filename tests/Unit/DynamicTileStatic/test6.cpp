// XFAIL: Linux
// RUN: %hc %s -o %t.out && %t.out

#include <hc.hpp>

#include <iostream>

#define __KERNEL__ __attribute__((amp))
#define __GROUP__ __attribute__((address_space(3)))

template<size_t GRID_SIZE, size_t TILE_SIZE>
bool test() {
  using namespace hc;


  array_view<int, 1> av(GRID_SIZE);
  tiled_extent<1> ex(GRID_SIZE, TILE_SIZE);
  ex.setDynamicGroupSegmentSize(0);
  
  completion_future fut = parallel_for_each(hc::accelerator().get_default_view(),
                    ex,
                    __KERNEL__ [=](tiled_index<1>& tidx) {
    tile_static int lds1[TILE_SIZE];

    // obtain workitem absolute index and workgroup index
    index<1> global = tidx.global;
    index<1> local = tidx.local;

    // fetch the address of a variable in group segment
    __GROUP__ unsigned char* ptr = (__GROUP__ unsigned char*)&lds1[local[0]];

    // fetch the address of the beginning of group segment
    __GROUP__ unsigned char* lds = (__GROUP__ unsigned char*)getLDS(0);

    // calculate the offset and set to the result global array_view
    av(global) = (ptr - lds);
  });

  // wait for kernel to complete
  fut.wait();

  // verify data
  bool ret = true;
  for (int i = 0; i < GRID_SIZE; ++i) {
#if 0
    std::cout << av[i] << " ";
#endif

    if (av[i] != sizeof(int) * (i % TILE_SIZE)) {
      ret = false;
      break;
    }

#if 0
    if ((i + 1) % TILE_SIZE == 0) {
      std::cout << "\n";
    }
#endif
  } 
#if 0
  std::cout << "\n";
#endif
  return ret;
}

int main() {
  bool ret = true;

  ret &= test<1, 1>();
  ret &= test<4, 2>();
  ret &= test<8, 4>();
  ret &= test<64, 16>();
  ret &= test<256, 32>();
  ret &= test<4096, 64>();

  return !(ret == true);
}
