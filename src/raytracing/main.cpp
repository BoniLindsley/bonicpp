// External dependencies.
#include <SDL3/SDL.h>
#include <argparse/argparse.hpp>
#include <boost/safe_numerics/safe_integer.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// Standard libraries.
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

struct Color {
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
};

struct CartesianCoordinate {
  double x;
  double y;
  double z;
};

struct SphericalCoordinate {
  double distance;
  double polar;
  double azimuth;
};

auto add(CartesianCoordinate self, CartesianCoordinate other)
    -> CartesianCoordinate {
  return CartesianCoordinate{
      .x = self.x + other.x,
      .y = self.y + other.y,
      .z = self.z + other.z,
  };
}

auto multiply(CartesianCoordinate self, double other)
    -> CartesianCoordinate {
  return CartesianCoordinate{
      .x = self.x * other,
      .y = self.y * other,
      .z = self.z * other,
  };
}

auto divide(CartesianCoordinate self, double other)
    -> CartesianCoordinate {
  return CartesianCoordinate{
      .x = self.x / other,
      .y = self.y / other,
      .z = self.z / other,
  };
}

auto subtract(CartesianCoordinate self, CartesianCoordinate other)
    -> CartesianCoordinate {
  return CartesianCoordinate{
      .x = self.x - other.x,
      .y = self.y - other.y,
      .z = self.z - other.z,
  };
}

auto dot(CartesianCoordinate self, CartesianCoordinate other) -> double {
  return self.x * other.x + self.y * other.y + self.z * other.z;
}

auto length_squared(CartesianCoordinate self) -> double {
  return dot(self, self);
}

auto length(CartesianCoordinate self) -> double {
  return std::sqrt(length_squared(self));
}

auto unit(CartesianCoordinate self) -> CartesianCoordinate {
  return divide(self, length(self));
}

auto to_cartesian_coordinate(SphericalCoordinate from)
    -> CartesianCoordinate {
  return CartesianCoordinate{
      .x = from.distance * std::sin(from.polar) * std::cos(from.azimuth),
      .y = from.distance * std::sin(from.polar) * std::sin(from.azimuth),
      .z = from.distance * std::cos(from.polar),
  };
}

auto to_spherical_coordinate(CartesianCoordinate from)
    -> SphericalCoordinate {
  auto distance =
      std::sqrt(from.x * from.x + from.y + from.y + from.z + from.z);
  return SphericalCoordinate{
      .distance = distance,
      .polar = from.z != 0 ? std::acos(from.z / distance) : 0,
      .azimuth = std::atan2(from.y, from.x),
  };
}

struct RenderConfig {
  CartesianCoordinate camera_position;
  unsigned int image_width;
  unsigned int image_height;
};

struct RenderResult {
  unsigned int image_height;
  unsigned int image_width;
  std::vector<Color> pixels;
};

