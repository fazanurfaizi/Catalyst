#include "test_harness.hpp"

void RunAtomicFileWriterTests();
void RunGameConfigSerializerTests();
void RunRegistrySerializerTests();
void RunAssetManagerEditorTests();

int main() {
  RunAtomicFileWriterTests();
  RunGameConfigSerializerTests();
  RunRegistrySerializerTests();
  RunAssetManagerEditorTests();
  return test::Summary("engine_tests");
}
