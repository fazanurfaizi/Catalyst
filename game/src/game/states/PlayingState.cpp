#include "game/states/PlayingState.hpp"

#include "engine/asset/manager/AssetManagerEditor.hpp"
#include "engine/audio/MusicPlayer.hpp"
#include "engine/components/AudioSource.hpp"
#include "engine/components/Light.hpp"
#include "engine/components/TextComponent.hpp"
#include "engine/core/Game.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/graphics/assets/FontAsset.hpp"
#include "engine/graphics/assets/TextureAtlasAsset.hpp"
#include "engine/math/Color.hpp"
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
                   const engine::graphics::TextureAtlasAsset &atlas) {
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
      game.Assets()->AcquireByName<engine::graphics::FontAsset>("main_font");

  m_bgm = game.Assets()->AcquireByName<engine::audio::SoundAsset>("main_bgm");

  m_atlas = game.Assets()->AcquireByName<engine::graphics::TextureAtlasAsset>(
      "main_atlas");

  if (m_bgm) {
    m_music = std::make_unique<engine::audio::MusicPlayer>();
    m_music->SetLooping(true);
    m_music->SetVolume(0.4f);

    if (m_music->Load(m_bgm->sourcePath.string())) {
      m_music->Play();
      engine::Logger::Info("[PlayingState] BGM playing: '%s'",
                           m_bgm->sourcePath.string().c_str());
    } else {
      engine::Logger::Warn("[PlayingState] failed to load BGM '%s'",
                           m_bgm->sourcePath.string().c_str());
      m_music.reset();
    }
  } else {
    engine::Logger::Warn("[PlayingState] 'main_bgm' missing; audio disabled");
  }

  m_lastReloadGen = game.Assets()->GetReloadGeneration();

  auto &reg = m_world.Registry();
  m_camera = m_world.CreateEntity("MainCamera");

  game.GetWindow().ClearWindow();

  auto &cam = reg.Emplace<engine::components::Camera2D>(m_camera);
  cam.orthoSize = 10.0f;
  cam.active = true;

  auto framebufferSize = game.Display().FramebufferSize();
  game.GetRenderer().SetAmbientLight({0.03f, 0.04f, 0.25f});

  m_animations.AddStrip("hero_idle", *m_atlas, "hero_idle", 10, 0.15f, true);
  m_animations.AddStrip("hero_attack", *m_atlas, "hero_attack", 10, 0.08f,
                        false);
  m_animations.AddStrip("hero_move", *m_atlas, "hero_move", 10, 0.1f, true);
  m_animations.AddStrip("coin_spin", *m_atlas, "coin", 6, 0.08f, true);

  BuildDemoScene(game);
  BuildUI(game);
}

void PlayingState::OnRender(engine::core::Game &game) {
  auto &renderer = game.GetRenderer();
  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);

  const float aspect = renderer.AspectRatio();
  const glm::mat4 vp = camera.ViewProjectionMatrix(aspect);
  const engine::math::Rect bounds = camera.VisibleWorldBounds(aspect);

  renderer.BeginFrame(vp, bounds);
  DrawWorldGrid(renderer, bounds, *m_atlas);
  renderer.RenderSprites(m_world, game.Assets());
  renderer.RenderLights(m_world);
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

  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);
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

  engine::systems::AudioSystem(m_world.Registry(), *game.Assets());
}

void PlayingState::OnFixedUpdate(engine::core::Game &game, double dt) {
  auto &reg = m_world.Registry();

  const auto atlasHandle = game.Assets()->FindByName("main_atlas");

  if (reg.IsAlive(m_hero)) {
    auto &a = reg.Get<engine::components::Animator>(m_hero);
    auto &t = reg.Get<engine::components::Transform>(m_hero);
    auto &v = reg.Get<engine::components::Velocity>(m_hero);

    // Transition: attack finished -> idle.
    if (a.finished && a.clipName == "hero_attack") {
      engine::components::PlayAnimation(a, atlasHandle, "hero_idle");
    }

    if (!v.ArrivedAtTarget(t.position)) {
      if (a.clipName != "hero_attack" && a.clipName != "hero_move") {
        engine::components::PlayAnimation(a, atlasHandle, "hero_move");
      }
    } else {
      t.position = v.targetPosition;
      if (a.clipName != "hero_attack" && a.clipName != "hero_idle") {
        engine::components::PlayAnimation(a, atlasHandle, "hero_idle");
      }
    }
  }

  // Enemy orbits origin.
  if (reg.IsAlive(m_enemy)) {
    const float angle = static_cast<float>(game.ElapsedTime());
    auto &t = reg.Get<engine::components::Transform>(m_enemy);
    t.position = {std::cos(angle) * 6.0f, std::sin(angle) * 4.0f};
    t.rotation = angle;
  }

  if (reg.IsAlive(m_torch)) {
    auto &light = reg.Get<engine::components::PointLight>(m_torch);
    const float t = static_cast<float>(game.ElapsedTime());
    // Cheap two-sine flicker — looks random enough, no state needed.
    const float flicker =
        0.80f + 0.15f * std::sin(t * 12.7f) + 0.05f * std::sin(t * 31.3f);
    light.diffuse = glm::vec3{1.00f, 0.65f, 0.25f} * flicker;
  }

  engine::systems::AnimationSystem(reg, m_animations, static_cast<float>(dt));
  engine::systems::ParticleSystem(reg, dt);
  engine::systems::MoveTowardTargetSystem(reg, dt);

  reg.FlushDeferred();
}