auto render(RenderConfig render_config) -> RenderResult {
  auto& camera_position = render_config.camera_position;
  auto& image_height = render_config.image_height;
  auto& image_width = render_config.image_width;

  auto viewport_position = SphericalCoordinate{
      .distance = 100.0, .polar = 0.0, .azimuth = 0.0};
  auto viewport_width = 128.0;
  auto viewport_height = viewport_width / image_width * image_height;

  auto pi = std::atan(1.0) * 4;
  std::array<CartesianCoordinate, 3> viewport_basis{
      to_cartesian_coordinate(SphericalCoordinate{
          .distance = viewport_width,
          .polar = viewport_position.polar + pi / 2,
          .azimuth = 0.0,
      }),
      to_cartesian_coordinate(SphericalCoordinate{
          .distance = viewport_height,
          .polar = viewport_position.polar + pi / 2,
          .azimuth = viewport_position.azimuth + pi / 2,
      }),
      to_cartesian_coordinate(viewport_position),
  };

  // Render to buffer.
  RenderResult render_result{
      .image_height = image_height,
      .image_width = image_width,
      .pixels = std::vector<Color>(
          static_cast<std::size_t>(image_width) * image_height)};
  auto& pixels = render_result.pixels;
  for (int j = 0; j < image_height; ++j) {
    for (int i = 0; i < image_width; ++i) {
      // Pixel to render in viewport space.
      auto pixel_viewport_u = (i * 2.0 + 1) / (2.0 * image_width);
      auto pixel_viewport_v = (j * 2.0 + 1) / (2.0 * image_height);

      // Pixel to render in camera space.
      auto pixel_camera_position = CartesianCoordinate{
          .x = (pixel_viewport_u - 0.5) * viewport_basis[0].x +
               (pixel_viewport_v - 0.5) * viewport_basis[1].x +
               viewport_basis[2].x,
          .y = (pixel_viewport_u - 0.5) * viewport_basis[0].y +
               (pixel_viewport_v - 0.5) * viewport_basis[1].y +
               viewport_basis[2].y,
          .z = (pixel_viewport_u - 0.5) * viewport_basis[0].z +
               (pixel_viewport_v - 0.5) * viewport_basis[1].z +
               viewport_basis[2].z,
      };

      auto sphere_position =
          CartesianCoordinate{.x = 200, .y = 200, .z = 500};
      auto sphere_radius = 50;
      auto sphere_camera_position =
          subtract(sphere_position, camera_position);
      auto quad_a = length_squared(pixel_camera_position);
      auto quad_h = dot(pixel_camera_position, sphere_camera_position);
      auto quad_c = length_squared(sphere_camera_position) -
                    sphere_radius * sphere_radius;
      auto quad_discriminant = quad_h * quad_h - quad_a * quad_c;
      auto sqrt_discriminant = std::sqrt(quad_discriminant);

      if (quad_discriminant >= 0) {
        auto root = (quad_h - sqrt_discriminant) / quad_a;
        auto intersection =
            add(camera_position, multiply(pixel_camera_position, root));
        auto normal =
            multiply(unit(subtract(intersection, sphere_position)), 255);
        pixels[image_width * j + i] = Color{
            .red = static_cast<uint8_t>(std::fabs(normal.x)),
            .green = static_cast<uint8_t>(std::fabs(normal.y)),
            .blue = static_cast<uint8_t>(std::fabs(normal.z)),
        };
      }
    }
  }

  return render_result;
}

void copy_to_stdout(const RenderResult& render_result) {
  // Prepare sample output data.
  auto maximum_value = 255;

  // Output file format magic number.
  spdlog::debug("Writing PPM header to stdout.");
  std::cout << "P3\n";

  // Output header line: X and Y dimension.
  std::cout << render_result.image_width << ' '
            << render_result.image_height << "\n";

  // Output header line: Maximum channel value.
  std::cout << maximum_value << "\n";

  // Output data.
  spdlog::debug("Writing PPM data to stdout.");
  for (auto color : render_result.pixels) {
    std::cout << static_cast<int>(color.red) << ' '
              << static_cast<int>(color.green) << ' '
              << static_cast<int>(color.blue) << '\n';
  }
}

struct SDLMainArguments {
  int window_width;
  int window_height;
};

