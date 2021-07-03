#include <Animation.hpp>
using namespace UniEngine;

std::shared_ptr<Bone> & Animation::UnsafeGetRootBone()
{
    return m_rootBone;
}

void Animation::OnGui()
{
}

Animation::Animation()
{
}

Animation::~Animation()
{
}
