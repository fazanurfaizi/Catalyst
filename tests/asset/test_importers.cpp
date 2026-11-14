#include "test_harness.hpp"

#include "engine/asset/core/ImportRequest.hpp"
#include "engine/asset/core/ImportResult.hpp"
#include "engine/asset/core/ImportedAssetData.hpp"
#include "engine/asset/importer/animation/AnimationImporter.hpp"
#include "engine/asset/importer/font/FontImporter.hpp"
#include "engine/asset/importer/shader/ShaderImporter.hpp"
#include "engine/asset/importer/sound/SoundImporter.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImporter.hpp"
#include "engine/asset/importer/texture2d/Texture2DImporter.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string_view>

namespace {

namespace fs = std::filesystem;
using engine::asset::ImportedAssetDataPtr;
using engine::asset::ImportError;
using engine::asset::ImportRequest;
using engine::asset::ImportResult;

const char *kSourceDir = CATALYST_SOURCE_DIR;

fs::path TempDir(const char *name) {
  const fs::path dir =
      fs::temp_directory_path() / "catalyst_assets_test" / "importers" / name;
  fs::remove_all(dir);
  fs::create_directories(dir);
  return dir;
}

void WriteText(const fs::path &path, std::string_view text) {
  fs::create_directories(path.parent_path());
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out.write(text.data(), static_cast<std::streamsize>(text.size()));
}

bool WasError(const ImportResult<ImportedAssetDataPtr> &r,
              ImportError::Code code) {
  return r.IsError() && r.Error().GetCode() == code;
}

void Put16(std::ofstream &out, std::uint16_t val) {
  const char buffer[2] = {
      static_cast<char>(val & 0xFF),
      static_cast<char>((val >> 8) & 0xFF),
  };
  out.write(buffer, 2);
}

void Put32(std::ofstream &out, std::uint32_t val) {
  const char buffer[4] = {
      static_cast<char>(val & 0xFF),
      static_cast<char>((val >> 8) & 0xFF),
      static_cast<char>((val >> 16) & 0xFF),
      static_cast<char>((val >> 24) & 0xFF),
  };
  out.write(buffer, 4);
}

fs::path WriteSilentWav(const fs::path &path) {
  constexpr std::uint32_t kFrames = 100;
  constexpr std::uint16_t kChannels = 1;
  constexpr std::uint32_t kRate = 44100;
  constexpr std::uint16_t kBits = 16;
  const std::uint32_t dataBytes = kFrames * kChannels * (kBits / 8);

  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out.write("RIFF", 4);
  Put32(out, 36 + dataBytes);
  out.write("WAVE", 4);
  out.write("fmt ", 4);
  Put32(out, 16);
  Put16(out, 1);
  Put16(out, kChannels);
  Put32(out, kRate);
  Put32(out, kRate * kChannels * (kBits / 8));
  Put16(out, static_cast<std::uint16_t>(kChannels * (kBits / 8)));
  Put16(out, kBits);
  out.write("data", 4);
  Put32(out, dataBytes);
  for (std::uint32_t i = 0; i < dataBytes; ++i) {
    out.put(0);
  }
  return path;
}

} // namespace

void RunFontImporterTests() {
  engine::asset::FontImporter importer;

  const fs::path valid =
      fs::path(kSourceDir) / "assets" / "fonts" / "Roboto-Regular.ttf";
  const auto ok = importer.Import(ImportRequest(valid));
  CHECK(ok.IsOk());
  if (ok.IsOk()) {
    CHECK(ok.Value()->GetAssetType() != engine::asset::AssetType::None);
  }

  const auto dir = TempDir("font");
  const auto bad = dir / "broken.ttf";
  WriteText(bad, "this is not a TTF");

  CHECK(WasError(importer.Import(ImportRequest(bad)),
                 ImportError::Code::MalformedContent));
  CHECK(WasError(importer.Import(ImportRequest(dir / "missing.ttf")),
                 ImportError::Code::FileUnreadable));
}

void RunShaderImporterTests() {
  engine::asset::ShaderImporter importer;
  const auto dir = TempDir("shader");

  WriteText(dir / "basic.vert.glsl", "#version 330\nvoid main(){}\n");
  WriteText(dir / "basic.frag.glsl",
            "#version 330\nout vec4 c;\nvoid main(){c=vec4(1);}\n");

  CHECK(importer.Import(ImportRequest(dir / "basic.vert.glsl")).IsOk());

  CHECK(WasError(importer.Import(ImportRequest(dir / "basic.frag.glsl")),
                 ImportError::Code::UnsupportedFormat));

  WriteText(dir / "lone.vert.glsl", "#version 330\nvoid main(){}\n");
  CHECK(WasError(importer.Import(ImportRequest(dir / "lone.vert.glsl")),
                 ImportError::Code::FileUnreadable));
}

