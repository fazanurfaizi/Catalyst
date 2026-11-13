#include "test_harness.hpp"

#include "engine/asset/importer/ImporterRegistry.hpp"
#include "engine/asset/manager/AssetManagerEditor.hpp"

#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace {

class FakePngImporter : public engine::asset::IAssetImporter {
public:
  std::string_view GetId() const override { return "test.fake_png"; }

  engine::asset::AssetType GetAssetType() const override {
    return engine::asset::AssetType::Texture2D;
  }

  std::vector<std::string_view> GetSupportedExtensions() const override {
    return {".png"};
  }

  std::unique_ptr<engine::asset::ImportSettings>
  GetDefaultSettings() const override {
    return nullptr;
  }

  engine::asset::ImportResult<engine::asset::ImportedAssetDataPtr>
  Import(const engine::asset::ImportRequest &) const override {
    return engine::asset::ImportResult<engine::asset::ImportedAssetDataPtr>::
        Err(engine::asset::ImportError::Make(
            engine::asset::ImportError::Code::UnsupportedFormat,
            "test importer"));
  }
};

} // namespace

void RunAssetManagerEditorTests() {
  namespace fs = std::filesystem;

  const fs::path root =
      fs::temp_directory_path() / "catalyst_assets_test" / "editor";
  fs::remove_all(root);
  fs::create_directories(root / "textures");
  {
    std::ofstream out(root / "textures" / "new.png");
    out << "x";
  }

  engine::asset::ImporterRegistry registry;
  registry.Register(std::make_unique<FakePngImporter>());

  engine::asset::AssetManagerEditor editor(registry, root);

  const auto handle = editor.ImportAsset("textures/new.png", "new_tex");
  CHECK(!handle.IsNull());
  CHECK(editor.IsValid(handle));
  CHECK(editor.IsDirty());

  // Duplikat mengembalikan handle yang sama, bukan record baru.
  const auto again = editor.ImportAsset("textures/new.png", "other");
  CHECK_EQ(again, handle);

  // Path yang tidak ada tidak menambah record.
  const auto missing = editor.ImportAsset("textures/missing.png", "missing");
  CHECK(missing.IsNull());

  // Path escaping ditolak.
  CHECK(editor.ImportAsset("../escape.png", "escape").IsNull());
  CHECK(editor.ImportAsset("/etc/passwd", "abs").IsNull());

  editor.ClearDirty();
  CHECK(!editor.IsDirty());
}
