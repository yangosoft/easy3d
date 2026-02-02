#pragma once

#include "sengi/node.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace sengi
{
    using SharedPtrNode = std::shared_ptr<Node>;
    class Scene
    {
    public:
        Scene() = default;
        ~Scene() = default;

        Scene(int id, std::string_view scene_name) : id(id), name(scene_name) {}

        void setName(std::string_view scene_name);

        std::string_view getName() const;

        void addNode(SharedPtrNode node);

        void removeNode(SharedPtrNode node);

        void removeNodeById(int id);

        void removeAllNodes();

        void removeNodeByName(std::string_view name);

        const std::vector<SharedPtrNode> &getChildNodes() const;

        void applyToAllNodes(std::function<void(SharedPtrNode)> func);
        

    private:
        std::vector<SharedPtrNode> childs;
        std::string name;
        int id;
    };

} // namespace sengi