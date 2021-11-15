#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "wavefront_parser.hpp"

struct hash {
    std::size_t operator()(const std::tuple<std::size_t, std::size_t, std::size_t>& k) const {
        auto [a, b, c] = k;
        return a * 1009 * 1009 + b * 1009 + c;
    }
};

std::string get_dir(const std::string& path) {
    auto last_slash_idx = path.rfind('/');
    if (std::string::npos != last_slash_idx) {
        return path.substr(0, last_slash_idx);
    }
    return "";
}

struct mtl_items {
    GLuint albedo_texture = -1;
    GLuint specular_map = -1;
    GLuint norm_map = -1;
    GLuint mask = -1;
    glm::vec3 specular_color{};
    float specular_power{};
};

std::unordered_map<std::string, mtl_items> get_mtl(const std::string& path, bool with_textures) {
    std::ifstream in(path, std::ios_base::in);
    std::string line;

    std::string dir = get_dir(path);
    std::string current;

    std::unordered_map<std::string, mtl_items> result;

    unsigned char *image;
    int width, height, channels;

    while (std::getline(in, line)) {
        std::istringstream str(line);

        std::string cmd;

        if (!(str >> cmd)) {
            continue;
        }

        if (cmd[0] == '#') {
            continue;
        }

        if (cmd == "newmtl") {
            str >> current;
            continue;
        }

        if (cmd == "Ns") {
            float sp;
            str >> sp;
            result[current].specular_power = sp;
            continue;
        }

        if (cmd == "Ks") {
            float r, g, b;
            str >> r >> g >> b;
            result[current].specular_color = {r, g, b};
            continue;
        }

        if (cmd == "map_Ks") {
            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string img_path = dir;
            img_path += "/";
            img_path += name;

            auto& mtli = result[current];

            if (with_textures) {
                image = stbi_load(img_path.c_str(), &width, &height, &channels, 3);

                glGenTextures(1, &mtli.specular_map);
                glBindTexture(GL_TEXTURE_2D, mtli.specular_map);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGB,
                    width, height, 0,
                    GL_RGB, GL_UNSIGNED_BYTE, image
                );
                glGenerateMipmap(GL_TEXTURE_2D);

                stbi_image_free(image);
            }

            continue;
        }

        if (cmd == "norm") {
            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string img_path = dir;
            img_path += "/";
            img_path += name;

            auto& mtli = result[current];

            if (with_textures) {
                image = stbi_load(img_path.c_str(), &width, &height, &channels, 3);

                glGenTextures(1, &mtli.norm_map);
                glBindTexture(GL_TEXTURE_2D, mtli.norm_map);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGB,
                    width, height, 0,
                    GL_RGB, GL_UNSIGNED_BYTE, image
                );
                glGenerateMipmap(GL_TEXTURE_2D);

                stbi_image_free(image);
            }

            continue;
        }

        if (cmd == "map_Ka") {
            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string img_path = dir;
            img_path += "/";
            img_path += name;

            auto& mtli = result[current];

            if (with_textures) {
                image = stbi_load(img_path.c_str(), &width, &height, &channels, 3);

                glGenTextures(1, &mtli.albedo_texture);
                glBindTexture(GL_TEXTURE_2D, mtli.albedo_texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGB,
                    width, height, 0,
                    GL_RGB, GL_UNSIGNED_BYTE, image
                );
                glGenerateMipmap(GL_TEXTURE_2D);

                stbi_image_free(image);
            }

            continue;
        }

        if (cmd == "map_d") {

            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string img_path = dir;
            img_path += "/";
            img_path += name;

            auto& mtli = result[current];

            if (with_textures) {
                image = stbi_load(img_path.c_str(), &width, &height, &channels, 3);

                glGenTextures(1, &mtli.mask);
                glBindTexture(GL_TEXTURE_2D, mtli.mask);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGB,
                    width, height, 0,
                    GL_RGB, GL_UNSIGNED_BYTE, image
                );
                glGenerateMipmap(GL_TEXTURE_2D);

                stbi_image_free(image);
            }
        }

    }

    return result;
}

