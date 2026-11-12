#include "game/states/PlayingState.hpp"

#include "engine/audio/MusicPlayer.hpp"
#include "engine/components/AudioSource.hpp"
#include "engine/components/Light.hpp"
#include "engine/components/TextComponent.hpp"
#include "engine/core/Game.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/math/Color.hpp"
#include "engine/resources/SoundAsset.hpp"
#include "engine/resources/TextureAtlasAsset.hpp"
#include "engine/scene/SceneSerializer.hpp"
#include "engine/systems/AudioSystem.hpp"
#include "engine/systems/TextRenderSystem.hpp"
#include "engine/utils/Logger.hpp"

#include "engine/components/Animator.hpp"
#include "engine/components/Camera2D.hpp"
#include "engine/components/RenderLayer.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"
#include "engine/components/Velocity.hpp"

#include "engine/systems/AnimationSystem.hpp"
#include "engine/systems/CameraFollowSystem.hpp"
#include "engine/systems/MovementSystem.hpp"
#include "engine/systems/ParticleSystem.hpp"

#include <cmath>
#include <cstdlib>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <string>

namespace {
void DrawWorldGrid(engine::graphics::Renderer &renderer,
                   const engine::math::Rect &bounds,
                   const engine::resources::TextureAtlasAsset &atlas) {
  auto *region = atlas.GetRegion("solidWhite");
  if (!region)
    return;

  const int x0 = static_cast<int>(std::floor(bounds.Left()));
  const int x1 = static_cast<int>(std::ceil(bounds.Right()));
  const int y0 = static_cast<int>(std::floor(bounds.Top()));
  const int y1 = static_cast<int>(std::ceil(bounds.Bottom()));

  constexpr int kMaxCells = 4096;
  if ((x1 - x0) * (y1 - y0) > kMaxCells)
    return;

  const engine::math::Color evenCell{0.13f, 0.17f, 0.24f, 1.0f};
  const engine::math::Color oddCell{0.16f, 0.20f, 0.28f, 1.0f};
  const engine::math::Color axisCell{0.20f, 0.24f, 0.34f, 1.0f};
  const auto *tex = &atlas.GetTexture();

  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      engine::math::Color color =
          (x == 0 || y == 0) ? axisCell : (((x + y) & 1) ? oddCell : evenCell);

      renderer.Draw(
          {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f},
          {1.0f, 1.0f}, 0.0f, color, tex, region->uvMin, region->uvMax,
          static_cast<int>(engine::components::RenderLayer::Ground));
    }
  }
}
} // namespace

void PlayingState::ResolveHandles(engine::core::Game &game) {
  auto *assets = game.Assets();
  if (!assets) {
    engine::Logger::Error("[PlayingState] no AssetManager");
    return;
  }

  m_fontHandle = assets->FindByName("main_font");
  m_atlasHandle = assets->FindByName("main_atlas");
  m_bgmHandle = assets->FindByName("main_bgm");
  m_heroAttackSfxHandle = assets->FindByName("hero_attack_sfx");

  if (m_atlasHandle.IsNull())
    engine::Logger::Error(
        "[PlayingState] 'main_atlas' not found — render will be broken");
  if (m_fontHandle.IsNull())
    engine::Logger::Warn("[PlayingState] 'main_font' not found");
  if (m_bgmHandle.IsNull())
    engine::Logger::Warn("[PlayingState] 'main_bgm' not found");
  if (m_heroAttackSfxHandle.IsNull())
    engine::Logger::Warn("[PlayingState] 'hero_attack_sfx' not found");
}

void PlayingState::BuildAnimations() {
  if (!m_atlasCache) {
    engine::Logger::Warn("[PlayingState] BuildAnimations: no atlas cache");
    return;
  }
  m_animations = engine::animation::AnimationLibrary{};
  m_animations.AddStrip("hero_idle", *m_atlasCache, "hero_idle", 10, 0.15f,
                        true);
  m_animations.AddStrip("hero_attack", *m_atlasCache, "hero_attack", 10, 0.08f,
                        false);
  m_animations.AddStrip("hero_move", *m_atlasCache, "hero_move", 10, 0.10f,
                        true);
  m_animations.AddStrip("coin_spin", *m_atlasCache, "coin", 6, 0.08f, true);
}

