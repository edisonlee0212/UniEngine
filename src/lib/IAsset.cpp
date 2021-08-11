#include <IAsset.hpp>
#include <DefaultResources.hpp>
#include <AssetManager.hpp>
using namespace UniEngine;
void IAsset::Save()
{
    //assert(m_handle >= DefaultResources::GetMaxHandle());
    m_saved = true;
}
void IAsset::Load()
{
    //assert(m_handle() >= DefaultResources::GetMaxHandle());
}
