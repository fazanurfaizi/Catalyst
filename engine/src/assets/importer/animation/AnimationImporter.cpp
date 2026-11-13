#include "engine/asset/importer/animation/AnimationImporter.hpp"
#include "engine/asset/importer/animation/AnimationImportResult.hpp"
#include "engine/resources/AnimationAsset.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

using json = nlohmann::json;

namespace engine::asset {

namespace {
/**
 * @brief convert a JSON sequence node to a variant value.
 */
bool ParseValueNode(const json &node,
                    std::variant<float, glm::vec2, glm::vec3, glm::vec4> &out) {
  if (node.is_number()) {
    out = node.get<float>();
    return true;
  }
  if (!node.is_array()) {
    return false;
  }

  for (const auto &v : node) {
    if (!v.is_number())
      return false;
  }

  auto seq = node.get<std::vector<float>>();
  switch (seq.size()) {
  case 1:
    out = node[0].get<float>();
    return true;
  case 2:
    out = glm::vec2(seq[0], seq[1]);
    return true;
  case 3:
    out = glm::vec3(seq[0], seq[1], seq[2]);
    return true;
  case 4:
    out = glm::vec4(seq[0], seq[1], seq[2], seq[3]);
    return true;
  default:
    return false;
  }
}

ImportResult<ImportedAssetDataPtr>
ImportAnimation(const ImportRequest &request) {
  if (!std::filesystem::exists(request.absolutePath)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileNotFound,
                          "Animation file not found")
            .WithPath(request.absolutePath));
  }

  std::ifstream file(request.absolutePath, std::ios::binary);
  if (!file) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileUnreadable,
                          "Animation file unreadable or empty")
            .WithPath(request.absolutePath));
  }

  json doc;
  try {
    file >> doc;
  } catch (const json::exception &e) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent, e.what())
            .WithPath(request.absolutePath));
  }

  if (!doc.is_object() || !doc.contains("Animation") ||
      !doc["Animation"].is_string()) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "Missing or non-scalar 'Animation' root key")
            .WithPath(request.absolutePath));
  }

  auto result = std::make_unique<AnimationImportResult>();
  result->name = doc["Animation"].get<std::string>();

  if (doc.contains("Duration") && doc["Duration"].is_number()) {
    if (!doc["Duration"].is_number()) {
      return ImportResult<ImportedAssetDataPtr>::Err(
          ImportError::Make(ImportError::Code::MalformedContent,
                            "'Duration' must be a number")
              .WithPath(request.absolutePath));
    }
    result->duration = doc["Duration"].get<float>();
    if (result->duration <= 0.0f || !std::isfinite(result->duration)) {
      return ImportResult<ImportedAssetDataPtr>::Err(
          ImportError::Make(ImportError::Code::MalformedContent,
                            "'Duration' must be finite and non-negative")
              .WithPath(request.absolutePath));
    }
  }

  if (doc.contains("Tracks")) {
    if (!doc["Tracks"].is_array()) {
      return ImportResult<ImportedAssetDataPtr>::Err(
          ImportError::Make(ImportError::Code::MalformedContent,
                            "'Tracks' must be an array")
              .WithPath(request.absolutePath));
    }

    const auto &tracksArray = doc["Tracks"];
    for (std::size_t i = 0; i < tracksArray.size(); ++i) {
      const auto &trackNode = tracksArray[i];
      const std::string where = "Track [" + std::to_string(i) + "]";

      if (!trackNode.is_object()) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              where + " is not an object")
                .WithPath(request.absolutePath));
      }

      if (!trackNode.contains("TrackProperty") ||
          !trackNode["TrackProperty"].is_string()) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              where + " missing string 'TrackProperty'")
                .WithPath(request.absolutePath));
      }

      const std::string property =
          trackNode["TrackProperty"].get<std::string>();
      if (resources::StringToTrackProperty.find(property) ==
          resources::StringToTrackProperty.end()) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              where + " unknown TrackProperty '" + property +
                                  "'")
                .WithPath(request.absolutePath));
      }

      ParsedTrack track;
      track.propertyName = property;

      if (trackNode.contains("Keyframes")) {
        if (!trackNode["Keyframes"].is_array()) {
          return ImportResult<ImportedAssetDataPtr>::Err(
              ImportError::Make(ImportError::Code::MalformedContent,
                                "'Keyframes' must be an array")
                  .WithPath(request.absolutePath)
                  .WithLine(static_cast<int>(i)));
        }

        const auto &keyframesArray = trackNode["Keyframes"];
        for (std::size_t j = 0; j < keyframesArray.size(); ++j) {
          const auto &kf = keyframesArray[j];
          const std::string kfWhere =
              where + " keyframe [" + std::to_string(j) + "]";

          if (!kf.is_object() || !kf.contains("Time") ||
              !kf.contains("Value")) {
            return ImportResult<ImportedAssetDataPtr>::Err(
                ImportError::Make(ImportError::Code::MalformedContent,
                                  kfWhere + " missing 'Time' or 'Value'")
                    .WithPath(request.absolutePath));
          }

          if (!kf["Time"].is_number()) {
            return ImportResult<ImportedAssetDataPtr>::Err(
                ImportError::Make(ImportError::Code::MalformedContent,
                                  kfWhere + " 'Time' must be a number")
                    .WithPath(request.absolutePath));
          }

          ParsedKeyframe keyframe;
          keyframe.time = kf["Time"].get<float>();
          if (!std::isfinite(keyframe.time) || keyframe.time < 0.0f) {
            return ImportResult<ImportedAssetDataPtr>::Err(
                ImportError::Make(ImportError::Code::MalformedContent,
                                  kfWhere + " 'Time' must be finite and >= 0")
                    .WithPath(request.absolutePath));
          }

          if (!ParseValueNode(kf["Value"], keyframe.value)) {
            return ImportResult<ImportedAssetDataPtr>::Err(
                ImportError::Make(ImportError::Code::MalformedContent,
                                  "Unsupported keyframe value shape (expected "
                                  "scalar or 1/2/3/4-element array)")
                    .WithPath(request.absolutePath)
                    .WithLine(static_cast<int>(j)));
          }
          track.keyframes.emplace_back(std::move(keyframe));
        }
      }
      result->tracks.emplace_back(std::move(track));
    }
  }

  return ImportResult<ImportedAssetDataPtr>::Ok(std::move(result));
}

} // namespace

ImportResult<ImportedAssetDataPtr>
AnimationImporter::Import(const ImportRequest &request) const {
  return ImportAnimation(request);
}

} // namespace engine::asset
