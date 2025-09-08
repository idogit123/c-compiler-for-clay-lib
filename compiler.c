#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Please provide program to compile.\n");
        return 1;
    }

    char *input_filename = argv[1];

    printf("Compiling program: %s\n", input_filename);

    FILE *input = fopen(input_filename, "r");
    if (!input) {
        printf("Could not open file: %s\n", input_filename);
        return 1;
    }
    FILE *output = fopen(strcat(input_filename, ".c"), "w");
    if (!output) {
        printf("Could not create output file.\n");
        fclose(input);
        return 1;
    }

    fprintf(output, "#include <stdio.h>\nint main() {\n");

    char line[256];
    while (fgets(line, sizeof(line), input)) {
        if (strncmp(line, "print ", 6) == 0) {
            // Extract the string inside quotes
            char *start = strchr(line, '"');
            char *end = strrchr(line, '"');
            if (start && end && start != end) {
                *end = '\0';
                fprintf(output, "    printf(\"%s\\n\");\n", start + 1);
            }
        }
    }

    fprintf(output, "    return 0;\n}\n");

    fclose(input);
    fclose(output);

    printf("Compilation finished. Output file: %s\n", input_filename);

    return 0;
}