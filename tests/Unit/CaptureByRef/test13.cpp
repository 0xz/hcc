
// RUN: %hc %s -o %t.out && %t.out

#include <amp.h>
#include <iostream>
#include <cstdlib>

// added for checking HSA profile
#include <hc.hpp>

// test C++AMP with fine-grained SVM
// requires HSA Full Profile to operate successfully

#define VECTOR_SIZE (1024)

class POD {
public:
  int getFoo() restrict(cpu,amp) { return foo; }
  int getBar() restrict(cpu,amp) { return bar; }
  int getFooCrossBar() restrict(cpu,amp) { return foo * bar; }
  void setFoo(int f) restrict(cpu) { foo = f; }
  void setBar(int b) restrict(cpu) { bar = b; }
private:
  int foo;
  int bar;
};

bool test() {
  using namespace Concurrency;

  int table[VECTOR_SIZE];
  for (int i = 0; i < VECTOR_SIZE; ++i) {
    table[i] = i;
  }

  POD p;
  p.setFoo(rand() % 15 + 1);
  p.setBar(rand() % 15 + 1);

  extent<1> ex(VECTOR_SIZE);
  parallel_for_each(ex, [&](index<1> idx) restrict(amp) {
    // capture array type, and POD type by reference
    // use member function to access POD type
    table[idx[0]] *= (p.getFoo() * p.getBar());
  });

  // verify result
  for (int i = 0; i < VECTOR_SIZE; ++i) {
    if (table[i] != i * (p.getFooCrossBar())) {
      std::cout << "Failed at " << i << std::endl;
      return false;
    }
  }

  std::cout << "Passed" << std::endl;
  return true;
}

int main() {
  bool ret = true;

  // only conduct the test in case we are running on a HSA full profile stack
  hc::accelerator acc;
  if (acc.is_hsa_accelerator() &&
      acc.get_profile() == hc::hcAgentProfileFull) {
    ret &= test();
  }

  return !(ret == true);
}

