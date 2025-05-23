#include "bvh_format.hpp"

#include <animation/skeleton.hpp>
#include <utils/filepath.hpp>
#include <utils/str_utils.hpp>

#include <tracy/Tracy.hpp>

namespace {
void load_joint(
    std::ifstream& file, std::unordered_map<std::string, xen::SkeletonJoint&>& joints, xen::Skeleton& skeleton,
    xen::SkeletonJoint& parent_joint
)
{
    ZoneScopedN("[BvhFormat]::load_joint");

    std::string token;
    file >> token;

    if (token.front() == 'E') {    // "End Site" declaration, stopping recursion
        std::getline(file, token); // "Site"
        std::getline(file, token); // Opening scope '{'

        file >> token;
        if (token.front() != 'O') // "OFFSET"
            throw std::invalid_argument("Error: Invalid BVH joint offset");

        xen::Vector3f offset;
        file >> offset.x >> offset.y >> offset.z;
        std::getline(file, token);

        // TODO: use the last offset?

        file >> token;
        if (token.front() != '}')
            throw std::invalid_argument("Error: Invalid BVH joint closing scope");
        std::getline(file, token);

        return;
    }

    if (token.front() != 'J') // "JOINT"
        throw std::invalid_argument("Error: Invalid BVH joint declaration");

    file >> token;
    xen::SkeletonJoint& currentJoint = joints.emplace(token, skeleton.add_node()).first->second;
    currentJoint.add_parents(parent_joint);

    std::getline(file, token);

    file >> token;
    if (token.front() != '{')
        throw std::invalid_argument("Error: Invalid BVH joint opening scope");
    std::getline(file, token);

    file >> token;
    if (token.front() != 'O') // "OFFSET"
        throw std::invalid_argument("Error: Invalid BVH joint offset");

    xen::Vector3f jointOffset;
    file >> jointOffset.x >> jointOffset.y >> jointOffset.z;
    currentJoint.set_translation(jointOffset);
    std::getline(file, token);

    file >> token;
    if (token.front() != 'C') // "CHANNELS"
        throw std::invalid_argument("Error: Invalid BVH joint channels");

    file >> token;
    if (token.front() != '3')
        throw std::invalid_argument("Error: Invalid BVH joint channel count");
    std::getline(file, token);

    load_joint(file, joints, skeleton, currentJoint);

    file >> token;
    if (token.front() != '}')
        throw std::invalid_argument("Error: Invalid BVH joint closing scope");
    std::getline(file, token);

    while (!file.eof()) {
        file >> std::ws; // Discarding all leading white spaces

        if (file.peek() == '}') // Reaching the end of scope, no other joint is declared on this level
            break;

        load_joint(file, joints, skeleton, parent_joint);
    }
}
}

namespace xen::BvhFormat {
/*
Skeleton load(FilePath const& filepath)
{
ZoneScopedN("BvhFormat::load");
ZoneTextF("Path: %s", filepath.to_utf8().c_str());

std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);

if (!file) {
    throw std::invalid_argument("Error: Could not open the BVH file '" + filepath + "'");
}

std::string const format = StrUtils::to_lower_copy(filepath.recover_extension().to_utf8());

if (format != "bvh") {
    throw std::invalid_argument("Error: '" + filepath + "' doesn't have a .bvh extension");
}

std::string token;

file >> token;
if (token != "HIERARCHY") {
    throw std::invalid_argument("Error: Invalid BVH header");
}

file >> token;
if (token != "ROOT") {
    throw std::invalid_argument("Error: Invalid BVH root joint");
}

Skeleton skeleton;
std::unordered_map<std::string, SkeletonJoint&> joints;

file >> token;
SkeletonJoint& root_joint = joints.emplace(token, skeleton.add_node()).first->second;

std::getline(file, token); // Skipping the rest of the line

file >> token;
if (token.front() != '{') {
    throw std::invalid_argument("Error: Invalid BVH root joint opening scope");
}
std::getline(file, token);

file >> token;
if (token.front() != 'O') { // "OFFSET"
    throw std::invalid_argument("Error: Invalid BVH root joint offset");
}

Vector3f root_pos;
file >> root_pos.x >> root_pos.y >> root_pos.z;
root_joint.set_translation(root_pos);
std::getline(file, token);

file >> token;
if (token.front() != 'C') { // "CHANNELS"
    throw std::invalid_argument("Error: Invalid BVH root joint channels");
}

file >> token;
if (token.front() != '6') {
    throw std::invalid_argument("Error: Invalid BVH root joint channel count");
}
std::getline(file, token);

load_joint(file, joints, skeleton, root_joint);
std::getline(file, token); // Root joint's closing scope

// TODO: import animation

return skeleton;
}
*/
}