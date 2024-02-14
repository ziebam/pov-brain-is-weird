#define NOB_IMPLEMENTATION
#include "./nob.h"

static const char *raylib_modules[] = {
    "raudio",
    "rcore",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};

bool build_raylib()
{
    bool result = true;

    Nob_Cmd cmd = {0};
    Nob_File_Paths object_files = {0};
    Nob_Procs procs = {0};

    if (!nob_mkdir_if_not_exists("./build/raylib"))
        nob_return_defer(false);

    const char *build_path = "./build/raylib/gcc";
    if (!nob_mkdir_if_not_exists(build_path))
        nob_return_defer(false);

    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i)
    {
        const char *input_path = nob_temp_sprintf("./raylib/raylib-5.0/src/%s.c", raylib_modules[i]);
        const char *output_path = nob_temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);

        nob_da_append(&object_files, output_path);

        if (nob_needs_rebuild(output_path, &input_path, 1))
        {
            cmd.count = 0;
            nob_cmd_append(&cmd, "gcc");
            nob_cmd_append(&cmd, "-DPLATFORM_DESKTOP", "-fPIC");
            nob_cmd_append(&cmd, "-I./raylib/raylib-5.0/src/external/glfw/include");
            nob_cmd_append(&cmd, "-c", input_path);
            nob_cmd_append(&cmd, "-o", output_path);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&procs, proc);
        }
    }
    if (!nob_procs_wait(procs))
        nob_return_defer(false);

    cmd.count = 0;
    Nob_String_Builder input_path = {0};
    nob_cmd_append(&cmd, "ar", "-crs", "./build/raylib/gcc/libraylib.a");
    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i)
    {
        input_path.count = 0;
        nob_sb_append_cstr(&input_path, "./build/raylib/gcc/");
        nob_sb_append_cstr(&input_path, raylib_modules[i]);
        nob_sb_append_cstr(&input_path, ".o");
        nob_sb_append_null(&input_path);
        nob_cmd_append(&cmd, nob_temp_strdup(input_path.items));
    }
    if (!nob_cmd_run_sync(cmd))
        nob_return_defer(false);

defer:
    nob_cmd_free(cmd);
    nob_da_free(object_files);
    nob_da_free(procs);
    nob_temp_reset();
    return result;
}

bool build_pov_brain_is_weird(bool plaform_windows)
{
    bool result = true;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc");
    if(plaform_windows){
        nob_cmd_append(&cmd, "-mwindows");
    }
    nob_cmd_append(&cmd, "-Wall", "-Wextra");
    nob_cmd_append(&cmd, "-I./build/");
    nob_cmd_append(&cmd, "-I./raylib/raylib-5.0/src/");
    nob_cmd_append(&cmd, "-o", "./build/pov-brain-is-weird");
    nob_cmd_append(&cmd, "./pov-brain-is-weird.c");
    nob_cmd_append(&cmd, "-L./build/raylib/gcc");
    nob_cmd_append(&cmd, "-l:libraylib.a");
    nob_cmd_append(&cmd, "-lm");
    if(plaform_windows){
        nob_cmd_append(&cmd, "-lwinmm", "-lgdi32");
        nob_cmd_append(&cmd, "-static");
    }
    if (!nob_cmd_run_sync(cmd))
        nob_return_defer(false);

defer:
    nob_cmd_free(cmd);
    return result;
}

void print_usage(void)
{
    fprintf(stderr, "nob [-platform] [platform]\n");
    fprintf(stderr, "\tplatforms supported: windows, linux\n");
}

int main(int argc, char **argv)
{
    bool plaform_windows = true;
    if(argc!=1){
        if(strcmp(argv[1], "-platform")==0){
            if(strcmp(argv[2], "linux")==0){
                plaform_windows = false;
            } else if (strcmp(argv[2], "windows")==0){
                plaform_windows = false;
            } else {
                fprintf(stderr, "Error: unsupported OS\n");
                print_usage();
                exit(69);
            }
        } else {
            print_usage();
            exit(69);
        }
    }
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists("build"))
        return 1;

    if (!build_raylib())
        return 1;
    if (!build_pov_brain_is_weird(plaform_windows))
        return 1;

    return 0;
}
