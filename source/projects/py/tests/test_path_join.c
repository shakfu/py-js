
/*

external_path = "$HOME/Documents/Max 8/Packages/py-js/externals/py.mxo"

char external_contents_path[MAX_PATH_CHARS];
char external_resources_path[MAX_PATH_CHARS];

path_join(external_contents_path, external_path, "Contents");
path_join(external_resources_path, external_contents_path, "Resources");

*/

#include <stdio.h>
#include <string.h>

#define MAX_PATH_CHARS 512


void path_join(char* destination, const char* path1, const char* path2)
{
    //char filename[MAX_FILENAME_CHARS]; 
    //strncpy_zero(filename,str->s_name, MAX_FILENAME_CHARS); 

    if(path1 == NULL && path2 == NULL) {
        strcpy(destination, "");
    }
    else if(path2 == NULL || strlen(path2) == 0) {
        strcpy(destination, path1);
    }
    else if(path1 == NULL || strlen(path1) == 0) {
        strcpy(destination, path2);
    } 
    else {
        char directory_separator[] = "/";
#ifdef WIN32
        directory_separator[0] = '\\';
#endif
        const char *last_char = path1;
        while(*last_char != '\0')
            last_char++;        
        int append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(destination, path1);
        if(append_directory_separator)
            strcat(destination, directory_separator);
        strcat(destination, path2);
    }
}


// int main(int argc, char **argv)
// {
//     const char *d = "/usr/bin";
//     const char* f = "filename.txt";
//     char result[strlen(d) + strlen(f) + 2];
//     path_join(result, d, f);
//     printf("%s\n", result);
//     return 0;
// }

int main(int argc, char **argv)
{
    const char* support_python_path = "support/python/3.9";

	const char* external_path = "Documents/Max 8/Packages/py-js/externals/py.mxo";

	char external_contents_path[MAX_PATH_CHARS];
	char external_resources_path[MAX_PATH_CHARS];

	path_join(external_contents_path, external_path, "Contents");
	path_join(external_resources_path, external_contents_path, "Resources");

    printf("%s\n", external_resources_path);
    return 0;
}
