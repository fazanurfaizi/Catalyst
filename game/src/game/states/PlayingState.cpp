#include "game/states/PlayingState.hpp"

#include "engine/asset/manager/AssetManagerEditor.hpp"
#include "engine/audio/MusicPlayer.hpp"
#include "engine/components/AudioSource.hpp"
#include "engine/components/Light.hpp"
#include "engine/components/TextComponent.hpp"
#include "engine/core/Game.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/math/Color.hpp"
#include "engine/resources/FontAsset.hpp"
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
  const int cols = x1 - x0;
  const int rows = y1 - y0;
  if (cols * rows > kMaxCells)
    return;

  const engine::math::Color evenCell{0.13f, 0.17f, 0.24f, 1.0f};
  const engine::math::Color oddCell{0.16f, 0.20f, 0.28f, 1.0f};
  const engine::math::Color axisCell{0.20f, 0.24f, 0.34f, 1.0f};
  const auto *tex = &atlas.GetTexture();

  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      engine::math::Color color;
      if (x == 0 || y == 0)
        color = axisCell;
      else
        color = ((x + y) & 1) ? oddCell : evenCell;

      renderer.Draw({static_cast<float>(x) + 0.5f,
                     static_cast<float>(y) + 0.5f}, // position
                    {1.0f, 1.0f},                   // scale
                    0.0f,                           // rotation
                    color, tex, region->uvMin, region->uvMax,
                    static_cast<int>(engine::components::RenderLayer::Ground));
    }
  }
}
} // namespace

void PlayingState::OnEnter(engine::core::Game &game) {
  engine::Logger::Info("OnInitialize");

  using engine::asset::AssetManagerEditor;

  m_fontHandle = game.Assets()->FindByName("main_font");
  m_atlasHandle = game.Assets()->FindByName("main_atlas");

  m_font =
      game.Assets()->AcquireByName<engine::resources::FontAsset>("main_font");

  m_bgm =
      game.Assets()->AcquireByName<engine::resources::SoundAsset>("main_bgm");

  m_atlas = game.Assets()->AcquireByName<engine::resources::TextureAtlasAsset>(
      "main_atlas");

  if (m_bgm) {
    m_music = std::make_unique<engine::audio::MusicPlayer>();
    m_music->SetLooping(true);
    m_music->SetVolume(0.4f);

    // if (m_music->Load(m_bgm->sourcePath.string())) {
    //   m_music->Play();
    //   engine::Logger::Info("[PlayingState] BGM playing: '%s'",
    //                        m_bgm->sourcePath.string().c_str());
    // } else {
    //   engine::Logger::Warn("[PlayingState] failed to load BGM '%s'",
    //                        m_bgm->sourcePath.string().c_str());
    //   m_music.reset();
    // }
  } else {
    engine::Logger::Warn("[PlayingState] 'main_bgm' missing; audio disabled");
  }

  m_lastReloadGen = game.Assets()->GetReloadGeneration();

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
      engine::Logger::Warn(
          "[PlayingState] scene.json is missing entity '%s' "
          "(or its Name component is empty — re-save the scene)",
          name);
  }

  auto camera = m_scene.FindEntity("MainCamera");
  auto hero = m_scene.FindEntity("Hero");
  auto coin = m_scene.FindEntity("Coin");

  // Reset animation
  if (auto *a = reg.TryGet<engine::components::Animator>(*hero)) {
    engine::components::PlayAnimation(*a, m_atlasHandle, "hero_idle");
  }

  if (auto *a = reg.TryGet<engine::components::Animator>(*coin)) {
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

  if (auto *s = reg.TryGet<engine::components::AudioSource>(*hero)) {
    s->clip = game.Assets()->FindByName("hero_attack_sfx");
  }

  if (auto *cam = reg.TryGet<engine::components::Camera2D>(*camera)) {
    if (reg.IsAlive(*hero))
      cam->followTarget = *hero;
    cam->active = true;
  }

  m_animations.AddStrip("hero_idle", *m_atlas, "hero_idle", 10, 0.15f, true);
  m_animations.AddStrip("hero_attack", *m_atlas, "hero_attack", 10, 0.08f,
                        false);
  m_animations.AddStrip("hero_move", *m_atlas, "hero_move", 10, 0.10f, true);
  m_animations.AddStrip("coin_spin", *m_atlas, "coin", 6, 0.08f, true);
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

  const float aspect = renderer.AspectRatio();
  const glm::mat4 vp = camera->ViewProjectionMatrix(aspect);
  const engine::math::Rect bounds = camera->VisibleWorldBounds(aspect);

  renderer.BeginFrame(vp, bounds);
  DrawWorldGrid(renderer, bounds, *m_atlas);
  renderer.RenderSprites(m_scene, game.Assets());
  renderer.RenderLights(m_scene);
  renderer.EndFrame();

  const glm::mat4 uiProjection =
      game.Display().UiProjection() / game.Display().AspectRatio();
  engine::systems::TextRenderSystem(reg, *game.Assets(), m_textRenderer,
                                    uiProjection);
}