auto sdl_main(SDLMainArguments sdl_main_arguments) -> int {
  spdlog::debug("Initialising SDL.");
  auto is_success = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
  if (not is_success) {
    spdlog::error("Unable to initialise SDL: {}", SDL_GetError());
    return 1;
  }
  struct SDLSystem {
    ~SDLSystem() {
      spdlog::debug("Uninitialising SDL.");
      SDL_Quit();
    }
  } sdl_system;

  spdlog::debug("Creating SDL window.");
  using Window =
      std::unique_ptr<SDL_Window, decltype([](SDL_Window* window) {
                        spdlog::debug("Destroying SDL window.");
                        SDL_DestroyWindow(window);
                      })>;
  Window window{SDL_CreateWindow(
      "raytracing-how", sdl_main_arguments.window_width,
      sdl_main_arguments.window_height, 0)};
  if (not window) {
    spdlog::error("Unable to create SDL window: {}", SDL_GetError());
    return 1;
  }

  {
    int window_width = 0;
    int window_height = 0;
    SDL_GetWindowSize(window.get(), &window_width, &window_height);
    spdlog::debug(
        "SDL window size: {} x {}", window_width, window_height);
  };

  spdlog::debug("Creating SDL renderer.");
  using Renderer =
      std::unique_ptr<SDL_Renderer, decltype([](SDL_Renderer* renderer) {
                        spdlog::debug("Destroying SDL renderer.");
                        SDL_DestroyRenderer(renderer);
                      })>;
  Renderer renderer{SDL_CreateRenderer(window.get(), nullptr)};
  if (not renderer) {
    spdlog::error("Unable to create SDL renderer: {}", SDL_GetError());
    return 1;
  }

  using Texture =
      std::unique_ptr<SDL_Texture, decltype([](SDL_Texture* texture) {
                        if (texture != nullptr) {
                          spdlog::debug(
                              "Destroying SDL render texture.");
                          SDL_DestroyTexture(texture);
                        }
                      })>;
  Texture texture;
  auto camera_position = CartesianCoordinate{.x = 0., .y = 0., .z = 0.};

  auto refresh_rate = 60;
  auto refresh_interval = std::chrono::seconds() / refresh_rate;
  auto next_refresh_time = std::chrono::steady_clock::now();

  int frame = 0;
  spdlog::debug("Starting SDL event loop.");
  while (true) {
    auto now = std::chrono::steady_clock::now();
    auto wait_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            next_refresh_time - now);
    auto wait_time_ms =
        boost::safe_numerics::safe<int>(wait_time.count());
    if (wait_time_ms < 0) {
      wait_time_ms = 0;
    }

    SDL_Event event;
    auto one_on_success = SDL_WaitEventTimeout(&event, wait_time_ms);
    if (one_on_success == 1) {
      switch (event.type) {
      case SDL_EVENT_KEY_DOWN: {
        spdlog::debug(
            "Received SDL key event: {} {} {}", event.key.key,
            event.key.mod, event.key.repeat);
        if (event.key.repeat == 0 and
            event.key.mod bitand SDL_KMOD_CTRL) {
          switch (event.key.key) {
          case 'p': {
            // TODO: Render to a file instead.
            spdlog::debug("Rendering to stdout.");
            auto render_result = render(
                {.camera_position = camera_position,
                 .image_width = 1024u,
                 .image_height = 768u});
            copy_to_stdout(render_result);
            break;
          }
          default:
            break;
          }
        }
        break;
      }
      case SDL_EVENT_QUIT:
        spdlog::debug("Received SDL quit event.");
        return 0;
      case SDL_EVENT_WINDOW_RESIZED:
        spdlog::debug("Received SDL window resize event.");
        spdlog::debug(
            "Pointer {}: old", static_cast<void*>(texture.get()));
        texture.reset();
        spdlog::debug(
            "Pointer {}: new", static_cast<void*>(texture.get()));
        break;
      default:
        break;
      }
      continue;
    }

    frame += 1;
    spdlog::debug("Rendering frame: {}", frame);

    if (not texture) {
      int texture_width = 0;
      int texture_height = 0;
      {
        auto is_success = SDL_GetCurrentRenderOutputSize(
            renderer.get(), &texture_width, &texture_height);
        if (not is_success) {
          spdlog::error(
              "Unable to determine new renderer size: {}",
              SDL_GetError());
          return 1;
        }
      }

      spdlog::debug("Creating texture.");
      texture.reset(SDL_CreateTexture(
          renderer.get(), SDL_PIXELFORMAT_RGB24,
          SDL_TEXTUREACCESS_STREAMING, texture_width, texture_height));
      if (not texture) {
        spdlog::error(
            "Unable to create SDL render texture: {}", SDL_GetError());
        return 1;
      }
      spdlog::debug(
          "Pointer {}: new", static_cast<void*>(texture.get()));
    }

    float texture_width = 0;
    float texture_height = 0;
    auto is_success = SDL_GetTextureSize(
        texture.get(), &texture_width, &texture_height);
    if (not is_success) {
      spdlog::error(
          "Unable to determine SDL render texture size: {}",
          SDL_GetError());
      return 1;
    }

    if (texture_height == 0 or texture_width == 0) {
      spdlog::error(
          "Unable to render to texture of dimension {} x {}",
          texture_width, texture_height);
      return 1;
    }

    // Render output.
    auto render_result = render(
        {.camera_position = camera_position,
         .image_width = static_cast<unsigned int>(texture_width),
         .image_height = static_cast<unsigned int>(texture_height)});

    uint8_t* pixels;
    int pitch;
    {
      auto is_success = SDL_LockTexture(
          texture.get(), nullptr, reinterpret_cast<void**>(&pixels),
          &pitch);
      if (not is_success) {
        spdlog::error(
            "Unable to lock render texture: {}", SDL_GetError());
        return 1;
      }
    }
    auto line_begin = pixels;
    for (auto pixel_index = 0; pixel_index < render_result.pixels.size();
         ++pixel_index) {
      auto color = render_result.pixels[pixel_index];
      pixels[0] = color.red;
      pixels[1] = color.green;
      pixels[2] = color.blue;
      pixels += 3;
      if ((pixel_index + 1) % render_result.image_width == 0) {
        line_begin += pitch;
        pixels = line_begin;
      }
    }
    SDL_UnlockTexture(texture.get());

    {
      auto is_success = SDL_RenderClear(renderer.get());
      if (not is_success) {
        spdlog::error("Unable to clear renderer: {}", SDL_GetError());
        return 1;
      }
    }
    {
      auto is_success = SDL_RenderTexture(
          renderer.get(), texture.get(), nullptr, nullptr);
      if (not is_success) {
        spdlog::error("Unable to copy texture: {}", SDL_GetError());
        return 1;
      }
    }
    SDL_RenderPresent(renderer.get());

    next_refresh_time = now + refresh_interval;
  }
  spdlog::debug("Stopped SDL event loop.");

  return 0;
}

