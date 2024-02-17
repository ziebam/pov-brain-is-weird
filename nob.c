#define NOB_IMPLEMENTATION
#include "./nob.h"

static const char *raylibModules[] = {
    "raudio",
    "rcore",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};

bool buildRaylib() {
    bool result = true;

    Nob_Cmd cmd = {0};
    Nob_File_Paths objectFiles = {0};
    Nob_Procs procs = {0};

    if (!nob_mkdir_if_not_exists("./build/raylib")) nob_return_defer(false);

    const char *buildPath = "./build/raylib/gcc";
    if (!nob_mkdir_if_not_exists(buildPath)) nob_return_defer(false);

    for (size_t i = 0; i < NOB_ARRAY_LEN(raylibModules); i++) {
        const char *inputPath = nob_temp_sprintf("./raylib/raylib-5.0/src/%s.c", raylibModules[i]);
        const char *outputPath = nob_temp_sprintf("%s/%s.o", buildPath, raylibModules[i]);

        nob_da_append(&objectFiles, outputPath);

        if (nob_needs_rebuild(outputPath, &inputPath, 1)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, "gcc");
            nob_cmd_append(&cmd, "-DPLATFORM_DESKTOP", "-fPIC");
            nob_cmd_append(&cmd, "-I./raylib/raylib-5.0/src/external/glfw/include");
            nob_cmd_append(&cmd, "-c", inputPath);
            nob_cmd_append(&cmd, "-o", outputPath);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&procs, proc);
        }
    }
    if (!nob_procs_wait(procs)) nob_return_defer(false);

    cmd.count = 0;

    const char *libraylibPath = nob_temp_sprintf("%s/libraylib.a", buildPath);

    if (nob_needs_rebuild(libraylibPath, objectFiles.items, objectFiles.count)) {
        nob_cmd_append(&cmd, "ar", "-crs", libraylibPath);
        for (size_t i = 0; i < NOB_ARRAY_LEN(raylibModules); ++i) {
            const char *inputPath = nob_temp_sprintf("%s/%s.o", buildPath, raylibModules[i]);
            nob_cmd_append(&cmd, inputPath);
        }
        if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }

defer:
    nob_cmd_free(cmd);
    nob_da_free(objectFiles);
    nob_da_free(procs);
    return result;
}

bool buildPovBrainIsWeird(bool platformWindows) {
    bool result = true;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc");

    if (platformWindows) nob_cmd_append(&cmd, "-mwindows");

    nob_cmd_append(&cmd, "-Wall", "-Wextra");
    nob_cmd_append(&cmd, "-I./build/");
    nob_cmd_append(&cmd, "-I./raylib/raylib-5.0/src/");
    nob_cmd_append(&cmd, "-o", "./build/pov-brain-is-weird");
    nob_cmd_append(&cmd, "./pov-brain-is-weird.c");
    nob_cmd_append(&cmd, "-L./build/raylib/gcc");
    nob_cmd_append(&cmd, "-l:libraylib.a");
    nob_cmd_append(&cmd, "-lm");  // needed on Linux, doesn't cause issues on Windows

    if (platformWindows) {
        nob_cmd_append(&cmd, "-lwinmm", "-lgdi32");
        nob_cmd_append(&cmd, "-static");
    }

    if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);

defer:
    nob_cmd_free(cmd);
    return result;
}

void print_usage(void) {
    nob_log(NOB_INFO, "usage: [./]nob [-platform] [platform]");
    nob_log(NOB_INFO, "platforms supported: windows, linux");
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    bool platformWindows = true;
    if (argc != 1) {
        if (strcmp(argv[1], "-platform") == 0) {
            if (strcmp(argv[2], "linux") == 0) {
                platformWindows = false;
            } else if (strcmp(argv[2], "windows") == 0) {
                platformWindows = true;
            } else {
                nob_log(NOB_ERROR, "unsupported platform\n");
                print_usage();
                return 1;
            }
        } else {
            print_usage();
            return 1;
        }
    }

    if (!nob_mkdir_if_not_exists("build")) return 1;

    if (!buildRaylib()) return 1;
    if (!buildPovBrainIsWeird(platformWindows)) return 1;

    return 0;
}
