/**
 * @file    COREMOD_memory.c
 * @brief   milk memory functions
 *
 * Functions to handle images and streams
 *
 */

/* ================================================================== */
/* ================================================================== */
/*            MODULE INFO                                             */
/* ================================================================== */
/* ================================================================== */

// module default short name
// all CLI calls to this module functions will be <shortname>.<funcname>
// if set to "", then calls use <funcname>
#define MODULE_SHORTNAME_DEFAULT ""

// Module short description
#define MODULE_DESCRIPTION "Memory management for images and variables"

/* =============================================================================================== */
/* =============================================================================================== */
/*                                        HEADER FILES                                             */
/* =============================================================================================== */
/* =============================================================================================== */

#include "CommandLineInterface/CLIcore.h"

#include "CommandLineInterface/timeutils.h"

#include "clearall.h"
#include "create_image.h"
#include "create_variable.h"

#include "delete_image.h"
#include "delete_sharedmem_image.h"

#include "fps_ID.h"
#include "fps_create.h"
#include "fps_list.h"

#include "image_ID.h"
#include "image_complex.h"
#include "image_copy.h"
#include "image_copy_shm.h"
#include "image_keyword.h"
#include "image_keyword_addD.h"
#include "image_keyword_addL.h"
#include "image_keyword_addS.h"
#include "image_keyword_list.h"
#include "image_make2D.h"
#include "image_make3D.h"
#include "image_mk_amph_from_complex.h"
#include "image_mk_complex_from_amph.h"
#include "image_mk_complex_from_reim.h"
#include "image_mk_reim_from_complex.h"
#include "image_set_counters.h"

#include "list_image.h"
#include "list_variable.h"
#include "logshmim.h"

#include "read_shmim.h"
#include "read_shmim_size.h"
#include "read_shmimall.h"

#include "shmimlog.h"
#include "shmimlogcmd.h"

#include "saveall.h"
#include "shmim_purge.h"
#include "shmim_setowner.h"
#include "stream_TCP.h"
#include "stream_UDP.h"
#include "stream_ave.h"
#include "stream_copy.h"
#include "stream_delay.h"
#include "stream_merge.h"
#include "stream_diff.h"
#include "stream_halfimdiff.h"
#include "stream_monitorlimits.h"
#include "stream_paste.h"
#include "stream_pixmapdecode.h"
#include "stream_poke.h"
#include "stream_sem.h"
#include "stream_updateloop.h"

#include "variable_ID.h"

/* ================================================================== */
/* ================================================================== */
/*            INITIALIZE LIBRARY                                      */
/* ================================================================== */
/* ================================================================== */

// Module initialization macro in CLIcore.h
// macro argument defines module name for bindings
//
INIT_MODULE_LIB(COREMOD_memory)

static errno_t init_module_CLI()
{

    data.MEM_MONITOR = 0; // 1 if memory monitor is on

    clearall_addCLIcmd();
    list_image_addCLIcmd();

    //KEYWORDS
    image_keyword_addCLIcmd();
    CLIADDCMD_COREMOD_memory__image_keyword_list();
    CLIADDCMD_COREMOD_memory__image_keyword_addD();
    CLIADDCMD_COREMOD_memory__image_keyword_addL();
    CLIADDCMD_COREMOD_memory__image_keyword_addS();

    // READ SHARED MEM IMAGE AND SIZE
    CLIADDCMD_COREMOD_memory__read_sharedmem_image();
    CLIADDCMD_COREMOD_memory__read_sharedmem_image_size();
    read_shmimall_addCLIcmd();

    // CREATE IMAGE
    create_image_addCLIcmd();
    CLIADDCMD_COREMOD_memory__mk2Dim();
    CLIADDCMD_COREMOD_memory__mk3Dim();

    // COPY IMAGE
    image_copy_addCLIcmd();
    CLIADDCMD_COREMOD_memory__image_copy_shm();

    // DELETE IMAGE
    CLIADDCMD_COREMOD_memory__delete_image();
    CLIADDCMD_COREMOD_memory__delete_sharedmem_image();

    list_variable_addCLIcmd();

    // FPS
    fps_list_addCLIcmd();
    fps_create_addCLIcmd();

    // TYPE CONVERSIONS TO AND FROM COMPLEX
    CLIADDCMD_COREMOD__mk_complex_from_reim();
    CLIADDCMD_COREMOD__mk_complex_from_amph();
    CLIADDCMD_COREMOD__mk_reim_from_complex();
    CLIADDCMD_COREMOD__mk_amph_from_complex();

    // SET IMAGE FLAGS / COUNTERS
    image_set_counters_addCLIcmd();

    // MANAGE SEMAPHORES
    stream_sem_addCLIcmd();

    // STREAMS
    CLIADDCMD_COREMOD_memory__shmim_purge();
    shmim_setowner_addCLIcmd();

    stream_updateloop_addCLIcmd();
    CLIADDCMD_COREMOD_memory__streamdelay();
    saveall_addCLIcmd();
    stream__TCP_addCLIcmd();
    stream__UDP_addCLIcmd();
    stream_pixmapdecode_addCLIcmd();

    CLIADDCMD_COREMOD_memory__stream_copy();
    CLIADDCMD_COREMOD_memory__stream_merge();
    CLIADDCMD_COREMOD_memory__stream_poke();

    stream_diff_addCLIcmd();
    stream_paste_addCLIcmd();
    stream_halfimdiff_addCLIcmd();

    CLIADDCMD_streamaverage();
    stream_monitorlimits_addCLIcmd();

    // DATA LOGGING
    logshmim_addCLIcmd();

    CLIADDCMD_COREMOD_memory__shmimlog();
    CLIADDCMD_COREMOD_memory__shmimlogcmd();

    // add atexit functions here

    return RETURN_SUCCESS;
}
