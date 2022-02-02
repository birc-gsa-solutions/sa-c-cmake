#include <stdio.h>

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "%s genome reads", argv[0]);
        return 1;
    }
    const char *genome_fname = argv[1];
    const char *reads_fname = argv[2];

    printf("Search in %s for reads from %s\n", genome_fname, reads_fname);

    return 0;
}
