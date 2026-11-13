#include "engine/asset/importer/shader/ShaderImporter.hpp"
#include "engine/asset/core/ImportResult.hpp"
#include "engine/asset/importer/shader/ShaderImportResult.hpp"

#include <fstream>
#include <ios>
#include <sstream>
#include <string>

namespace engine::asset {

namespace {

bool ReadFile(const std::filesystem::path &path, std::string &out) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;
  std::ostringstream ss;
  ss << in.rdbuf();
  out = ss.str();
  return true;
}

ImportResult<ImportedAssetDataPtr> ImportShader(const ImportRequest &request) {
  std::string stem = request.absolutePath.stem().string();
  if (!stem.ends_with(".vert")) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(
            ImportError::Code::UnsupportedFormat,
            "Shader entry must be named '<name>.vert.glsl'; got '" +
                request.absolutePath.filename().string() + "'")
            .WithPath(request.absolutePath));
  }

  stem.resize(stem.size() - 5);
  if (stem.empty()) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "Shader base name is empty")
            .WithPath(request.absolutePath));
  }

  const std::filesystem::path vertPath = request.absolutePath;
  const std::filesystem::path fragPath =
      request.absolutePath.parent_path() / (stem + ".frag.glsl");

  std::string vertSrc, fragSrc;
  if (!ReadFile(vertPath, vertSrc)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileUnreadable,
                          "Cannot read vertex shader")
            .WithPath(vertPath));
  }
  if (!ReadFile(fragPath, fragSrc)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileUnreadable,
                          "Cannot read fragment shader")
            .WithPath(fragPath));
  }

  if (vertSrc.empty() || fragSrc.empty()) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "Shader source is empty")
            .WithPath(request.absolutePath));
  }

  auto result = std::make_unique<ShaderImportResult>();
  result->vertexSource = std::move(vertSrc);
  result->fragmentSource = std::move(fragSrc);
  result->debugName = stem;
  return ImportResult<ImportedAssetDataPtr>::Ok(std::move(result));
}

} // namespace

ImportResult<ImportedAssetDataPtr>
ShaderImporter::Import(const ImportRequest &request) const {
  return ImportShader(request);
}

} // namespace engine::asset
