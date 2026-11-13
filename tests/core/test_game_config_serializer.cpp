#include "test_harness.hpp"

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/core/GameConfigSerializer.hpp"

#include <filesystem>
#include <fstream>

void RunGameConfigSerializerTests() {
  namespace fs = std::filesystem;
  using engine::asset::AssetHandle;
  using engine::asset::AssetRecord;
  using engine::asset::AssetRecordState;
  using engine::asset::AssetType;
  using engine::core::GameConfig;
  using engine::core::GameConfigSerializer;

  const fs::path dir =
      fs::temp_directory_path() / "catalyst_assets_test" / "config";
  fs::remove_all(dir);
  fs::create_directories(dir / "assets" / "fonts");

  {
    std::ofstream out(dir / "assets" / "fonts" / "foo.ttf");
    out << "x";
  }

  GameConfig cfg;
  cfg.version = 1;
  cfg.projectName = "test-project";
  cfg.root = dir / "assets";

  const auto fontHandle = AssetHandle::FromString("eb3f88131e45d21c");
  const auto depHandle = AssetHandle::FromString("c9991dd7ab017a59");
  CHECK(!fontHandle.IsNull());
  CHECK(!depHandle.IsNull());

  AssetRecord font(fontHandle, AssetType::Font, "fonts/foo.ttf", "main_font");
  font.settingsHash = 8396310794483152859ULL;
  font.dependencies.push_back(depHandle);
  cfg.assets.push_back(font);

  AssetRecord missing(AssetHandle::FromString("1111111111111111"),
                      AssetType::Sound, "audio/none.mp3", "missing_sfx");
  cfg.assets.push_back(missing);

  const fs::path file = dir / "game.json";

  CHECK(GameConfigSerializer::Save(cfg, file));
  CHECK(fs::exists(file));

  auto loaded = GameConfigSerializer::Load(file);
  CHECK(loaded.has_value());
  if (!loaded)
    return;

  CHECK_EQ(loaded->version, 1);
  CHECK_EQ(loaded->projectName, std::string("test-project"));
  CHECK_EQ(loaded->root, fs::absolute(cfg.root));
  CHECK_EQ(loaded->assets.size(), std::size_t{2});

  const auto &l0 = loaded->assets[0];
  CHECK_EQ(l0.handle, fontHandle);
  CHECK_EQ(l0.type, AssetType::Font);
  CHECK_EQ(l0.path, fs::path("fonts/foo.ttf"));
  CHECK_EQ(l0.name, std::string("main_font"));
  CHECK_EQ(l0.settingsHash, std::uint64_t{8396310794483152859ULL});
  CHECK_EQ(l0.dependencies.size(), std::size_t{1});
  if (l0.dependencies.size() == 1)
    CHECK_EQ(l0.dependencies[0], depHandle);
  CHECK_EQ(l0.state, AssetRecordState::Valid);

  const auto &l1 = loaded->assets[1];
  CHECK_EQ(l1.state, AssetRecordState::Missing);

  // Record dengan path kosong harus dilewati, bukan menghasilkan entri rusak.
  GameConfig bad = cfg;
  bad.assets.push_back(AssetRecord(AssetHandle::FromString("2222222222222222"),
                                   AssetType::Texture2D, "", "empty"));
  const fs::path badFile = dir / "bad.json";
  CHECK(GameConfigSerializer::Save(bad, badFile));

  auto badLoaded = GameConfigSerializer::Load(badFile);
  CHECK(badLoaded.has_value());
  if (badLoaded)
    CHECK_EQ(badLoaded->assets.size(), std::size_t{2});

  // Versi tidak dikenal harus ditolak.
  {
    std::ofstream out(dir / "v2.json");
    out << R"({"version":2,"assets":[]})";
  }
  CHECK(!GameConfigSerializer::Load(dir / "v2.json").has_value());

  // Tanpa array assets harus ditolak.
  {
    std::ofstream out(dir / "noassets.json");
    out << R"({"version":1})";
  }
  CHECK(!GameConfigSerializer::Load(dir / "noassets.json").has_value());
}
