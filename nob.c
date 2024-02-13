#define NOB_IMPLEMENTATION
#include "./nob.h"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists("build")) return 1;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, "./pov-brain-is-weird.c");
    nob_cmd_append(&cmd, "-Wall", "-Wextra");
    nob_cmd_append(&cmd, "-o", "./build/pov-brain-is-weird");

    bool build_succeeded = nob_cmd_run_sync(cmd);
    nob_cmd_free(cmd);

    if (!build_succeeded) return 1;

    return 0;
}