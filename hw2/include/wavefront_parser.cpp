#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "wavefront_parser.hpp"

struct hash {
    std::size_t operator()(const std::tuple<std::size_t, std::size_t, std::size_t>& k) const {
        auto [a, b, c] = k;
        return a * 1009 * 1009 + b * 1009 + c;
    }
};

object parse_object(const std::string &file) {
    std::ifstream in(file, std::ios_base::in);

    std::string line;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    std::vector<vertex> vertices;
    std::vector<int> indices;

    std::unordered_map<std::tuple<std::size_t, std::size_t, std::size_t>, int, hash> ids;

    while (std::getline(in, line)) {
        std::replace(line.begin(), line.end(), '/', ' ');
        std::istringstream str(line);
        std::string cmd;

        str >> cmd;
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
    }

    return object(vertices, indices);
}
