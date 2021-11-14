#pragma once

#include "scene_storage.hpp"
#include "object.hpp"

void parse_scene(const std::string& file, scene_storage& scene, bool with_textures = true);
