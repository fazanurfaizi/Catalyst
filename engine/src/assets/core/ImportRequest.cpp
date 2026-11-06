#include <filesystem>
#include <utility>
namespace engine::asset {

class ImportSettings;

struct ImportRequest {
  ImportRequest() = delete;
  ImportRequest(std::filesystem::path absPath,
                const ImportSettings *s = nullptr)
      : absolutePath(std::move(absPath)), settings(s) {}

  std::filesystem::path absolutePath;
  const ImportSettings *settings = nullptr;
};

} // namespace engine::asset
