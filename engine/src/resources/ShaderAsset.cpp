#include "engine/resources/ShaderAsset.hpp"

#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::resources {

bool ShaderAsset::EnsureGpu() const {
  if (m_gpu)
    return true;

  if (!cpu) {
    Logger::Error("[ShaderAsset] EnsureGpu: no CPU data");
    return false;
  }

  if (cpu->vertexSource.empty() || cpu->fragmentSource.empty()) {
    Logger::Error("[ShaderAsset] '%s': empty vertex or fragment source",
                  cpu->debugName.c_str());
    return false;
  }

  auto shader = std::make_shared<graphics::Shader>();
  if (!shader->Compile(cpu->vertexSource, cpu->fragmentSource)) {
    Logger::Error("[ShaderAsset] '%s': compile/link failed",
                  cpu->debugName.c_str());
    return false;
  }

  m_gpu = std::move(shader);
  return true;
}

void ShaderAsset::ReleaseGpu() const { m_gpu.reset(); }

} // namespace engine::resources
