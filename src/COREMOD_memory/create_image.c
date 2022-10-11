/**
 * @file    create_image.c
 * @brief   create images and streams
 */

#include "CommandLineInterface/CLIcore.h"
#include "image_ID.h"
#include "list_image.h"
#include "stream_sem.h"

// ==========================================
// forward declarations
// ==========================================

errno_t create_image_ID(const char *name,
                        long        naxis,
                        uint32_t   *size,
                        uint8_t     datatype,
                        int         shared,
                        int         nbkw,
                        int         CBsize,
                        imageID    *outID);

// ==========================================
// command line interface wrapper functions
// ==========================================

static errno_t create_image__cli()
{
    uint32_t *imsize;
    long      naxis = 0;
    long      i;
    uint8_t   datatype;

    if(0 + CLI_checkarg(1, CLIARG_STR_NOT_IMG) +
            CLI_checkarg_noerrmsg(2, CLIARG_LONG) ==
            0)
    {
        naxis  = 0;
        imsize = (uint32_t *) malloc(sizeof(uint32_t) * 5);
        i      = 2;
        while(data.cmdargtoken[i].type == 2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        switch(data.precision)
        {
            case 0:
                create_image_ID(data.cmdargtoken[1].val.string,
                                naxis,
                                imsize,
                                _DATATYPE_FLOAT,
                                data.SHARED_DFT,
                                data.NBKEYWORD_DFT,
                                0,
                                NULL);
                break;
            case 1:
                create_image_ID(data.cmdargtoken[1].val.string,
                                naxis,
                                imsize,
                                _DATATYPE_DOUBLE,
                                data.SHARED_DFT,
                                data.NBKEYWORD_DFT,
                                0,
                                NULL);
                break;
        }
        free(imsize);
        return CLICMD_SUCCESS;
    }
    else if(0 + CLI_checkarg(1, CLIARG_STR_NOT_IMG) +
            CLI_checkarg(2, CLIARG_STR_NOT_IMG) +
            CLI_checkarg(3, CLIARG_LONG) ==
            0) // type option exists
    {
        datatype = 0;

        if(strcmp(data.cmdargtoken[2].val.string, "c") == 0)
        {
            printf("type = CHAR\n");
            datatype = _DATATYPE_UINT8;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "i") == 0)
        {
            printf("type = INT\n");
            datatype = _DATATYPE_INT32;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "f") == 0)
        {
            printf("type = FLOAT\n");
            datatype = _DATATYPE_FLOAT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "d") == 0)
        {
            printf("type = DOUBLE\n");
            datatype = _DATATYPE_DOUBLE;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "cf") == 0)
        {
            printf("type = COMPLEX_FLOAT\n");
            datatype = _DATATYPE_COMPLEX_FLOAT;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "cd") == 0)
        {
            printf("type = COMPLEX_DOUBLE\n");
            datatype = _DATATYPE_COMPLEX_DOUBLE;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "u") == 0)
        {
            printf("type = USHORT\n");
            datatype = _DATATYPE_UINT16;
        }

        if(strcmp(data.cmdargtoken[2].val.string, "l") == 0)
        {
            printf("type = LONG\n");
            datatype = _DATATYPE_INT64;
        }

        if(datatype == 0)
        {
            printf("Data type \"%s\" not recognized\n",
                   data.cmdargtoken[2].val.string);
            printf("must be : \n");
            printf("  c : CHAR\n");
            printf("  i : INT32\n");
            printf("  f : FLOAT\n");
            printf("  d : DOUBLE\n");
            printf("  cf: COMPLEX FLOAT\n");
            printf("  cd: COMPLEX DOUBLE\n");
            printf("  u : USHORT16\n");
            printf("  l : LONG64\n");
            return CLICMD_INVALID_ARG;
        }
        naxis  = 0;
        imsize = (uint32_t *) malloc(sizeof(uint32_t) * 5);
        i      = 3;
        while(data.cmdargtoken[i].type == 2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }

        create_image_ID(data.cmdargtoken[1].val.string,
                        naxis,
                        imsize,
                        datatype,
                        data.SHARED_DFT,
                        data.NBKEYWORD_DFT,
                        0,
                        NULL);

        free(imsize);
        return CLICMD_SUCCESS;
    }
    else
    {
        return CLICMD_INVALID_ARG;
    }
}

