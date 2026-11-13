#include "asset/test_harness.hpp"

void RunAtomicFileWriterTests();
void RunGameConfigSerializerTests();

int main() {
  RunAtomicFileWriterTests();
  RunGameConfigSerializerTests();
  return test::Summary("engine_tests");
}
