#include <stdio.h> /* defines FILENAME_MAX */
#ifdef WINDOWS
#include <direct.h>
#define get_current_dir _getcwd
#else
#include <unistd.h>
#define get_current_dir getcwd
#endif

int main(int argc, char* argv[])
{
    char current_path[FILENAME_MAX];

    if (!get_current_dir(current_path, sizeof(current_path))) {
        // return errno;
        return 1;
    }

    current_path[sizeof(current_path) - 1] = '\0'; /* not really required */

    printf("The current working directory is %s", current_path);
    return 0;
}



