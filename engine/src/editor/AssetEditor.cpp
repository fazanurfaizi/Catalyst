#include "engine/editor/AssetEditor.hpp"

#include "engine/assets/cache/AssetState.hpp"
#include "engine/assets/manager/AssetManagerEditor.hpp"
#include "imgui.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace engine::editor {

namespace {

std::string ToLower(std::string s) {
  for (char &c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return s;
}

bool MatchesFilter(const std::string &needle,
                   const asset::AssetRecord &record) {
  if (needle.empty())
    return true;

  return ToLower(record.name).find(needle) != std::string::npos ||
         ToLower(record.path.generic_string()).find(needle) !=
             std::string::npos;
}

ImVec4 StateColor(asset::AssetLoadState state) {
  switch (state) {
  case asset::AssetLoadState::Uploaded:
    return ImVec4(0.4f, 0.9f, 0.4f, 1.0f);
  case asset::AssetLoadState::Loaded:
    return ImVec4(0.6f, 0.8f, 1.0f, 1.0f);
  case asset::AssetLoadState::Loading:
    return ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
  case asset::AssetLoadState::Failed:
    return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
  case asset::AssetLoadState::Unloaded:
  default:
    return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
  }
}

} // namespace

void DrawAssetBrowser(asset::IAssetManager &assets, AssetBrowserState &state) {
  if (!ImGui::Begin("Assets")) {
    ImGui::End();
    return;
  }

  ImGui::InputTextWithHint("##filter", "filter name/path", state.filter,
                           sizeof(state.filter));

  std::vector<asset::AssetRecord> records = assets.RecordsSnapshot();
  std::sort(records.begin(), records.end(), [](const auto &a, const auto &b) {
    const std::string an = a.name.empty() ? a.path.generic_string() : a.name;
    const std::string bn = b.name.empty() ? b.path.generic_string() : b.name;
    return an < bn;
  });

  const std::string filter = ToLower(state.filter);

  if (ImGui::BeginTable("##assets", 5,
                        ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_SizingStretchProp)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("State");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("Actions");
    ImGui::TableHeadersRow();

    for (const auto &record : records) {
      if (!MatchesFilter(filter, record))
        continue;

      ImGui::PushID(record.handle.ToString().c_str());
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);

      std::string label =
          record.name.empty() ? record.path.filename().string() : record.name;
      if (record.builtin)
        label += "  [builtin]";

      if (ImGui::Selectable(label.c_str(), state.selected == record.handle)) {
        state.selected = record.handle;
      }

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(asset::AssetTypeToString(record.type));

      const asset::AssetLoadState loadState = assets.State(record.handle);

      ImGui::TableSetColumnIndex(2);
      ImGui::TextColored(StateColor(loadState), "%s",
                         asset::AssetLoadStateToString(loadState));

      ImGui::TableSetColumnIndex(3);
      ImGui::TextUnformatted(record.path.generic_string().c_str());

      ImGui::TableSetColumnIndex(4);

      if (record.builtin) {
        ImGui::TextDisabled("compiled-in");
      } else {
        ImGui::BeginDisabled(loadState == asset::AssetLoadState::Loading);
        if (ImGui::SmallButton("Reimport"))
          assets.Reimport(record.handle);
        ImGui::EndDisabled();
      }

      ImGui::PopID();
    }

    ImGui::EndTable();
  }

  if (!state.selected.IsNull()) {
    if (const auto record = assets.RecordCopy(state.selected); record) {
      ImGui::SeparatorText("Details");
      ImGui::Text("Name:   %s",
                  record->name.empty() ? "(unnamed)" : record->name.c_str());
      ImGui::Text("Path:   %s", record->path.generic_string().c_str());
      ImGui::Text("Handle: %s", record->handle.ToString().c_str());
      ImGui::Text("Type:   %s", asset::AssetTypeToString(record->type));
      ImGui::Text("Hash:   %llu",
                  static_cast<unsigned long long>(record->settingsHash));

      if (record->builtin) {
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f),
                           "Dibawa engine; tidak punya file proyek.");
      }

      if (!record->dependencies.empty()) {
        ImGui::TextUnformatted("Depends on:");
        for (const auto &dep : record->dependencies)
          ImGui::BulletText("%s", dep.ToString().c_str());
      }

      if (const auto error = assets.LastError(state.selected); error) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s",
                           error->GetMessage().c_str());
        if (!error->GetPath().empty()) {
          ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.4f, 1.0f), "%s",
                             error->GetPath().generic_string().c_str());
        }
      }
    }
  }

  if (auto *editor = dynamic_cast<asset::AssetManagerEditor *>(&assets)) {
    ImGui::SeparatorText("Import");
    ImGui::InputText("Path", state.importPath, sizeof(state.importPath));
    ImGui::InputText("Name (opt)", state.importName, sizeof(state.importName));

    if (ImGui::Button("Import")) {
      const asset::AssetHandle handle =
          editor->ImportAsset(state.importPath, state.importName);
      if (!handle.IsNull())
        state.selected = handle;
    }
  }

  ImGui::End();
}

} // namespace engine::editor