void PlayingState::OnLateUpdate(engine::core::Game &game, double dt) {
  auto &reg = m_world.Registry();

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
  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);

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

  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);
  auto &in = game.Input();

  const auto fbSize = game.Display().FramebufferSize();
  auto cursor = in.CursorPos();
  glm::vec2 cursorFb = game.Display().CursorToFramebuffer(
      {static_cast<float>(cursor.x), static_cast<float>(cursor.y)});
  glm::vec2 worldPos = camera.ScreenToWorld(cursorFb, fbSize);

  if (button == engine::input::MouseButton::Left) {
    auto &animation = reg.Get<engine::components::Animator>(m_hero);
    if (animation.clipName == "hero_idle") {
      engine::components::PlayAnimation(
          animation, game.Assets()->FindByName("main_atlas"), "hero_attack");
      engine::systems::PlayAudioNow(reg, *game.Assets(), m_hero);
    }
  }

  if (button == engine::input::MouseButton::Right) {
    if (auto *val = reg.TryGet<engine::components::Velocity>(m_hero)) {
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

void PlayingState::BuildDemoScene(engine::core::Game &game) {
  auto &reg = m_world.Registry();
  auto &cam = reg.Get<engine::components::Camera2D>(m_camera);

  // Tower
  {
    m_tower = m_world.CreateEntity("Tower");
    reg.Emplace<engine::components::Transform>(m_tower,
                                               engine::components::Transform{
                                                   .position = {-4.0f, -2.0f},
                                                   .scale = {1.5f, 1.5f},
                                               });
    reg.Emplace<engine::components::Sprite>(
        m_tower, engine::components::Sprite{
                     .layer = engine::components::RenderLayer::Entities});
    ApplyRegion(m_tower, "tower");
  }

  // Hero
  {
    m_hero = m_world.CreateEntity("Hero");
    reg.Emplace<engine::components::Transform>(
        m_hero, engine::components::Transform{.position = {4.0f, -2.0f},
                                              .scale = {1.2f, 1.7f}});
    reg.Emplace<engine::components::Sprite>(
        m_hero, engine::components::Sprite{
                    .layer = engine::components::RenderLayer::Entities});
    reg.Emplace<engine::components::Velocity>(
        m_hero, engine::components::Velocity{
                    .targetPosition = {4.0f, -2.0f},
                    .speed = 5.0f,
                });
    reg.Emplace<engine::components::Animator>(m_hero);
    ApplyRegion(m_hero, "hero_idle_0");
    cam.followTarget = m_hero;
    auto &hero_light = reg.Emplace<engine::components::PointLight>(m_hero);
    hero_light.ambient = {0.15f, 0.08f, 0.02f};
    hero_light.diffuse = {1.00f, 0.65f, 0.25f}; // warm fire orange
    hero_light.specular = {0.00f, 0.00f, 0.00f};
    hero_light.constant = 1.0f;
    hero_light.linear = 1.5f;
    hero_light.quadratic = 1.2f;

    auto &sfx = reg.Emplace<engine::components::AudioSource>(m_hero);
    sfx.clip = game.Assets()->FindByName("hero_attack_sfx");
    sfx.volume = 0.8;

    if (reg.IsAlive(m_hero)) {
      auto &a = reg.Get<engine::components::Animator>(m_hero);
      if (a.finished && a.clipName == "hero_idle") {
        engine::components::PlayAnimation(a, m_atlasHandle, "hero_idle");
      }
    }
  }

  // Enemy
  {
    m_enemy = m_world.CreateEntity("Enemy");
    reg.Emplace<engine::components::Transform>(
        m_enemy, engine::components::Transform{.scale = {0.9f, 1.0f}});
    reg.Emplace<engine::components::Sprite>(
        m_enemy, engine::components::Sprite{
                     .layer = engine::components::RenderLayer::Entities});
    ApplyRegion(m_enemy, "enemy");
  }

  // Coin
  {
    m_coin = m_world.CreateEntity("Coin");
    reg.Emplace<engine::components::Transform>(
        m_coin, engine::components::Transform{.position = {3.0f, 3.0f},
                                              .scale = {0.8f, 0.8f}});
    reg.Emplace<engine::components::Sprite>(
        m_coin, engine::components::Sprite{
                    .color = engine::math::kColorWhite,
                    .layer = engine::components::RenderLayer::Ground});
    reg.Emplace<engine::components::Animator>(m_coin);
    ApplyRegion(m_coin, "coin_0");

    if (reg.IsAlive(m_coin)) {
      auto &a = reg.Get<engine::components::Animator>(m_coin);
      if (a.finished && a.clipName == "coin_spin") {
        engine::components::PlayAnimation(a, m_atlasHandle, "coin_spin");
      }
    }
  }

  // Torch
  {
    m_torch = m_world.CreateEntity("Torch");
    reg.Emplace<engine::components::Transform>(m_torch,
                                               engine::components::Transform{
                                                   .position = {0.0f, 3.0f},
                                                   .scale = {0.6f, 0.6f},
                                               });
    reg.Emplace<engine::components::Sprite>(
        m_torch, engine::components::Sprite{
                     .color = engine::math::kColorWhite,
                     .layer = engine::components::RenderLayer::Entities,
                 });
    ApplyRegion(m_torch,
                "tower"); // reuse an orange-red solid as the torch body

    auto &torch = reg.Emplace<engine::components::PointLight>(m_torch);
    torch.ambient = {0.15f, 0.08f, 0.02f};
    torch.diffuse = {1.00f, 0.65f, 0.25f}; // warm fire orange
    torch.specular = {0.00f, 0.00f, 0.00f};
    torch.constant = 1.0f;
    torch.linear = 2.0f;
    torch.quadratic = 3.5f;
  }
}

void PlayingState::ApplyRegion(engine::ecs::Entity e, std::string_view region) {
  if (!m_atlas)
    return;
  if (!m_atlas->GetRegion(std::string(region))) {
    engine::Logger::Warn("[PlayingState] unknown region: %s",
                         std::string(region).c_str());
    return;
  }
  auto &s = m_world.Registry().Get<engine::components::Sprite>(e);
  s.atlas = m_atlasHandle;
  s.regionName = std::string(region);
}

void PlayingState::DrawSceneDebug(engine::core::Game &game) {
  auto &reg = m_world.Registry();

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

  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);
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
    auto e = m_world.CreateEntity("Emitter");
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
      game.Assets()->AcquireByName<engine::graphics::FontAsset>("main_font");
  auto newAtlas =
      game.Assets()->AcquireByName<engine::graphics::TextureAtlasAsset>(
          "main_atlas");
  auto newBgm =
      game.Assets()->AcquireByName<engine::audio::SoundAsset>("main_bgm");

  if (!newAtlas) {
    engine::Logger::Warn(
        "[PlayingState] reload: 'main_atlas' unavailable; keeping previous");
    return;
  }

  m_font = std::move(newFont);
  m_atlas = std::move(newAtlas);

  if (m_music && newBgm) {
    m_bgm = std::move(newBgm);
    if (m_music->Load(m_bgm->sourcePath.string())) {
      m_music->Play();
      engine::Logger::Info("[PlayingState] BGM reloaded");
    } else {
      engine::Logger::Warn(
          "[PlayingState] BGM reload failed; continuing without audio");
      m_music.reset();
    }
  }

  m_animations = engine::animation::AnimationLibrary{};
  m_animations.AddStrip("hero_idle", *m_atlas, "hero_idle", 10, 0.15f, true);
  m_animations.AddStrip("hero_attack", *m_atlas, "hero_attack", 10, 0.08f,
                        false);
  m_animations.AddStrip("hero_move", *m_atlas, "hero_move", 10, 0.10f, true);
  m_animations.AddStrip("coin_spin", *m_atlas, "coin", 6, 0.08f, true);

  ApplyRegion(m_hero, "hero_idle_0");
  ApplyRegion(m_tower, "tower");
  ApplyRegion(m_enemy, "enemy");
  ApplyRegion(m_coin, "coin_0");
  ApplyRegion(m_torch, "tower");

  engine::Logger::Info("[PlayingState] assets reloaded");
}

void PlayingState::BuildUI(engine::core::Game &game) {
  auto &reg = m_world.Registry();

  m_goldText = m_world.CreateEntity("GoldText");
  reg.Emplace<engine::components::Transform>(
      m_goldText, engine::components::Transform{.position = {20.0f, 30.0f}});
  {
    auto &t = reg.Emplace<engine::components::TextComponent>(m_goldText);
    t.text = "Gold: 100";
    t.font = m_fontHandle;
    t.color = {1.0f, 0.85f, 0.2f, 1.0f};
    t.scale = 2.0f;
    t.alignment = engine::graphics::TextAlignment::Left;
  }

  m_waveText = m_world.CreateEntity("WaveText");
  reg.Emplace<engine::components::Transform>(
      m_waveText, engine::components::Transform{.position = {400.0f, 30.0f}});
  {
    auto &t = reg.Emplace<engine::components::TextComponent>(m_waveText);
    t.text = "Wave 1 / 10";
    t.font = m_fontHandle;
    t.color = {1.0f, 1.0f, 1.0f, 1.0f};
    t.scale = 1.5f;
  }

  m_livesText = m_world.CreateEntity("LivesText");
  reg.Emplace<engine::components::Transform>(
      m_livesText, engine::components::Transform{.position = {400.0f, 70.0f}});
  {
    auto &t = reg.Emplace<engine::components::TextComponent>(m_livesText);
    t.text = "Lives: 20";
    t.font = m_fontHandle;
    t.color = {1.0f, 0.4f, 0.4f, 1.0f};
    t.scale = 1.5f;
  }
}
