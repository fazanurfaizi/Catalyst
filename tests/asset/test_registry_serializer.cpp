#include "test_harness.hpp"

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/persistence/RegistrySerializer.hpp"

#include <filesystem>
#include <fstream>

void RunRegistrySerializerTests() {
  namespace fs = std::filesystem;
  using engine::asset::AssetHandle;
  using engine::asset::AssetRecord;
  using engine::asset::AssetType;
  using engine::asset::RegistrySerializer;

  const auto fontHandle = AssetHandle::FromString("eb3f88131e45d21c");
  const auto depHandle = AssetHandle::FromString("c9991dd7ab017a59");

  std::vector<AssetRecord> records;
  records.emplace_back(fontHandle, AssetType::Font, "fonts/a.ttf", "font_a");
  records.back().settingsHash = 8396310794483152859ULL;
  records.back().dependencies.push_back(depHandle);
  records.emplace_back(AssetHandle::FromString("aaaa000000000001"),
                       AssetType::Texture2D, "textures/b.png", "");

  const auto json = RegistrySerializer::ToJson(records);
  CHECK(json.is_array());
  CHECK_EQ(json.size(), std::size_t{2});
  CHECK_EQ(json[0]["type"].get<std::string>(), std::string("Font"));
  CHECK_EQ(json[1].contains("name"), false);

  const auto back = RegistrySerializer::FromJson(json);
  CHECK_EQ(back.size(), std::size_t{2});
  if (back.size() == 2) {
    CHECK_EQ(back[0].handle, fontHandle);
    CHECK_EQ(back[0].settingsHash, std::uint64_t{8396310794483152859ULL});
    CHECK_EQ(back[0].dependencies.size(), std::size_t{1});
    CHECK_EQ(back[1].type, AssetType::Texture2D);
    CHECK_EQ(back[1].path, fs::path("textures/b.png"));
  }

  const fs::path root =
      fs::temp_directory_path() / "catalyst_assets_test" / "registry";
  fs::remove_all(root);
  fs::create_directories(root / "fonts");
  {
    std::ofstream out(root / "fonts" / "a.ttf");
    out << "x";
  }
  const auto withRoot = RegistrySerializer::FromJson(json, root);
  CHECK_EQ(withRoot[0].state, engine::asset::AssetRecordState::Valid);
  CHECK_EQ(withRoot[1].state, engine::asset::AssetRecordState::Missing);

  nlohmann::json mixed = nlohmann::json::array();
  mixed.push_back({{"type", "Font"}, {"path", "fonts/a.ttf"}});
  mixed.push_back({{"path", "no_type.ttf"}});
  mixed.push_back({{"type", "Nope"}, {"path", "x"}});
  mixed.push_back({{"type", "Font"}});
  const auto parsed = RegistrySerializer::FromJson(mixed);
  CHECK_EQ(parsed.size(), std::size_t{1});
  if (!parsed.empty()) {
    CHECK(!parsed[0].handle.IsNull());
    CHECK_EQ(parsed[0].handle,
             RegistrySerializer::DeriveHandleFromPath("fonts/a.ttf"));
  }

  std::vector<AssetRecord> emptyPath;
  emptyPath.emplace_back(AssetHandle::FromString("bbbb000000000001"),
                         AssetType::Font, "", "broken");
  CHECK_EQ(RegistrySerializer::ToJson(emptyPath).size(), std::size_t{0});
}
