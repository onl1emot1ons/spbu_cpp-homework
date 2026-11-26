//
// Created by konst on 26.11.2026.
//

#ifndef HW5_SITE_PATH_H
#define HW5_SITE_PATH_H

#endif //HW5_SITE_PATH_H


#pragma once

#include <algorithm>
#include <filesystem>

inline bool is_inside_root(const std::filesystem::path& root,
                           const std::filesystem::path& candidate) {
    const auto match = std::mismatch(
        root.begin(), root.end(),
        candidate.begin(), candidate.end()
    );
    return match.first == root.end();
}