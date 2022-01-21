/**
 * @file    fps_list.c
 * @brief   list function parameter structure
 */

#include <dirent.h>

#include "CommandLineInterface/CLIcore.h"

// ==========================================
// Forward declaration(s)
// ==========================================

errno_t fps_list();

// ==========================================
// Command line interface wrapper function(s)
// ==========================================

// ==========================================
// Register CLI command(s)
// ==========================================

errno_t fps_list_addCLIcmd()
{

    RegisterCLIcommand("fpslist",
                       __FILE__,
                       fps_list,
                       "list function parameter structures (FPSs)",
                       "no argument",
                       "fpslist",
                       "errno_t fps_list()");

    return RETURN_SUCCESS;
}

errno_t fps_list()
{
    long fpsID;
    long fpscnt = 0;

    int NBchar_fpsID   = 5;
    int NBchar_fpsname = 12;
    int NBchar_NBparam = 4;

    for (fpsID = 0; fpsID < data.NB_MAX_FPS; fpsID++)
    {
        if (data.fpsarray[fpsID].SMfd > -1)
        {

            if (fpscnt == 0)
            {
                printf("FPSs currently connected :\n");
            }
            // connected
            printf("%*ld  %*s  %*ld/%*ld entries\n",
                   NBchar_fpsID,
                   fpsID,
                   NBchar_fpsname,
                   data.fpsarray[fpsID].md[0].name,
                   NBchar_NBparam,
                   data.fpsarray[fpsID].NBparamActive,
                   NBchar_NBparam,
                   data.fpsarray[fpsID].NBparam);

            fpscnt++;
        }
    }
    if (fpscnt == 0)
    {
        printf("No FPS currently connected\n");
    }

    //printf("\n %ld FPS(s) currently loaded\n\n", fpscnt);
    //printf("\n");

    printf("FPSs in system shared memory (%s):\n", data.shmdir);

    struct dirent *de;
    DIR           *dr = opendir(data.shmdir);
    if (dr == NULL)
    {
        printf("Could not open current directory");
        return RETURN_FAILURE;
    }

    fpscnt = 0;
    while ((de = readdir(dr)) != NULL)
    {
        if (strstr(de->d_name, ".fps.shm") != NULL)
        {
            char fpsname[100];
            int  slen  = strlen(de->d_name);
            int  slen1 = slen - strlen(".fps.shm");

            strncpy(fpsname, de->d_name, slen1);
            fpsname[slen1] = '\0';
            printf("%*ld  %*s\n",
                   NBchar_fpsID,
                   fpscnt,
                   NBchar_fpsname,
                   fpsname);
            fpscnt++;
        }
    }
    closedir(dr);

    return RETURN_SUCCESS;
}