void PlayingState::OnEnter(engine::core::Game &game) {
  engine::Logger::Info("[PlayingState] OnEnter");

  auto *assets = game.Assets();
  if (!assets) {
    engine::Logger::Error("[PlayingState] AssetManager missing — aborting");
    return;
  }

  ResolveHandles(game);

  for (auto h :
       {m_fontHandle, m_atlasHandle, m_bgmHandle, m_heroAttackSfxHandle}) {
    if (!h.IsNull())
      assets->PreloadCpu(h);
  }

  m_atlasCache =
      assets->Acquire<engine::resources::TextureAtlasAsset>(m_atlasHandle);

  if (!m_atlasCache) {
    engine::Logger::Error(
        "[PlayingState] atlas GPU not ready — scene render will fail");
    return;
  }

  if (!m_bgmHandle.IsNull()) {
    if (auto bgm =
            assets->Acquire<engine::resources::SoundAsset>(m_bgmHandle)) {
      m_music = std::make_unique<engine::audio::MusicPlayer>();
      m_music->Load(bgm->sourcePath);
      m_music->SetLooping(true);
      m_music->SetVolume(0.4f);
      m_music->Play();
    }
  }

  m_lastReloadGen = assets->GetReloadGeneration();

  game.GetWindow().ClearWindow();
  game.GetRenderer().SetAmbientLight({0.03f, 0.04f, 0.25f});

  const char *kScenePath = "scene.json";
  if (!engine::scene::SceneSerializer::Load(m_scene, kScenePath)) {
    engine::Logger::Error("[PlayingState] failed to load scene '%s'",
                          kScenePath);
    return;
  }

  auto &reg = m_scene.Registry();

  for (auto name : {"MainCamera", "Hero", "Tower", "Enemy", "Coin", "Torch",
                    "GoldText", "WaveText", "LivesText"}) {
    if (!m_scene.FindEntity(name))
      engine::Logger::Warn("[PlayingState] scene.json missing entity '%s'",
                           name);
  }

  auto camera = m_scene.FindEntity("MainCamera");
  auto hero = m_scene.FindEntity("Hero");
  auto coin = m_scene.FindEntity("Coin");

  BuildAnimations();

  if (hero && reg.IsAlive(*hero)) {
    if (auto *a = reg.TryGet<engine::components::Animator>(*hero))
      engine::components::PlayAnimation(*a, m_atlasHandle, "hero_idle");
  }
  if (coin && reg.IsAlive(*coin)) {
    if (auto *a = reg.TryGet<engine::components::Animator>(*coin))
      engine::components::PlayAnimation(*a, m_atlasHandle, "coin_spin");
  }

  for (auto name : {"Hero", "Tower", "Enemy", "Coin", "Torch"}) {
    auto e = m_scene.FindEntity(name);
    if (!e)
      continue;
    if (auto *s = reg.TryGet<engine::components::Sprite>(*e);
        s && !s->regionName.empty()) {
      ApplyRegion(*e, s->regionName);
    }
  }

  for (auto name : {"GoldText", "WaveText", "LivesText"}) {
    auto e = m_scene.FindEntity(name);
    if (!e)
      continue;
    if (auto *t = reg.TryGet<engine::components::TextComponent>(*e)) {
      t->font = m_fontHandle;
    }
  }

  if (hero && reg.IsAlive(*hero)) {
    if (auto *s = reg.TryGet<engine::components::AudioSource>(*hero)) {
      s->clip = m_heroAttackSfxHandle;
    }
  }

  if (camera && reg.IsAlive(*camera)) {
    if (auto *cam = reg.TryGet<engine::components::Camera2D>(*camera)) {
      if (hero && reg.IsAlive(*hero))
        cam->followTarget = *hero;
      cam->active = true;
    }
  }
}

