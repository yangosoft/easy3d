#include "sengi/scene.hpp"

using namespace sengi;

void Scene::addNode(std::shared_ptr<Node> node)
{
    childs.push_back(node);
}

void Scene::removeNode(std::shared_ptr<Node> node)
{
    childs.erase(std::remove(childs.begin(), childs.end(), node), childs.end());
}

void Scene::removeNodeById(int id)
{
    childs.erase(std::remove_if(childs.begin(), childs.end(),
                                [id](const std::shared_ptr<Node> &node)
                                { return node->get_id() == id; }),
                 childs.end());
}

void Scene::removeAllNodes()
{
    childs.clear();
}

void Scene::removeNodeByName(std::string_view name)
{
    childs.erase(std::remove_if(childs.begin(), childs.end(),
                                [name](const std::shared_ptr<Node> &node)
                                { return node->get_name() == name; }),
                 childs.end());
}

const std::vector<std::shared_ptr<Node>> &Scene::getChildNodes() const
{
    return childs;
}

void Scene::setName(std::string_view scene_name)
{
    name = scene_name;
}

std::string_view Scene::getName() const
{
    return name;
}