auto main(int argc, char** argv) -> int {
  auto program_name = "raytracing-how";
  auto program_version = "0.0.1";
  argparse::ArgumentParser program(
      program_name, program_version, argparse::default_arguments::none);
  program.add_epilog(
      "Logging level can be changed by setting $SPDLOG_LEVEL"
      " to 'critical', 'error', 'warn', 'info', 'debug' or 'trace'."
      " Override with '--quiet' and '--verbose'.");

  auto help_count = 0;
  program.add_argument("-h", "--help")
      .action([&](const auto&) { ++help_count; })
      .default_value(false)
      .help("Shows help message and exits.")
      .implicit_value(true)
      .nargs(0);

  auto verbose_count = 0;
  program.add_argument("-v", "--verbose")
      .action([&](const auto&) { ++verbose_count; })
      .append()
      .default_value(false)
      .help("Print debugging information. Overrides `$SPDLOG_LEVEL`")
      .implicit_value(true)
      .nargs(0);

  program.add_argument("-q", "--quiet")
      .action([&](const auto&) { --verbose_count; })
      .append()
      .default_value(false)
      .help("Print less debugging information. Stacks with `--verbose`.")
      .implicit_value(true)
      .nargs(0);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  if (help_count > 0) {
    std::cerr << program;
    return 0;
  }

  // Log to stderr.
  auto logger = spdlog::stderr_color_mt("default");
  spdlog::set_default_logger(logger);

  // Set up log level.
  if (program.is_used("--verbose") or program.is_used("--quiet")) {
    // There are 7 levels in total, enumerated in "quietness" order.
    // -   0: trace
    // -   1: debug
    // -   2: info
    // -   3: warn
    // -   4: eror
    // -   5: critical
    // -   6: off
    // Convert from verbosity flags.
    // -   -4: off
    // -   -3: critical
    // -   -2: error
    // -   -1: warn
    // -   0: info, currently default spdlog log level.
    // -   1: debug
    // -   2: trace
    auto default_log_level = spdlog::get_level();
    auto log_level =
        spdlog::level::level_enum{default_log_level - verbose_count};
    if (log_level < spdlog::level::trace) {
      log_level = spdlog::level::trace;
    } else if (log_level >= spdlog::level::off) {
      log_level = spdlog::level::off;
    }
    spdlog::set_level(log_level);
    spdlog::trace(
        "Command line sets verbosity to {}.",
        static_cast<int>(log_level));
  } else {
    // Use environment variable for logging level by default.
    spdlog::cfg::load_env_levels();
  }
  {
    auto log_level = spdlog::get_level();
    spdlog::debug(
        "Logging initialised to level {} ({}).",
        static_cast<int>(log_level),
        spdlog::level::to_string_view(log_level));
  }

  return sdl_main({.window_width = 800, .window_height = 600});
}
