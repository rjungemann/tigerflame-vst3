// TigerFlame - Test Main
// Entry point for Catch2 tests

#include "catch2/catch_all.hpp"

// This file is just a placeholder - Catch2 v3 uses its own main
// when linked with Catch2::Catch2WithMain

int main(int argc, char* argv[]) {
    // This should never be called directly when using Catch2::Catch2WithMain
    // But we provide it for completeness
    return Catch::Session().run(argc, argv);
}
