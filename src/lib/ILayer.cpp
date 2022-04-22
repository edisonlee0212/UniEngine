//
// Created by lllll on 4/22/2022.
//
#include "ILayer.hpp"
#include "Scene.hpp"

using namespace UniEngine;

std::shared_ptr<Scene> ILayer::GetScene()
{
    return m_scene.lock();
}