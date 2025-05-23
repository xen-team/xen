#pragma once

#include <string>
#include <vector>

namespace fastgltf {
class Asset;
}

namespace xen {
class World;
class FilePath;
class Transform;

namespace GltfColliderLoader {

void ComputeGlobalTransforms(fastgltf::Asset const& asset, std::vector<xen::Transform>& out_global_transforms);

void LoadCollidersFromGltf(
    xen::World& world, fastgltf::Asset const& asset, std::vector<xen::Transform> const& global_node_transforms,
    std::string const& proxy_prefix = "PROXY_"
);
}
}