//
// Created by konst on 26.11.2026.
//


#include "../site_path.h"
#include <iostream>

int main() {
    const std::filesystem::path root = "C:/site/www";

    if (!is_inside_root(root, "C:/site/www/docs/index.html")) {
        std::cerr << "A path inside www was rejected\n";
        return 1;
    }

    if (is_inside_root(root, "C:/site/www_other/index.html")) {
        std::cerr << "A path outside www was accepted\n";
        return 1;
    }

    std::cout << "Root boundary checks (OK)\n";
    return 0;
}