void PlayingState::OnUpdate(engine::core::Game &game, double dt) {
  auto *assets = game.Assets();
  if (!assets)
    return;

  if (assets->GetReloadGeneration() != m_lastReloadGen) {
    m_lastReloadGen = assets->GetReloadGeneration();
    ReloadAssets(game);
  }

  auto &reg = m_scene.Registry();
  auto camE = m_scene.FindEntity("MainCamera");
  if (!camE)
    return;
  auto *camera = reg.TryGet<engine::components::Camera2D>(*camE);
  if (!camera)
    return;

  auto &in = game.Input();
  const float speed = 5.0f * static_cast<float>(dt);

  if (in.IsKeyDown(engine::input::Key::W))
    camera->Move({0.0f, speed});
  if (in.IsKeyDown(engine::input::Key::S))
    camera->Move({0.0f, -speed});
  if (in.IsKeyDown(engine::input::Key::A))
    camera->Move({-speed, 0.0f});
  if (in.IsKeyDown(engine::input::Key::D))
    camera->Move({speed, 0.0f});
  if (in.IsKeyDown(engine::input::Key::Q))
    camera->RotateBy(+0.5f * static_cast<float>(dt));
  if (in.IsKeyDown(engine::input::Key::E))
    camera->RotateBy(-0.5f * static_cast<float>(dt));

  engine::systems::AudioSystem(reg, *assets);
}

void PlayingState::OnFixedUpdate(engine::core::Game &game, double dt) {
  auto &reg = m_scene.Registry();

  if (auto hero = m_scene.FindEntity("Hero"); hero && reg.IsAlive(*hero)) {
    auto *a = reg.TryGet<engine::components::Animator>(*hero);
    auto *t = reg.TryGet<engine::components::Transform>(*hero);
    auto *v = reg.TryGet<engine::components::Velocity>(*hero);
    if (a && t && v) {
      if (a->finished && a->clipName == "hero_attack")
        engine::components::PlayAnimation(*a, m_atlasHandle, "hero_idle");

      if (!v->ArrivedAtTarget(t->position)) {
        if (a->clipName != "hero_attack" && a->clipName != "hero_move")
          engine::components::PlayAnimation(*a, m_atlasHandle, "hero_move");
      } else {
        t->position = v->targetPosition;
        if (a->clipName != "hero_attack" && a->clipName != "hero_idle")
          engine::components::PlayAnimation(*a, m_atlasHandle, "hero_idle");
      }
    }
  }

  if (auto enemy = m_scene.FindEntity("Enemy"); enemy && reg.IsAlive(*enemy)) {
    if (auto *t = reg.TryGet<engine::components::Transform>(*enemy)) {
      const float angle = static_cast<float>(game.ElapsedTime());
      t->position = {std::cos(angle) * 6.0f, std::sin(angle) * 4.0f};
      t->rotation = angle;
    }
  }

  if (auto torch = m_scene.FindEntity("Torch"); torch && reg.IsAlive(*torch)) {
    if (auto *light = reg.TryGet<engine::components::PointLight>(*torch)) {
      const float t = static_cast<float>(game.ElapsedTime());
      const float flicker =
          0.80f + 0.15f * std::sin(t * 12.7f) + 0.05f * std::sin(t * 31.3f);
      light->diffuse = glm::vec3{1.00f, 0.65f, 0.25f} * flicker;
    }
  }

  engine::systems::AnimationSystem(reg, m_animations, static_cast<float>(dt));
  engine::systems::ParticleSystem(reg, static_cast<float>(dt));
  engine::systems::MoveTowardTargetSystem(reg, static_cast<float>(dt));
  reg.FlushDeferred();
}

void PlayingState::OnLateUpdate(engine::core::Game &game, double dt) {
  (void)game;
  engine::systems::CameraFollowSystem(m_scene.Registry(), dt);
}

void PlayingState::OnRender(engine::core::Game &game) {
  auto &renderer = game.GetRenderer();
  auto &reg = m_scene.Registry();

  auto camE = m_scene.FindEntity("MainCamera");
  if (!camE)
    return;
  auto *camera = reg.TryGet<engine::components::Camera2D>(*camE);
  if (!camera)
    return;
  if (!m_atlasCache)
    return;

  const float aspect = renderer.AspectRatio();
  const glm::mat4 vp = camera->ViewProjectionMatrix(aspect);
  const engine::math::Rect bounds = camera->VisibleWorldBounds(aspect);

  renderer.BeginFrame(vp, bounds);
  DrawWorldGrid(renderer, bounds, *m_atlasCache);
  renderer.RenderSprites(m_scene, game.Assets());
  renderer.RenderLights(m_scene);
  renderer.EndFrame();

  const glm::mat4 uiProjection =
      game.Display().UiProjection() / game.Display().AspectRatio();
  engine::systems::TextRenderSystem(reg, *game.Assets(), m_textRenderer,
                                    uiProjection);
}

