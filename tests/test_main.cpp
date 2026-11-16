#include "test_harness.hpp"

void RunAtomicFileWriterTests();
void RunGameConfigSerializerTests();
void RunRegistrySerializerTests();
void RunAssetManagerEditorTests();
void RunAudioEngineTests();
void RunFontImporterTests();
void RunShaderImporterTests();
void RunTexture2DImporterTests();
void RunTextureAtlasImporterTests();
void RunAnimationImporterTests();
void RunSoundImporterTests();

int main() {
  RunAtomicFileWriterTests();
  RunGameConfigSerializerTests();
  RunRegistrySerializerTests();
  RunAssetManagerEditorTests();
  RunAudioEngineTests();
  RunFontImporterTests();
  RunShaderImporterTests();
  RunTexture2DImporterTests();
  RunTextureAtlasImporterTests();
  RunAnimationImporterTests();
  RunSoundImporterTests();
  return test::Summary("engine_tests");
}
