#pragma once

#include "sengi/node.hpp"

#include <memory>
#include <vector>

namespace sengi
{
    class Scene
    {
    public:
        Scene() = default;
        ~Scene() = default;

        Scene(int id, std::string_view scene_name) : id(id), name(scene_name) {}

        void setName(std::string_view scene_name);

        std::string_view getName() const;

        void addNode(std::shared_ptr<Node> node);

        void removeNode(std::shared_ptr<Node> node);

        void removeNodeById(int id);

        void removeAllNodes();

        void removeNodeByName(std::string_view name);

        const std::vector<std::shared_ptr<Node>> &getChildNodes() const;

    private:
        std::vector<std::shared_ptr<Node>> childs;
        std::string name;
        int id;
    };

} // namespace sengi