void PlayingState::OnUpdate(engine::core::Game &game, double dt) {
  if (game.Assets()->GetReloadGeneration() != m_lastReloadGen) {
    m_lastReloadGen = game.Assets()->GetReloadGeneration();
    ReloadAssets(game);
  }

  auto &reg = m_scene.Registry();
  auto camE = m_scene.FindEntity("MainCamera");
  auto &camera = reg.Get<engine::components::Camera2D>(*camE);
  auto &in = game.Input();

  const float speed = 5.0f * static_cast<float>(dt);

  if (in.IsKeyDown(engine::input::Key::W))
    camera.Move({0.0f, speed});
  if (in.IsKeyDown(engine::input::Key::S))
    camera.Move({0.0f, -speed});
  if (in.IsKeyDown(engine::input::Key::A))
    camera.Move({-speed, 0.0f});
  if (in.IsKeyDown(engine::input::Key::D))
    camera.Move({speed, 0.0f});
  if (in.IsKeyDown(engine::input::Key::Q))
    camera.RotateBy(+0.5f * static_cast<float>(dt));
  if (in.IsKeyDown(engine::input::Key::E))
    camera.RotateBy(-0.5f * static_cast<float>(dt));

  engine::systems::AudioSystem(m_scene.Registry(), *game.Assets());
}

