#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace engine::rhi {

class Device;
class ShaderStagesImpl;

enum class ShaderStage { Vertex, Fragment };

class ShaderStageSource {
public:
  ShaderStage stage{ShaderStage::Vertex};
  std::string path; // .glsl source path
  std::string entryPoint{"main"};
};

class ShaderStagesCreateInfo {
public:
  std::string name;
  std::vector<ShaderStageSource> stages;
  std::vector<std::pair<std::string, std::string>> defines;
};

class ShaderStages {
public:
  ShaderStages(Device &device, const ShaderStagesCreateInfo &createInfo);
  ~ShaderStages();

  ShaderStages(const ShaderStages &) = delete;
  ShaderStages &operator=(const ShaderStages &) = delete;

  [[nodiscard]] const std::string &Name() const;
  [[nodiscard]] ShaderStagesImpl &GetImpl();

private:
  std::unique_ptr<ShaderStagesImpl> m_impl;
};

} // namespace engine::rhi