void RunTexture2DImporterTests() {
  engine::asset::Texture2DImporter importer;

  const fs::path valid =
      fs::path(kSourceDir) / "assets" / "textures" / "tower.png";
  const auto ok = importer.Import(ImportRequest(valid));
  CHECK(ok.IsOk());
  if (ok.IsOk()) {
    CHECK(ok.Value()->GetAssetType() == engine::asset::AssetType::Texture2D);
  }

  const auto dir = TempDir("texture");
  const auto bad = dir / "bad.png";
  WriteText(bad, "not an image");
  CHECK(WasError(importer.Import(ImportRequest(bad)),
                 ImportError::Code::DecodeFailed));

  CHECK(WasError(importer.Import(ImportRequest(dir / "missing.png")),
                 ImportError::Code::FileNotFound));
}

void RunTextureAtlasImporterTests() {
  engine::asset::TextureAtlasImporter importer;
  const auto dir = TempDir("atlas");

  fs::create_directories(dir / "sprites");
  fs::copy_file(fs::path(kSourceDir) / "assets" / "textures" / "tower.png",
                dir / "sprites" / "tower.png",
                fs::copy_options::overwrite_existing);

  const auto manifest = dir / "atlas.atlas";
  WriteText(manifest, R"({"sources":[{"type":"directory","path":"sprites"}]})");

  const auto ok = importer.Import(ImportRequest(manifest));
  CHECK(ok.IsOk());

  WriteText(dir / "malformed.atlas", "{ not json");
  CHECK(WasError(importer.Import(ImportRequest(dir / "malformed.atlas")),
                 ImportError::Code::MalformedContent));

  WriteText(dir / "nosources.atlas", R"({"sources":42})");
  CHECK(WasError(importer.Import(ImportRequest(dir / "nosources.atlas")),
                 ImportError::Code::MalformedContent));

  WriteText(dir / "unknown.atlas",
            R"({"sources":[{"type":"video","path":"x"}]})");
  CHECK(WasError(importer.Import(ImportRequest(dir / "unknown.atlas")),
                 ImportError::Code::MalformedContent));

  WriteText(dir / "traversal.atlas",
            R"({"sources":[{"type":"directory","path":"../../"}]})");
  CHECK(WasError(importer.Import(ImportRequest(dir / "traversal.atlas")),
                 ImportError::Code::MalformedContent));

  WriteText(dir / "notobject.atlas", R"({"sources":[42]})");
  CHECK(WasError(importer.Import(ImportRequest(dir / "notobject.atlas")),
                 ImportError::Code::MalformedContent));
}

void RunAnimationImporterTests() {
  engine::asset::AnimationImporter importer;
  const auto dir = TempDir("animation");

  WriteText(dir / "ok.anim", R"({
    "Animation": "hero_walk",
    "Duration": 1.0,
    "Tracks": [
      { 
        "TrackProperty": "Position",
        "Keyframes": [
          {"Time": 0.0, "Value": [0.0, 1.0]},
          {"Time": 0.5, "Value": [1.0, 2.0]}
        ] 
      }
    ]
  })");
  CHECK(importer.Import(ImportRequest(dir / "ok.anim")).IsOk());

  WriteText(dir / "unknown_track.anim", R"({
    "Animation": "bad",
    "Duration": 1.0,
    "Tracks": [ { "TrackProperty": "Explode" } ]
  })");
  CHECK(WasError(importer.Import(ImportRequest(dir / "unknown_track.anim")),
                 ImportError::Code::MalformedContent));

  WriteText(dir / "negative.anim", R"({
    "Animation": "bad",
    "Duration": -1.0,
    "Tracks": []
  })");
  CHECK(WasError(importer.Import(ImportRequest(dir / "negative.anim")),
                 ImportError::Code::MalformedContent));

  WriteText(dir / "bad_time.anim", R"({
    "Animation": "bad",
    "Duration": 1.0,
    "Tracks": [ { "TrackProperty": "Position",
                  "Keyframes": [ {"Time": "soon", "Value": 1.0} ] } ]
  })");
  CHECK(WasError(importer.Import(ImportRequest(dir / "bad_time.anim")),
                 ImportError::Code::MalformedContent));

  WriteText(dir / "bad_value.anim", R"({
    "Animation": "bad",
    "Duration": 1.0,
    "Tracks": [ { "TrackProperty": "Position",
                  "Keyframes": [ {"Time": 0.0, "Value": ["x"]} ] } ]
  })");
  CHECK(WasError(importer.Import(ImportRequest(dir / "bad_value.anim")),
                 ImportError::Code::MalformedContent));
}

void RunSoundImporterTests() {
  engine::asset::SoundImporter importer;
  const auto dir = TempDir("sound");

  const auto wav = WriteSilentWav(dir / "silence.wav");
  const auto ok = importer.Import(ImportRequest(wav));
  CHECK(ok.IsOk());
  if (ok.IsOk()) {
    CHECK(ok.Value()->GetAssetType() != engine::asset::AssetType::None);
  }

  const auto bad = dir / "broken.wav";
  WriteText(bad, "definitely not audio");
  CHECK(WasError(importer.Import(ImportRequest(bad)),
                 ImportError::Code::DecodeFailed));

  CHECK(WasError(importer.Import(ImportRequest(dir / "missing.wav")),
                 ImportError::Code::FileNotFound));
}
