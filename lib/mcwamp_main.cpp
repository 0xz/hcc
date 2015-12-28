//===----------------------------------------------------------------------===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include <cassert>
#include "clamp-config.hxx"
/* Flag set by ‘--verbose’. */
static int verbose_flag;
static bool build_mode = false, install_mode = true; // use install mode by default

static bool amp_mode = true, hcc_mode = false;

static bool bolt_rewrite_mode = false;

void replace(std::string& str,
        const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    while(start_pos != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos = str.find(from);
    }
}

void cxxflags(void) {
    if (!build_mode && !install_mode) {
        std::cerr << "Please specify --install or --build mode before flags\n";
        abort();
    }

    if (hcc_mode) {
        std::cout << " -hc";
    }

    // Common options
    std::cout << " -std=c++amp -stdlib=libc++";

    // clamp
    if (build_mode) {
        std::cout << " -I" CMAKE_CLAMP_INC_DIR;

        // bolt and boost
        if (bolt_rewrite_mode) {
            std::cout << " -I" CMAKE_BOLT_SRC_INC_DIR;
            std::cout << " -I" CMAKE_BOLT_BIN_INC_DIR;
            std::cout << " -I" CMAKE_BOOST_INC_DIR;
        }
    } else if (install_mode) {
        if (const char *p = getenv("HCC_HOME")) {
            std::cout << " -I" << p << "/include";

            // bolt and boost
            if (bolt_rewrite_mode) {
                std::cout << " -I" << p << "/include/Bolt";
                std::cout << " -I" << p << "/include/Boost";
            }
        } else {
            std::cout << " -I" CMAKE_INSTALL_INC;
    
            // bolt and boost
            if (bolt_rewrite_mode) {
                std::cout << " -I" CMAKE_INSTALL_BOLT_INC;
                std::cout << " -I" CMAKE_INSTALL_BOOST_INC;
            }
        }
    } else {
        assert(0 && "Unreacheable!");
    }

    std::cout << std::endl;
}

void ldflags(void) {
    if (hcc_mode) {
        std::cout << " -hc";
    }

    // Common options
    std::cout << " -std=c++amp";

    if (build_mode) {
        std::cout << " -L" CMAKE_AMPCL_LIB_DIR;

        if (bolt_rewrite_mode) {
            std::cout << " -L" CMAKE_BOLT_LIB_DIR;
            std::cout << " -L" CMAKE_BOOST_LIB_DIR;
        }

        std::cout << " -Wl,--rpath="
            CMAKE_AMPCL_LIB_DIR;

        if (bolt_rewrite_mode) {
            std::cout << ":" CMAKE_BOLT_LIB_DIR ":"
                         CMAKE_BOOST_LIB_DIR ;
        }
    } else if (install_mode) {
        if (const char *p = getenv("HCC_HOME")) {
            std::cout << " -L" << p << "/lib";
            std::cout << " -Wl,--rpath=" << p << "/lib";
        } else {
            std::cout << " -L" CMAKE_INSTALL_LIB;
            std::cout << " -Wl,--rpath=" CMAKE_INSTALL_LIB;
        }
    }

    std::cout << " -lc++ -lc++abi -ldl -lpthread ";
    if (bolt_rewrite_mode) {
        std::cout << "-lampBolt.runtime.clang ";
    }
    if (const char *p = getenv("TEST_CPU"))
        if (p == std::string("ON"))
        std::cout << " -lmcwamp_atomic ";
    std::cout << "-Wl,--whole-archive -lmcwamp -Wl,--no-whole-archive ";
}

void prefix(void) {
    if (const char *p = getenv("HCC_HOME")) {
        std::cout << p;
    } else {
        std::cout << CMAKE_INSTALL_PREFIX;
    }
}

// Support to perform google unit testing
void gtest(void) {
    if (build_mode) {
       std::cout << " -I" CMAKE_GTEST_INC_DIR;
       std::cout << " -L" CMAKE_AMPCL_LIB_DIR;
       std::cout << " -lmcwamp_gtest ";
    }
    else if (install_mode) {
       //Flags set using cxxflags and ldflsgs suffice
       std::cout << " -lmcwamp_gtest ";
    }
          
}

// Compiling as a shared library
void shared(void) {
#ifndef __APPLE__
    std::cout << " -shared -fPIC -Wl,-Bsymbolic ";
#else
#error Does not support Mac OS X
#endif
}

int main (int argc, char **argv) {
    if (std::string(argv[0]).find("hcc-config") != std::string::npos) {
        hcc_mode = true; amp_mode = false;
    } else {
        hcc_mode = false; amp_mode = true;
    }

    int c;
    while (1)
    {
        static struct option long_options[] =
        {
            /* These options set a flag. */
            {"verbose", no_argument,       &verbose_flag, 1},
            {"brief",   no_argument,       &verbose_flag, 0},
            /* These options don't set a flag.
               We distinguish them by their indices. */
            {"cxxflags", no_argument,       0, 'a'},
            {"build",    no_argument,       0, 'b'},
            {"install",  no_argument,       0, 'i'},
            {"ldflags",  no_argument,       0, 'l'},
            {"prefix",  no_argument,       0, 'p'},
            {"bolt",  no_argument,       0, 'o'},
            {"shared",  no_argument,       0, 's'},
            {"gtest",  no_argument,       0, 't'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "",
                long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'a':   // --cxxflags
                cxxflags();
                break;
            case 'l':   // --ldflags
                ldflags();
                break;
            case 'p':   // --prefix
                prefix();
                break;
            case 's':   // --shared
                shared();
                break;
            case 'b':   // --build
                build_mode = true;
                install_mode = false;
                break;
            case 'i':   // --install
                build_mode = false;
                install_mode = true;
                break;
            case 'o':   // --bolt
                bolt_rewrite_mode = true;
                break;
            case 't':   // --gtest
                gtest();
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort ();
        }
    }

    /* Instead of reporting ‘--verbose’
       and ‘--brief’ as they are encountered,
       we report the final status resulting from them. */
    if (verbose_flag)
        puts ("verbose flag is set");

    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

    exit (0);
}
