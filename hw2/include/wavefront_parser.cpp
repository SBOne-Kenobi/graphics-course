#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>

#include "lodepng.h"

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
    std::string albedo_file;
    std::string ao_file;
    std::string specular_file;
    glm::vec3 specular_color;
    float specular_power;
};

std::unordered_map<std::string, mtl_items> get_mtl(const std::string& path) {
    std::ifstream in(path, std::ios_base::in);
    std::string line;

    std::string dir = get_dir(path);
    std::string current;

    std::unordered_map<std::string, mtl_items> result;

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
            result[current].specular_file = img_path;
            continue;
        }

        if (cmd == "map_bump") {
            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string img_path = dir;
            img_path += "/";
            img_path += name;
            result[current].ao_file = img_path;
            continue;
        }

        if (cmd == "map_Ka") {
            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string img_path = dir;
            img_path += "/";
            img_path += name;
            result[current].albedo_file = img_path;
            continue;
        }
    }

    return result;
}

void parse_scene(const std::string& file, scene_storage& scene, bool with_textures) {
    std::ifstream in(file, std::ios_base::in);

    std::string dir = get_dir(file);
    std::unordered_map<std::string, mtl_items> mtl;

    std::string line;

    std::string current;
    GLuint albedo_texture = -1;
    GLuint ao_map = -1;
    GLuint specular_map = -1;
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
            if (with_textures && albedo_texture != (GLuint) -1) {
                obj.with_albedo_texture(albedo_texture);
            }
            if (with_textures && ao_map != (GLuint) -1) {
                obj.with_ao_map(ao_map);
            }
            if (with_textures && specular_map != (GLuint) - 1) {
                obj.with_specular_map(specular_map);
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
            texcoords.emplace_back(u, 1.0 - v);
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
            std::string name;
            str >> name;
            std::replace(name.begin(), name.end(), '\\', '/');
            std::string path = dir;
            path += "/";
            path += name;
            mtl = get_mtl(path);
            continue;
        }

        if (cmd == "usemtl") {
            str >> current;
            auto& mtli = mtl[current];
            specular_color = mtli.specular_color;
            specular_power = mtli.specular_power;

            if (with_textures) {
                std::vector<unsigned char> image;
                unsigned width, height;

                if (!mtli.specular_file.empty()) {
                    lodepng::decode(image, width, height, mtli.specular_file);

                    glGenTextures(1, &specular_map);
                    glBindTexture(GL_TEXTURE_2D, specular_map);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexImage2D(
                        GL_TEXTURE_2D, 0, GL_RGBA,
                        width, height, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, image.data()
                    );
                    glGenerateMipmap(GL_TEXTURE_2D);
                } else {
                    specular_map = -1;
                }

                image.clear();

                if (!mtli.albedo_file.empty()) {
                    lodepng::decode(image, width, height, mtli.albedo_file);

                    glGenTextures(1, &albedo_texture);
                    glBindTexture(GL_TEXTURE_2D, albedo_texture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexImage2D(
                        GL_TEXTURE_2D, 0, GL_RGBA,
                        width, height, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, image.data()
                    );
                    glGenerateMipmap(GL_TEXTURE_2D);
                } else {
                    albedo_texture = -1;
                }

                image.clear();

                if (!mtli.ao_file.empty()) {
                    lodepng::decode(image, width, height, mtli.ao_file);

                    glGenTextures(1, &ao_map);
                    glBindTexture(GL_TEXTURE_2D, ao_map);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexImage2D(
                        GL_TEXTURE_2D, 0, GL_RGBA,
                        width, height, 0,
                        GL_RGBA, GL_UNSIGNED_BYTE, image.data()
                    );
                    glGenerateMipmap(GL_TEXTURE_2D);
                } else {
                    ao_map = -1;
                }

            }
            continue;
        }

    }
    in.close();
    add_object();

    std::cout << "Parse finished" << std::endl;
}
