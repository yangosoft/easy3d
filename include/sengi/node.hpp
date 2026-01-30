#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sengi
{

    class Node
    {
    public:
        Node() = default;
        Node(int id, const std::string &name) : id(id), name(name) {}

        // Set and get name
        void set_name(std::string_view new_name);
        std::string_view get_name() const;

        // Set and get position
        void set_position(float x, float y, float z);
        void get_position(float &x, float &y, float &z) const;
        // Set and get rotation (quaternion)
        void set_rotation(float x, float y, float z, float w);
        void get_rotation(float &x, float &y, float &z, float &w) const;
        // Set and get scale
        void set_scale(float sx, float sy, float sz);
        void get_scale(float &sx, float &sy, float &sz) const;

        // Get ID
        int get_id() const;
        void set_id(int new_id);

        // Add child node
        void add_child(std::shared_ptr<Node> child);
        // Remove child node
        void remove_child(std::shared_ptr<Node> child);
        // Get child nodes
        const std::vector<std::shared_ptr<Node>> &get_childs() const;

        virtual ~Node() = default;

    private:
        float position[3]{0.0f, 0.0f, 0.0f};
        float rotation[4]{0.0f, 0.0f, 0.0f, 1.0f}; // Quaternion
        float scale[3]{1.0f, 1.0f, 1.0f};
        int id;
        std::string name;
        std::vector<std::shared_ptr<Node>> childs;
    };
} // namespace sengi