void PlayingState::OnKey(engine::core::Game &game, engine::input::Key key,
                         int scancode, engine::input::KeyAction action,
                         int mods) {
  (void)scancode;
  (void)mods;
  if (action != engine::input::KeyAction::Press)
    return;

  if (key == engine::input::Key::Escape)
    game.GetWindow().RequestClose();
  if (key == engine::input::Key::F11)
    game.GetWindow().ToggleFullscreen();
  if (key == engine::input::Key::Space)
    engine::Logger::Info("Space tapped");
}

void PlayingState::OnScroll(engine::core::Game &game, double dx, double dy) {
  (void)dx;
  auto &reg = m_scene.Registry();
  auto camE = m_scene.FindEntity("MainCamera");
  if (!camE)
    return;
  auto &camera = reg.Get<engine::components::Camera2D>(*camE);

  const auto fbSize = game.Display().FramebufferSize();
  auto cursor = game.Input().CursorPos();
  glm::vec2 cursorFb = game.Display().CursorToFramebuffer(
      {static_cast<float>(cursor.x), static_cast<float>(cursor.y)});

  const auto before = camera.ScreenToWorld(cursorFb, fbSize);
  camera.ZoomBy(dy > 0 ? 1.1f : 1.0f / 1.1f);
  const auto after = camera.ScreenToWorld(cursorFb, fbSize);
  camera.Move(before - after);
}

void PlayingState::OnMouseButton(engine::core::Game &game,
                                 engine::input::MouseButton button,
                                 engine::input::KeyAction action, int mods) {
  (void)mods;
  if (action != engine::input::KeyAction::Press)
    return;

  auto *assets = game.Assets();
  if (!assets)
    return;

  auto &reg = m_scene.Registry();
  auto camE = m_scene.FindEntity("MainCamera");
  auto heroE = m_scene.FindEntity("Hero");
  if (!camE || !heroE)
    return;

  auto &camera = reg.Get<engine::components::Camera2D>(*camE);
  auto &in = game.Input();

  const auto fbSize = game.Display().FramebufferSize();
  auto cursor = in.CursorPos();
  glm::vec2 cursorFb = game.Display().CursorToFramebuffer(
      {static_cast<float>(cursor.x), static_cast<float>(cursor.y)});
  glm::vec2 worldPos = camera.ScreenToWorld(cursorFb, fbSize);

  if (button == engine::input::MouseButton::Left) {
    if (auto *anim = reg.TryGet<engine::components::Animator>(*heroE)) {
      if (anim->clipName == "hero_idle") {
        engine::components::PlayAnimation(*anim, m_atlasHandle, "hero_attack");
        engine::systems::PlayAudioNow(reg, *assets, *heroE);
      }
    }
  }

  if (button == engine::input::MouseButton::Right) {
    if (auto *v = reg.TryGet<engine::components::Velocity>(*heroE)) {
      v->targetPosition = worldPos;
    }
  }
}

void PlayingState::OnMouseMove(engine::core::Game &game, double x, double y) {
  (void)game;
  (void)x;
  (void)y;
}

void PlayingState::OnResize(engine::core::Game &game, int w, int h) {
  (void)w;
  (void)h;
  const auto fb = game.Display().FramebufferSize();
  game.GetRenderer().Resize(fb.x, fb.y);
}

void PlayingState::ReloadAssets(engine::core::Game &game) {
  auto *assets = game.Assets();
  if (!assets)
    return;

  auto newAtlas =
      assets->Acquire<engine::resources::TextureAtlasAsset>(m_atlasHandle);
  if (!newAtlas) {
    engine::Logger::Warn(
        "[PlayingState] reload: atlas unavailable — keeping previous");
    return;
  }
  m_atlasCache = std::move(newAtlas);

  BuildAnimations();

  for (auto name : {"Hero", "Tower", "Enemy", "Coin", "Torch"}) {
    auto e = m_scene.FindEntity(name);
    if (!e)
      continue;
    if (auto *s = m_scene.Registry().TryGet<engine::components::Sprite>(*e);
        s && !s->regionName.empty()) {
      ApplyRegion(*e, s->regionName);
    }
  }

  for (auto name : {"GoldText", "WaveText", "LivesText"}) {
    auto e = m_scene.FindEntity(name);
    if (!e)
      continue;
    if (auto *t =
            m_scene.Registry().TryGet<engine::components::TextComponent>(*e)) {
      t->font = m_fontHandle;
    }
  }

  engine::Logger::Info("[PlayingState] assets reloaded");
}

