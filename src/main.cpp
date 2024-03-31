/*
 * Copyright (C) 2021-2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "license.hpp"

#include <cxxopts.hpp>
#include <cxxsemaphore.hpp>
#include <cxxshm.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sysexits.h>

int main(int argc, char **argv) {
    const std::string exe_name = std::filesystem::path(*argv).filename().string();
    cxxopts::Options  options(exe_name, "Writes the content of stdin to a named shared memory.");

    auto exit_usage = [&exe_name]() {
        std::cerr << "Use '" << exe_name << " --help' for more information." << '\n';
        exit(EX_USAGE);
    };

    options.add_options()("n,name", "shared memory name (mandatory)", cxxopts::value<std::string>());
    options.add_options()("i,invert", "invert all input bits", cxxopts::value<bool>()->default_value("false"));
    options.add_options()("r,repeat",
                          "repeat input if input size is smaller than shared memory",
                          cxxopts::value<bool>()->default_value("false"));
    options.add_options()("p,passthrough",
                          "output everything that is written to the shared memory to stdout",
                          cxxopts::value<bool>()->default_value("false"));
    options.add_options()("s,semaphore",
                          "protect the shared memory with an existing named semaphore against simultaneous access",
                          cxxopts::value<std::string>());
    options.add_options()("h,help", "print usage");
    options.add_options()("version", "print version information");
    options.add_options()("license", "show licenses");

    // parse arguments
    cxxopts::ParseResult args;
    try {
        args = options.parse(argc, argv);
    } catch (cxxopts::exceptions::parsing::exception &e) {
        std::cerr << "Failed to parse arguments: " << e.what() << '.' << '\n';
        exit_usage();
    }

    const auto REPEAT_INPUT = args["repeat"].as<bool>();
    const auto PASSTHROUGH  = args["passthrough"].as<bool>();

    // print usage
    if (args.count("help")) {
        std::cout << options.help() << '\n';
        std::cout << "This application uses the following libraries:" << '\n';
        std::cout << "  - cxxopts by jarro2783 (https://github.com/jarro2783/cxxopts)" << '\n';
        exit(EX_OK);
    }

    // print version
    if (args.count("version")) {
        std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << " (compiled with " << COMPILER_INFO << " on "
                  << SYSTEM_INFO << ')' << '\n';
        return EX_OK;
    }

    // print licenses
    if (args.count("license")) {
        print_licenses(std::cout);
        exit(EX_OK);
    }

    // open shm
    std::unique_ptr<cxxshm::SharedMemory> shm;
    try {
        shm = std::make_unique<cxxshm::SharedMemory>(args["name"].as<std::string>());
    } catch (const std::system_error &e) {
        std::cerr << e.what() << '\n';
        exit(EX_OSERR);
    } catch (const cxxopts::exceptions::option_has_no_value::exception &) {
        std::cerr << "Specifying an shared memory name is mandatory." << '\n';
        exit_usage();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        exit(EX_SOFTWARE);
    }

    std::unique_ptr<cxxsemaphore::Semaphore> semaphore;
    if (args.count("semaphore")) {
        try {
            semaphore = std::make_unique<cxxsemaphore::Semaphore>(args["semaphore"].as<std::string>());
        } catch (std::exception &e) {
            std::cerr << e.what() << '\n';
            return EX_SOFTWARE;
        }
    }

    const auto SHM_SIZE = shm->get_size();

    // allocate input buffer (if required for repeat option)
    std::vector<char> in_buffer;
    if (REPEAT_INPUT) in_buffer = std::vector<char>(SHM_SIZE);

    auto *shm_data = shm->get_addr<char *>();

    const int INVERT_MASK = args["invert"].as<bool>() ? ~0 : 0;

    if (semaphore) semaphore->wait();

    // copy file to shm (and buffer)
    std::size_t remaining = SHM_SIZE;
    std::size_t pos       = 0;
    while (remaining) {
        auto c = std::cin.get();
        if (c == EOF) break;

        c ^= INVERT_MASK;

        shm_data[pos] = static_cast<char>(c);  // NOLINT
        if (PASSTHROUGH) {
            std::cout.put(static_cast<char>(c));
            std::cout.flush();
        }
        if (REPEAT_INPUT) in_buffer[pos] = static_cast<char>(c);

        pos++;
        remaining--;
    }

    // copy buffered value to shm (if repeat is enabled and there war at least one input character)
    if (remaining && REPEAT_INPUT && pos) {
        const auto BUF_SIZE = pos;
        while (remaining) {
            const auto COPY_SIZE = std::min(BUF_SIZE, remaining);

            memcpy(shm_data + pos, in_buffer.data(), COPY_SIZE);  // NOLINT
            if (PASSTHROUGH) {
                std::cout.write(in_buffer.data(), static_cast<std::streamsize>(COPY_SIZE));
                std::cout.flush();
            }

            pos += COPY_SIZE;
            remaining -= COPY_SIZE;
        }
    }

    if (semaphore) semaphore->post();
}