void parse_scene(const std::string& file, scene_storage& scene, bool with_textures) {
    stbi_set_flip_vertically_on_load(1);

    std::ifstream in(file, std::ios_base::in);

    std::string dir = get_dir(file);
    std::unordered_map<std::string, mtl_items> mtl;

    std::string line;

    std::string current;
    GLuint albedo_texture = -1;
    GLuint specular_map = -1;
    GLuint norm_map = -1;
    GLuint mask = -1;
    glm::vec3 specular_color;
    float specular_power;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    std::vector<vertex> vertices;
    std::vector<int> indices;
    std::unordered_map<std::tuple<std::size_t, std::size_t, std::size_t>, int, hash> ids;

    auto add_object = [&]() {
        if (!indices.empty()) {
            object obj = object(std::move(vertices), specular_color, specular_power)
                .with_indices(std::move(indices));
            if (albedo_texture != (GLuint) -1) {
                obj.with_albedo_texture(albedo_texture);
            }
            if (specular_map != (GLuint) -1) {
                obj.with_specular_map(specular_map);
            }
            if (norm_map != (GLuint) -1) {
                obj.with_norm_map(norm_map);
            }
            if (mask != (GLuint) -1) {
                obj.with_mask(mask);
            }
            scene.add_object(std::move(obj));
            indices.clear();
            vertices.clear();
            ids.clear();
        }
    };

    while (std::getline(in, line)) {
        std::replace(line.begin(), line.end(), '/', ' ');
        std::istringstream str(line);
        std::string cmd;

        if (!(str >> cmd)) {
            continue;
        }
        if (cmd[0] == '#') {
            continue;
        }
        if (cmd == "v") {
            float x, y, z;
            str >> x >> y >> z;
            if (float w; str >> w) {
                x /= w;
                y /= w;
                z /= w;
            }
            positions.emplace_back(x, y, z);
            continue;
        }

        if (cmd == "vn") {
            float x, y, z;
            str >> x >> y >> z;
            normals.emplace_back(x, y, z);
            continue;
        }


        if (cmd == "vt") {
            float u, v;
            str >> u >> v;
            texcoords.emplace_back(u, v);
            continue;
        }

        if (cmd == "vp") {
            // there is not in sponza
            continue;
        }

        if (cmd == "f") {

            auto get_next = [&]() {
                std::size_t id_p, id_n, id_t;
                if (str >> id_p >> id_t >> id_n) {
                    id_p--;
                    id_t--;
                    id_n--;

                    auto k = std::make_tuple(id_p, id_n, id_t);

                    if (!ids.contains(k)) {
                        ids[k] = vertices.size();
                        vertex v{};
                        v.position = positions[id_p];
                        v.normal = normals[id_n];
                        v.texcoord = texcoords[id_t];
                        vertices.push_back(v);
                    }
                    return ids[k];
                }
                return -1;
            };

            int id0 = get_next();
            int id1 = get_next();
            int id2 = get_next();
            while (id2 != -1) {
                indices.push_back(id0);
                indices.push_back(id1);
                indices.push_back(id2);
                id1 = id2;
                id2 = get_next();
            }

            continue;
        }

        if (cmd == "l") {
            // there is not in sponza
            continue;
        }

        if (cmd == "g") {
            add_object();
            continue;
        }

        if (cmd == "mtllib") {
            add_object();
            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string path = dir;
            path += "/";
            path += name;
            mtl = get_mtl(path, with_textures);
            continue;
        }

        if (cmd == "usemtl") {
            add_object();
            str >> current;
            auto& mtli = mtl[current];
            specular_color = mtli.specular_color;
            specular_power = mtli.specular_power;
            specular_map = mtli.specular_map;
            albedo_texture = mtli.albedo_texture;
            norm_map = mtli.norm_map;
            mask = mtli.mask;
            continue;
        }

    }
    in.close();
    add_object();

    std::cout << "Parse finished" << std::endl;
}
