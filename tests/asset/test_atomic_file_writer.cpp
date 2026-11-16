#include "test_harness.hpp"

#include "engine/asset/persistence/AtomicFileWriter.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string ReadAll(const std::filesystem::path &path) {
  std::ifstream in(path, std::ios::binary);
  return std::string(std::istreambuf_iterator<char>(in),
                     std::istreambuf_iterator<char>());
}

} // namespace

void RunAtomicFileWriterTests() {
  namespace fs = std::filesystem;

  const fs::path dir =
      fs::temp_directory_path() / "catalyst_assets_test" / "atomic";
  fs::remove_all(dir);

  const fs::path target = dir / "nested" / "file.txt";

  CHECK(engine::asset::AtomicFileWriter::Write(target, "hello"));
  CHECK(fs::exists(target));
  CHECK_EQ(ReadAll(target), std::string("hello"));
  CHECK(!fs::exists(target.string() + ".tmp"));

  CHECK(engine::asset::AtomicFileWriter::Write(target, "world"));
  CHECK_EQ(ReadAll(target), std::string("world"));
  CHECK(!fs::exists(target.string() + ".tmp"));

  // Target berupa direktori: rename harus gagal dan tidak merusak apa pun.
  CHECK(!engine::asset::AtomicFileWriter::Write(dir, "x"));
  CHECK(fs::is_directory(dir));
}