void PlayingState::ApplyRegion(engine::ecs::Entity e, std::string_view region) {
  if (!m_atlasCache)
    return;
  if (!m_atlasCache->GetRegion(std::string(region))) {
    engine::Logger::Warn("[PlayingState] unknown region: %.*s",
                         static_cast<int>(region.size()), region.data());
    return;
  }
  auto &s = m_scene.Registry().Get<engine::components::Sprite>(e);
  s.atlas = m_atlasHandle;
  s.regionName = std::string(region);
}

void PlayingState::OnImGui(engine::core::Game &game) {
#ifndef DEBUG
  DrawSceneDebug(game);
#endif
}

void PlayingState::DrawSceneDebug(engine::core::Game &game) {
  auto &reg = m_scene.Registry();
  auto *assets = game.Assets();

  ImGui::Begin("Renderer");
  ImGui::Text("Draw calls: %u", game.GetRenderer().DrawCalls());
  ImGui::Text("Quads:      %u", game.GetRenderer().QuadCount());

  bool fullscreen = game.IsFullscreen();
  if (ImGui::Checkbox("Fullscreen", &fullscreen))
    game.SetFullscreen(fullscreen);

  const auto ws = game.Display().WindowSize();
  ImGui::Text("Window: %d x %d", ws.x, ws.y);
  ImGui::Text("Framebuffer: %d x %d", game.Display().FramebufferSize().x,
              game.Display().FramebufferSize().y);

  if (!game.IsFullscreen()) {
    if (ImGui::Button("1280x720"))
      game.SetWindowedSize(1280, 720);
    ImGui::SameLine();
    if (ImGui::Button("1600x900"))
      game.SetWindowedSize(1600, 900);
    ImGui::SameLine();
    if (ImGui::Button("1920x1080"))
      game.SetWindowedSize(1920, 1080);
  }

  ImGui::Separator();

  if (assets) {
    ImGui::Text("Atlas handle: %s", m_atlasHandle.IsNull()
                                        ? "(null)"
                                        : m_atlasHandle.ToString().c_str());
    if (ImGui::Button("Force Reimport Atlas"))
      assets->Reimport(m_atlasHandle);
    if (ImGui::Button("Save Scene"))
      engine::scene::SceneSerializer::Save(m_scene, "scene.json");
  }

  ImGui::Separator();

  if (auto camE = m_scene.FindEntity("MainCamera"); camE) {
    if (auto *camera = reg.TryGet<engine::components::Camera2D>(*camE)) {
      auto p = camera->position;
      ImGui::Text("Cam pos: (%.2f, %.2f)", p.x, p.y);
      ImGui::Text("Cam zoom: %.2f", camera->zoom);
      ImGui::Text("Rot:  %.2f rad", camera->rotation);
      ImGui::Text("Ortho half-height: %.2f", camera->orthoSize);
    }
  }

  ImGui::Separator();
  ImGui::Text("WASD move | Q/E rotate | Wheel zoom");

  ImGui::Separator();

  if (ImGui::Button("Spawn Particle Emitter") && m_atlasCache) {
    auto e = m_scene.CreateEntity("Emitter" + std::to_string(rand()));
    reg.Emplace<engine::components::Transform>(
        e,
        engine::components::Transform{
            .position = {(float)(rand() % 20 - 10), (float)(rand() % 14 - 7)}});

    reg.Emplace<engine::components::ParticleEmitter>(
        e, engine::components::ParticleEmitter{
               .emitRate = 200.0f,
               .duration = 1.5f,
               .minLife = 0.5f,
               .maxLife = 1.5f,
               .minSpeed = 1.0f,
               .maxSpeed = 5.0f,
               .minScale = 0.1f,
               .maxScale = 0.3f,
               .startColor = {1.0f, 0.8f, 0.2f, 1.0f},
               .endColor = {1.0f, 0.0f, 0.0f, 0.0f},
               .atlas = m_atlasHandle,
               .regionName = "solidWhite",
               .layer = engine::components::RenderLayer::Overlay,
           });
  }

  ImGui::End();
}
