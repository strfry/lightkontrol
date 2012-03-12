#include "padkontrol.h"

int main()
{
    PadKontrol pad;
    pk_init(&pad, 5, 4);
    pk_process(&pad);
    pk_deinit(&pad);
}
