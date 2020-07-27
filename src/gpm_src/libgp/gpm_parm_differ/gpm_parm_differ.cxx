#include "gpm_semantic_parm_tester.h"
#include <cstdio>

int main(int argc, char* argv[])
{
    // We should compare ages also
    if (argc < 2) {
        printf("Need two file names as arguments to diff against\n");
    }
    const int tmp=semantic_diff_parm_hdf5_format(argv[1], argv[2]);
    return tmp;
}