static errno_t create_image_shared__cli() // default precision
{
    uint32_t *imsize;
    long      naxis = 0;
    long      i;

    if(0 + CLI_checkarg(1, CLIARG_STR_NOT_IMG) +
            CLI_checkarg(2, CLIARG_LONG) ==
            0)
    {
        naxis  = 0;
        imsize = (uint32_t *) malloc(sizeof(uint32_t) * 5);
        i      = 2;
        while(data.cmdargtoken[i].type == 2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        switch(data.precision)
        {
            case 0:
                create_image_ID(data.cmdargtoken[1].val.string,
                                naxis,
                                imsize,
                                _DATATYPE_FLOAT,
                                1,
                                data.NBKEYWORD_DFT,
                                0,
                                NULL);
                break;
            case 1:
                create_image_ID(data.cmdargtoken[1].val.string,
                                naxis,
                                imsize,
                                _DATATYPE_DOUBLE,
                                1,
                                data.NBKEYWORD_DFT,
                                0,
                                NULL);
                break;
        }
        free(imsize);
        printf("Creating 10 semaphores\n");
        COREMOD_MEMORY_image_set_createsem(data.cmdargtoken[1].val.string,
                                           IMAGE_NB_SEMAPHORE);
        return CLICMD_SUCCESS;
    }
    else
    {
        return CLICMD_INVALID_ARG;
    }
}

static errno_t create_ushort_image_shared__cli() // default precision
{
    uint32_t *imsize;
    long      naxis = 0;
    long      i;

    if(0 + CLI_checkarg(1, CLIARG_STR_NOT_IMG) +
            CLI_checkarg(2, CLIARG_LONG) ==
            0)
    {
        naxis  = 0;
        imsize = (uint32_t *) malloc(sizeof(uint32_t) * 5);
        i      = 2;
        while(data.cmdargtoken[i].type == 2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        create_image_ID(data.cmdargtoken[1].val.string,
                        naxis,
                        imsize,
                        _DATATYPE_UINT16,
                        1,
                        data.NBKEYWORD_DFT,
                        0,
                        NULL);

        free(imsize);
        return CLICMD_SUCCESS;
    }
    else
    {
        return CLICMD_INVALID_ARG;
    }
}

static errno_t create_sshort_image_shared__cli() // default precision
{
    uint32_t *imsize;
    long      naxis = 0;
    long      i;

    if(0 + CLI_checkarg(1, CLIARG_STR_NOT_IMG) +
            CLI_checkarg(2, CLIARG_LONG) ==
            0)
    {
        naxis  = 0;
        imsize = (uint32_t *) malloc(sizeof(uint32_t) * 5);
        i      = 2;
        while(data.cmdargtoken[i].type == 2)
        {
            imsize[naxis] = data.cmdargtoken[i].val.numl;
            naxis++;
            i++;
        }
        create_image_ID(data.cmdargtoken[1].val.string,
                        naxis,
                        imsize,
                        _DATATYPE_INT16,
                        1,
                        data.NBKEYWORD_DFT,
                        0,
                        NULL);

        free(imsize);
        return CLICMD_SUCCESS;
    }
    else
    {
        return CLICMD_INVALID_ARG;
    }
}

static errno_t create_3Dimage_float()
{
    DEBUG_TRACE_FSTART();

    uint32_t *imsize;

    // CHECK ARGS
    //  printf("CREATING 3D IMAGE\n");
    imsize = (uint32_t *) malloc(sizeof(uint32_t) * 3);

    imsize[0] = data.cmdargtoken[2].val.numl;
    imsize[1] = data.cmdargtoken[3].val.numl;
    imsize[2] = data.cmdargtoken[4].val.numl;

    create_image_ID(data.cmdargtoken[1].val.string,
                    3,
                    imsize,
                    _DATATYPE_FLOAT,
                    data.SHARED_DFT,
                    data.NBKEYWORD_DFT,
                    0,
                    NULL);

    free(imsize);

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

// ==========================================
// Register CLI commands
// ==========================================

errno_t create_image_addCLIcmd()
{

    RegisterCLIcommand("creaim",
                       __FILE__,
                       create_image__cli,
                       "create image, default precision",
                       "<name> <xsize> <ysize> <opt: zsize>",
                       "creaim imname 512 512",
                       "long create_image_ID(const char *name, long naxis, "
                       "uint32_t *size, uint8_t datatype, 0, 10, NULL)");

    RegisterCLIcommand("creaimshm",
                       __FILE__,
                       create_image_shared__cli,
                       "create image in shared mem, default precision",
                       "<name> <xsize> <ysize> <opt: zsize>",
                       "creaimshm imname 512 512",
                       "long create_image_ID(const char *name, long naxis, "
                       "uint32_t *size, uint8_t datatype, 0, 10, NULL)");

    RegisterCLIcommand("creaushortimshm",
                       __FILE__,
                       create_ushort_image_shared__cli,
                       "create unsigned short image in shared mem",
                       "<name> <xsize> <ysize> <opt: zsize>",
                       "creaushortimshm imname 512 512",
                       "long create_image_ID(const char *name, long naxis, "
                       "long *size, _DATATYPE_UINT16, 0, 10, NULL)");

    RegisterCLIcommand("creasshortimshm",
                       __FILE__,
                       create_sshort_image_shared__cli,
                       "create signed short image in shared mem",
                       "<name> <xsize> <ysize> <opt: zsize>",
                       "creasshortimshm imname 512 512",
                       "long create_image_ID(const char *name, long naxis, "
                       "long *size, _DATATYPE_INT16, 0, 10, NULL)");

    RegisterCLIcommand("crea3dim",
                       __FILE__,
                       create_3Dimage_float,
                       "creates 3D image, single precision",
                       "<name> <xsize> <ysize> <zsize>",
                       "crea3dim imname 512 512 100",
                       "long create_image_ID(const char *name, long naxis, "
                       "long *size, _DATATYPE_FLOAT, 0, 10, NULL)");

    return RETURN_SUCCESS;
}

/* creates an image ID */
/* all images should be created by this function */
errno_t create_image_ID(const char *name,
                        long        naxis,
                        uint32_t   *size,
                        uint8_t     datatype,
                        int         shared,
                        int         NBkw,
                        int         CBsize,
                        imageID    *outID)
{
    DEBUG_TRACE_FSTART();
    DEBUG_TRACEPOINT("FARG %s %ld %d %d %d %d",
                     name,
                     naxis,
                     (int) datatype,
                     shared,
                     NBkw,
                     CBsize);

    imageID ID;

    ID = -1;
    if(image_ID(name) == -1)
    {
        ID = next_avail_image_ID();

        ImageStreamIO_createIm(&data.image[ID],
                               name,
                               naxis,
                               size,
                               datatype,
                               shared,
                               NBkw,
                               CBsize);
    }
    else
    {
        // Cannot create image : name already in use
        ID = image_ID(name);

        if(data.image[ID].md[0].datatype != datatype)
        {
            FUNC_RETURN_FAILURE("Pre-existing image \"%s\" has wrong type",
                                name);
        }
        if(data.image[ID].md[0].naxis != naxis)
        {
            FUNC_RETURN_FAILURE("Pre-existing image \"%s\" has wrong naxis",
                                name);
        }

        for(int i = 0; i < naxis; i++)
            if(data.image[ID].md[0].size[i] != size[i])
            {
                FUNC_RETURN_FAILURE(
                    "Pre-existing image \"%s\" has wrong size: axis %d "
                    ":  %ld  %ld",
                    name,
                    i,
                    (long) data.image[ID].md[0].size[i],
                    (long) size[i]);
            }
    }

    if(data.MEM_MONITOR == 1)
    {
        list_image_ID_ncurses();
    }

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

errno_t create_1Dimage_ID(const char *ID_name, uint32_t xsize, imageID *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID    = -1;
    long     naxis = 1;
    uint32_t naxes[1];

    naxes[0] = xsize;

    if(data.precision == 0)
    {
        // single precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_FLOAT,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }
    if(data.precision == 1)
    {
        // double precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_DOUBLE,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

errno_t create_1DCimage_ID(const char *ID_name, uint32_t xsize, imageID *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID    = -1;
    long     naxis = 1;
    uint32_t naxes[1];

    naxes[0] = xsize;

    if(data.precision == 0)
    {
        // single precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_COMPLEX_FLOAT,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }
    if(data.precision == 1)
    {
        // double precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_COMPLEX_DOUBLE,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

errno_t create_2Dimage_ID(const char *ID_name,
                          uint32_t    xsize,
                          uint32_t    ysize,
                          imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID    = -1;
    long     naxis = 2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    if(data.precision == 0)
    {
        // single precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_FLOAT,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }
    else if(data.precision == 1)
    {
        // double precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_DOUBLE,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }
    else
    {
        // single precision
        printf(
            "Default precision (%d) invalid value: assuming single "
            "precision\n",
            data.precision);
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_FLOAT,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

errno_t create_2Dimage_ID_double(const char *ID_name,
                                 uint32_t    xsize,
                                 uint32_t    ysize,
                                 imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID    = -1;
    long     naxis = 2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                      naxis,
                                      naxes,
                                      _DATATYPE_DOUBLE,
                                      data.SHARED_DFT,
                                      data.NBKEYWORD_DFT,
                                      0,
                                      &ID));

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

/* 2D complex image */
errno_t create_2DCimage_ID(const char *ID_name,
                           uint32_t    xsize,
                           uint32_t    ysize,
                           imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID    = -1;
    long     naxis = 2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    if(data.precision == 0)
    {
        // single precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_COMPLEX_FLOAT,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }
    if(data.precision == 1)
    {
        // double precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_COMPLEX_DOUBLE,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

/* 2D complex image */
errno_t create_2DCimage_ID_double(const char *ID_name,
                                  uint32_t    xsize,
                                  uint32_t    ysize,
                                  imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID    = -1;
    long     naxis = 2;
    uint32_t naxes[2];

    naxes[0] = xsize;
    naxes[1] = ysize;

    FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                      naxis,
                                      naxes,
                                      _DATATYPE_COMPLEX_DOUBLE,
                                      data.SHARED_DFT,
                                      data.NBKEYWORD_DFT,
                                      0,
                                      &ID));

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

/* 3D image, single precision */
errno_t create_3Dimage_ID_float(const char *ID_name,
                                uint32_t    xsize,
                                uint32_t    ysize,
                                uint32_t    zsize,
                                imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID    = -1;
    long     naxis = 3;
    uint32_t naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    //  printf("CREATING 3D IMAGE: %s %ld %ld %ld\n", ID_name, xsize, ysize, zsize);
    //  fflush(stdout);

    FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                      naxis,
                                      naxes,
                                      _DATATYPE_FLOAT,
                                      data.SHARED_DFT,
                                      data.NBKEYWORD_DFT,
                                      0,
                                      &ID));

    //  printf("IMAGE CREATED WITH ID = %ld\n",ID);
    //  fflush(stdout);

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

/* 3D image, double precision */
errno_t create_3Dimage_ID_double(const char *ID_name,
                                 uint32_t    xsize,
                                 uint32_t    ysize,
                                 uint32_t    zsize,
                                 imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID  ID;
    long     naxis = 3;
    uint32_t naxes[3];

    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                      naxis,
                                      naxes,
                                      _DATATYPE_DOUBLE,
                                      data.SHARED_DFT,
                                      data.NBKEYWORD_DFT,
                                      0,
                                      &ID));

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}




/* 3D image, default precision */
errno_t create_3Dimage_ID(const char *ID_name,
                          uint32_t    xsize,
                          uint32_t    ysize,
                          uint32_t    zsize,
                          imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID   ID    = -1;
    long      naxis = 3;
    uint32_t *naxes;

    naxes    = (uint32_t *) malloc(sizeof(uint32_t) * 3);
    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    if(data.precision == 0)
    {
        // single precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_FLOAT,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    if(data.precision == 1)
    {
        // double precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_DOUBLE,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    free(naxes);

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

/* 3D complex image */
errno_t create_3DCimage_ID(const char *ID_name,
                           uint32_t    xsize,
                           uint32_t    ysize,
                           uint32_t    zsize,
                           imageID    *outID)
{
    DEBUG_TRACE_FSTART();

    imageID   ID    = -1;
    long      naxis = 3;
    uint32_t *naxes;

    naxes    = (uint32_t *) malloc(sizeof(uint32_t) * 3);
    naxes[0] = xsize;
    naxes[1] = ysize;
    naxes[2] = zsize;

    if(data.precision == 0)
    {
        // single precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_COMPLEX_FLOAT,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    if(data.precision == 1)
    {
        // double precision
        FUNC_CHECK_RETURN(create_image_ID(ID_name,
                                          naxis,
                                          naxes,
                                          _DATATYPE_COMPLEX_DOUBLE,
                                          data.SHARED_DFT,
                                          data.NBKEYWORD_DFT,
                                          0,
                                          &ID));
    }

    free(naxes);

    if(outID != NULL)
    {
        *outID = ID;
    }

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}
