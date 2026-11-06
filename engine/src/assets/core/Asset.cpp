#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
namespace engine::asset {

class Asset {
public:
  virtual ~Asset() = default;

  /*
   * @brief Runtime type discriminator. Subclass return their own AssetType
   */
  virtual AssetType Type() const = 0;

  /*
   * @brief Identity of this asset. Immutable for the lifetime of the object
   */
  AssetHandle Handle() const noexcept { return m_handle; }

private:
  AssetHandle m_handle;
};

} // namespace engine::asset
