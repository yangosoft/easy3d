#include "sengi/node.hpp"

using namespace sengi;

void Node::set_name(std::string_view new_name)
{
    name = new_name;
}

std::string_view Node::get_name() const
{
    return name;
}

void Node::set_position(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
}

void Node::get_position(float &x, float &y, float &z) const
{
    x = position[0];
    y = position[1];
    z = position[2];
}

void Node::set_rotation(float x, float y, float z, float w)
{
    rotation[0] = x;
    rotation[1] = y;
    rotation[2] = z;
    rotation[3] = w;
}

void Node::get_rotation(float &x, float &y, float &z, float &w) const
{
    x = rotation[0];
    y = rotation[1];
    z = rotation[2];
    w = rotation[3];
}

void Node::set_scale(float sx, float sy, float sz)
{
    scale[0] = sx;
    scale[1] = sy;
    scale[2] = sz;
}

void Node::get_scale(float &sx, float &sy, float &sz) const
{
    sx = scale[0];
    sy = scale[1];
    sz = scale[2];
}

int Node::get_id() const
{
    return id;
}

void Node::set_id(int new_id)
{
    id = new_id;
}

void Node::add_child(std::shared_ptr<Node> child)
{
    childs.push_back(child);
}

void Node::remove_child(std::shared_ptr<Node> child)
{
    childs.erase(std::remove(childs.begin(), childs.end(), child), childs.end());
}

const std::vector<std::shared_ptr<Node>> &Node::get_childs() const
{
    return childs;
}
