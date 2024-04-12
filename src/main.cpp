/*
 * Copyright (C) 2021-2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "generated/version_info.hpp"
#include "license.hpp"

#include <cxxopts.hpp>
#include <cxxsemaphore.hpp>
#include <cxxshm.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sys/ioctl.h>
#include <sysexits.h>

int main(int argc, char **argv) {
    const std::string exe_name = std::filesystem::path(*argv).filename().string();
    cxxopts::Options  options(exe_name, "Writes the content of stdin to a named shared memory.");

    auto exit_usage = [&exe_name]() {
        std::cerr << "Use '" << exe_name << " --help' for more information." << '\n';
        exit(EX_USAGE);
    };

    options.add_options("shared memory")("n,name", "shared memory name (mandatory)", cxxopts::value<std::string>());
    options.add_options("settings")(
            "i,invert", "invert all input bits", cxxopts::value<bool>()->default_value("false"));
    options.add_options("settings")("r,repeat",
                                    "repeat input if input size is smaller than shared memory",
                                    cxxopts::value<bool>()->default_value("false"));
    options.add_options("settings")("p,passthrough",
                                    "output everything that is written to the shared memory to stdout",
                                    cxxopts::value<bool>()->default_value("false"));
    options.add_options("shared memory")(
            "s,semaphore",
            "protect the shared memory with an existing named semaphore against simultaneous access",
            cxxopts::value<std::string>());
    options.add_options("other")("h,help", "print usage");
    options.add_options("version information")("version", "print version and exit");
    options.add_options("version information")("longversion",
                                               "print version (including compiler and system info) and exit");
    options.add_options("version information")("shortversion", "print version (only version string) and exit");
    options.add_options("version information")("git-hash", "print git hash");
    options.add_options("other")("license", "show licences");

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
        static constexpr std::size_t MIN_HELP_SIZE = 80;
        if (isatty(STDIN_FILENO)) {
            struct winsize w {};
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {  // NOLINT
                options.set_width(std::max(static_cast<decltype(w.ws_col)>(MIN_HELP_SIZE), w.ws_col));
            }
        } else {
            options.set_width(MIN_HELP_SIZE);
        }
        std::cout << options.help() << '\n';
        std::cout << "This application uses the following libraries:" << '\n';
        std::cout << "  - cxxopts by jarro2783 (https://github.com/jarro2783/cxxopts)" << '\n';
        exit(EX_OK);
    }

    // print version
    if (args.count("shortversion")) {
        std::cout << PROJECT_VERSION << '\n';
        return EX_OK;
    }

    if (args.count("version")) {
        std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << '\n';
        return EX_OK;
    }

    if (args.count("longversion")) {
        std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << '\n';
        std::cout << "   compiled with " << COMPILER_INFO << '\n';
        std::cout << "   on system " << SYSTEM_INFO
#ifndef OS_LINUX
                  << "-nonlinux"
#endif
                  << '\n';
        std::cout << "   from git commit " << RCS_HASH << '\n';
        return EX_OK;
    }

    if (args.count("git-hash")) {
        std::cout << RCS_HASH << '\n';
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
