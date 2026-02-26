#include <ufsi.h>

int main(int argc, char **argv)
{
    int i;

    printf("Physical block size:                    %d\n", LF_BLKSIZ);
    printf("Index block size:                       %d\n", LF_IBSIZE);
    printf("Index blocks per physical block:        %d\n", LF_IBSBLK);
    printf("Data block pointers per index block:    %d\n", LF_IBLEN);
    printf("Bytes of data indexed by 1 index block: %d\n", LF_IDATA);
    printf(" \n");
    printf("Directory entry size:                   %d\n", LF_DBSIZE);
    printf("Directory name length:                  %d\n", LF_NAME_LEN);
    printf("Directory entries per physical block:   %d\n", LF_DBSBLK);

    for(i=0; i < 256; i++) {
        int sector = ib2sect(i);
        int offset = ib2disp(i);

        if (offset==0) printf(" \n");
        printf("Index block %3d is in physical block %2d, offset %3d\n",
               i, sector, offset);
    }

    return 0;
}
