extern "C" {
    #include <SDL.h>
    #include <unixstuff.h>
}

int main(int argc, char *argv[])
{
    SetParams(argc, argv);
    uqlxInit();

    QLRun();

    return 0;
}