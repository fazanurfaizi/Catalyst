#include "test_harness.hpp"

void RunAtomicFileWriterTests();
void RunGameConfigSerializerTests();
void RunRegistrySerializerTests();
void RunAssetManagerEditorTests();
void RunAudioEngineTests();

int main() {
  RunAtomicFileWriterTests();
  RunGameConfigSerializerTests();
  RunRegistrySerializerTests();
  RunAssetManagerEditorTests();
  RunAudioEngineTests();
  return test::Summary("engine_tests");
}