void PlayingState::OnFixedUpdate(engine::core::Game &game, double dt) {
  auto &reg = m_scene.Registry();
  const auto atlasHandle = game.Assets()->FindByName("main_atlas");

  if (auto hero = m_scene.FindEntity("Hero"); hero && reg.IsAlive(*hero)) {
    auto *a = reg.TryGet<engine::components::Animator>(*hero);
    auto *t = reg.TryGet<engine::components::Transform>(*hero);
    auto *v = reg.TryGet<engine::components::Velocity>(*hero);
    if (a && t && v) {
      if (a->finished && a->clipName == "hero_attack")
        engine::components::PlayAnimation(*a, atlasHandle, "hero_idle");

      if (!v->ArrivedAtTarget(t->position)) {
        if (a->clipName != "hero_attack" && a->clipName != "hero_move")
          engine::components::PlayAnimation(*a, atlasHandle, "hero_move");
      } else {
        t->position = v->targetPosition;
        if (a->clipName != "hero_attack" && a->clipName != "hero_idle")
          engine::components::PlayAnimation(*a, atlasHandle, "hero_idle");
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
    } else {
      engine::Logger::Warn(
          "[PlayingState] 'Torch' has no PointLight — scene.json is stale");
    }
  }

  engine::systems::AnimationSystem(reg, m_animations, static_cast<float>(dt));
  engine::systems::ParticleSystem(reg, static_cast<float>(dt));
  engine::systems::MoveTowardTargetSystem(reg, static_cast<float>(dt));
  reg.FlushDeferred();
}

void PlayingState::OnLateUpdate(engine::core::Game &game, double dt) {
  auto &reg = m_scene.Registry();

  engine::systems::CameraFollowSystem(reg, dt);
}

void PlayingState::OnImGui(engine::core::Game &game) {
#ifndef DEBUG
  DrawSceneDebug(game);
#endif // DEBUG
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
    game.ToggleFullscreen();

  if (key == engine::input::Key::Space)
    engine::Logger::Info("Space tapped");
}

void PlayingState::OnScroll(engine::core::Game &game, double dx, double dy) {
  (void)dx;
  // Zoom around the cursor for natural feel.
  auto &reg = m_scene.Registry();
  auto &camera =
      reg.Get<engine::components::Camera2D>(*m_scene.FindEntity("MainCamera"));

  const auto fbSize = game.Display().FramebufferSize();

  auto cursor = game.Input().CursorPos();
  glm::vec2 cursorFb = game.Display().CursorToFramebuffer(
      {static_cast<float>(cursor.x), static_cast<float>(cursor.y)});

  const auto before = camera.ScreenToWorld(cursorFb, fbSize);
  camera.ZoomBy(dy > 0 ? 1.1f : 1.0f / 1.1f);
  const auto after = camera.ScreenToWorld(cursorFb, fbSize);
  const auto delta = before - after;
  camera.Move({delta.x, delta.y});
}

void PlayingState::OnMouseButton(engine::core::Game &game,
                                 engine::input::MouseButton button,
                                 engine::input::KeyAction action, int mods) {
  (void)mods;
  if (action != engine::input::KeyAction::Press)
    return;

  auto &reg = m_scene.Registry();
  auto &camera =
      reg.Get<engine::components::Camera2D>(*m_scene.FindEntity("MainCamera"));
  auto &in = game.Input();

  const auto fbSize = game.Display().FramebufferSize();
  auto cursor = in.CursorPos();
  glm::vec2 cursorFb = game.Display().CursorToFramebuffer(
      {static_cast<float>(cursor.x), static_cast<float>(cursor.y)});
  glm::vec2 worldPos = camera.ScreenToWorld(cursorFb, fbSize);

  auto hero = m_scene.FindEntity("Hero");
  if (button == engine::input::MouseButton::Left) {
    auto &animation = reg.Get<engine::components::Animator>(*hero);
    if (animation.clipName == "hero_idle") {
      engine::components::PlayAnimation(
          animation, game.Assets()->FindByName("main_atlas"), "hero_attack");
      engine::systems::PlayAudioNow(reg, *game.Assets(), *hero);
    }
  }

  if (button == engine::input::MouseButton::Right) {
    if (auto *val = reg.TryGet<engine::components::Velocity>(*hero)) {
      val->targetPosition = worldPos;
    }
  }
}

void PlayingState::OnMouseMove(engine::core::Game &game, double x, double y) {
  (void)x;
  (void)y;
}

void PlayingState::OnResize(engine::core::Game &game, int w, int h) {
  const auto fb = game.Display().FramebufferSize();
  game.GetRenderer().Resize(fb.x, fb.y);
}

void PlayingState::ApplyRegion(engine::ecs::Entity e, std::string_view region) {
  if (!m_atlas)
    return;
  if (!m_atlas->GetRegion(std::string(region))) {
    engine::Logger::Warn("[PlayingState] unknown region: %s",
                         std::string(region).c_str());
    return;
  }
  auto &s = m_scene.Registry().Get<engine::components::Sprite>(e);
  s.atlas = m_atlasHandle;
  s.regionName = std::string(region);
}

void PlayingState::DrawSceneDebug(engine::core::Game &game) {
  auto &reg = m_scene.Registry();

  ImGui::Begin("Renderer");
  ImGui::Text("Draw calls: %u", game.GetRenderer().DrawCalls());
  ImGui::Text("Quads:      %u", game.GetRenderer().QuadCount());
  // ImGui::Text("Culled:     %u", game.GetRenderer().CulledCount());
  // ImGui::Text("Alive:      %zu", m_scene.Size());
  // ImGui::Text("Clips: %d", m_animations.Count());

  bool fullscreen = game.IsFullscreen();
  if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
    game.SetFullscreen(fullscreen);
  }

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

  if (ImGui::Button("Save Scene")) {
    engine::scene::SceneSerializer::Save(m_scene, "scene.json");
  }

  ImGui::Separator();

  auto &camera =
      reg.Get<engine::components::Camera2D>(*m_scene.FindEntity("MainCamera"));
  auto p = camera.position;
  ImGui::Text("Cam pos: (%.2f, %.2f)", p.x, p.y);
  ImGui::Text("Cam zoom: %.2f", camera.zoom);
  ImGui::Text("Rot:  %.2f rad", camera.rotation);
  ImGui::Text("Ortho half-height: %.2f", camera.orthoSize);
  ImGui::Separator();
  ImGui::Text("WASD  move");
  ImGui::Text("Q / E rotate");
  ImGui::Text("Wheel zoom at cursor");

  ImGui::Separator();

  if (ImGui::Button("Spawn Particle Emitter")) {
    auto e = m_scene.CreateEntity("Emitter");
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

  ImGui::Separator();

  ImGui::End();
}

void PlayingState::ReloadAssets(engine::core::Game &game) {
  auto newFont =
      game.Assets()->AcquireByName<engine::resources::FontAsset>("main_font");
  auto newAtlas =
      game.Assets()->AcquireByName<engine::resources::TextureAtlasAsset>(
          "main_atlas");
  auto newBgm =
      game.Assets()->AcquireByName<engine::resources::SoundAsset>("main_bgm");

  if (!newAtlas) {
    engine::Logger::Warn(
        "[PlayingState] reload: 'main_atlas' unavailable; keeping previous");
    return;
  }

  m_font = std::move(newFont);
  m_atlas = std::move(newAtlas);

  // if (m_music && newBgm) {
  //   m_bgm = std::move(newBgm);
  //   if (m_music->Load(m_bgm->sourcePath.string())) {
  //     m_music->Play();
  //     engine::Logger::Info("[PlayingState] BGM reloaded");
  //   } else {
  //     engine::Logger::Warn(
  //         "[PlayingState] BGM reload failed; continuing without audio");
  //     m_music.reset();
  //   }
  // }

  m_animations = engine::animation::AnimationLibrary{};
  m_animations.AddStrip("hero_idle", *m_atlas, "hero_idle", 10, 0.15f, true);
  m_animations.AddStrip("hero_attack", *m_atlas, "hero_attack", 10, 0.08f,
                        false);
  m_animations.AddStrip("hero_move", *m_atlas, "hero_move", 10, 0.10f, true);
  m_animations.AddStrip("coin_spin", *m_atlas, "coin", 6, 0.08f, true);

  auto applyRegion = [&](std::string_view entityName, std::string_view region) {
    if (auto e = m_scene.FindEntity(entityName); e)
      ApplyRegion(*e, region);
  };
  applyRegion("Hero", "hero_idle_0");
  applyRegion("Tower", "tower");
  applyRegion("Enemy", "enemy");
  applyRegion("Coin", "coin_0");
  applyRegion("Torch", "tower");

  engine::Logger::Info("[PlayingState] assets reloaded");
}
