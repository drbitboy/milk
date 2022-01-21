/**
 * @file    fps_CTRLscreen.c
 * @brief   FPS control TUI
 */

#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <time.h>

#include "CommandLineInterface/CLIcore.h"

#include "COREMOD_tools/COREMOD_tools.h"

#include "CommandLineInterface/timeutils.h"

#include "fpsCTRL_TUI_process_user_key.h"
#include "fps_GetTypeString.h"
#include "fps_disconnect.h"
#include "fps_outlog.h"
#include "fps_process_fpsCMDarray.h"
#include "fps_read_fpsCMD_fifo.h"
#include "fps_scan.h"

#include "TUItools.h"

static short unsigned int wrow, wcol;

inline static void print_help_entry(char *key, char *descr)
{
    screenprint_setbold();
    TUI_printfw("    %4s", key);
    screenprint_unsetbold();
    TUI_printfw("   %s", descr);
    TUI_newline();
}

inline static void
fpsCTRLscreen_print_DisplayMode_status(int fpsCTRL_DisplayMode, int NBfps)
{

    int  stringmaxlen = 500;
    char monstring[stringmaxlen];

    screenprint_setbold();

    if (snprintf(monstring,
                 stringmaxlen,
                 "[%d x %d] [PID %d] FUNCTION PARAMETER MONITOR: PRESS (x) TO "
                 "STOP, (h) FOR HELP [%d FPS]",
                 wrow,
                 wcol,
                 (int) getpid(),
                 NBfps) < 0)
    {
        PRINT_ERROR("snprintf error");
    }
    TUI_print_header(monstring, '-');
    screenprint_unsetbold();
    TUI_newline();

    if (fpsCTRL_DisplayMode == 1)
    {
        screenprint_setreverse();
        TUI_printfw("[h] Help");
        screenprint_unsetreverse();
    }
    else
    {
        TUI_printfw("[h] Help");
    }
    TUI_printfw("   ");

    if (fpsCTRL_DisplayMode == 2)
    {
        screenprint_setreverse();
        TUI_printfw("[F2] FPS CTRL");
        screenprint_unsetreverse();
    }
    else
    {
        TUI_printfw("[F2] FPS CTRL");
    }
    TUI_printfw("   ");

    if (fpsCTRL_DisplayMode == 3)
    {
        screenprint_setreverse();
        TUI_printfw("[F3] Sequencer");
        screenprint_unsetreverse();
    }
    else
    {
        TUI_printfw("[F3] Sequencer");
    }
    TUI_newline();
}

inline static void fpsCTRLscreen_print_help()
{
    // int attrval = A_BOLD;

    TUI_newline();
    print_help_entry("x", "Exit");

    TUI_newline();
    TUI_printfw("============ SCREENS");
    TUI_newline();
    print_help_entry("h/F2/F3", "Help/Control/Sequencer screen");

    TUI_newline();
    TUI_printfw("============ OTHER");
    TUI_newline();
    print_help_entry("s", "rescan");
    print_help_entry("T/t", "initialize (T)mux session / kill (t)mux session");
    print_help_entry("E/e", "(E)rase FPS and tmux sessions / (e)rase FPS only");
    print_help_entry("C/c/u", "start/stop/update CONF process");
    print_help_entry("R/r", "start/stop RUN process");
    print_help_entry("l", "list all entries");
    print_help_entry(">", "export fpsdatadir values fpsconfdir");
    print_help_entry("<", "import/load values from fpsconfdir");
    print_help_entry("P", "(P)rocess input file \"confscript\"");
    TUI_printfw("        format: setval <paramfulname> <value>");
    TUI_newline();
}

inline static void fpsCTRLscreen_print_nodeinfo(FUNCTION_PARAMETER_STRUCT *fps,
                                                KEYWORD_TREE_NODE *keywnode,
                                                int                nodeSelected,
                                                int fpsindexSelected,
                                                int pindexSelected)
{

    DEBUG_TRACEPOINT("Selected node %d in fps %d",
                     nodeSelected,
                     keywnode[nodeSelected].fpsindex);

    TUI_printfw("======== FPS info ( # %5d)", keywnode[nodeSelected].fpsindex);
    TUI_newline();

    char teststring[200];
    sprintf(teststring,
            "%s",
            fps[keywnode[nodeSelected].fpsindex].md->sourcefname);
    DEBUG_TRACEPOINT("TEST STRING : %s", teststring);

    DEBUG_TRACEPOINT("TEST LINE : %d",
                     fps[keywnode[nodeSelected].fpsindex].md->sourceline);

    TUI_printfw("    FPS call              : %s -> %s [",
                fps[keywnode[nodeSelected].fpsindex].md->callprogname,
                fps[keywnode[nodeSelected].fpsindex].md->callfuncname);

    for (int i = 0; i < fps[keywnode[nodeSelected].fpsindex].md->NBnameindex;
         i++)
    {
        TUI_printfw(" %s",
                    fps[keywnode[nodeSelected].fpsindex].md->nameindexW[i]);
    }
    TUI_printfw(" ]");
    TUI_newline();

    TUI_printfw("    FPS source            : %s %d",
                fps[keywnode[nodeSelected].fpsindex].md->sourcefname,
                fps[keywnode[nodeSelected].fpsindex].md->sourceline);
    TUI_newline();

    TUI_printfw("   %d libs : ",
                fps[keywnode[nodeSelected].fpsindex].md->NBmodule);
    for (int m = 0; m < fps[keywnode[nodeSelected].fpsindex].md->NBmodule; m++)
    {
        TUI_printfw(" [%s]",
                    fps[keywnode[nodeSelected].fpsindex].md->modulename[m]);
    }
    TUI_newline();

    DEBUG_TRACEPOINT(" ");
    TUI_printfw("    FPS work     directory    : %s",
                fps[keywnode[nodeSelected].fpsindex].md->workdir);
    TUI_newline();

    TUI_printfw(
        "    ( FPS output data directory : %s )  ( FPS input conf directory : "
        "%s)",
        fps[keywnode[nodeSelected].fpsindex].md->datadir,
        fps[keywnode[nodeSelected].fpsindex].md->confdir);
    TUI_newline();

    DEBUG_TRACEPOINT(" ");
    TUI_printfw("    FPS tmux sessions     :  ");

    EXECUTE_SYSTEM_COMMAND("tmux has-session -t %s:ctrl 2> /dev/null",
                           fps[keywnode[nodeSelected].fpsindex].md->name);
    if (data.retvalue == 0)
    {
        fps[keywnode[nodeSelected].fpsindex].md->status |=
            FUNCTION_PARAMETER_STRUCT_STATUS_TMUXCTRL;
    }
    else
    {
        fps[keywnode[nodeSelected].fpsindex].md->status &=
            ~FUNCTION_PARAMETER_STRUCT_STATUS_TMUXCTRL;
    }

    EXECUTE_SYSTEM_COMMAND("tmux has-session -t %s:conf 2> /dev/null",
                           fps[keywnode[nodeSelected].fpsindex].md->name);
    if (data.retvalue == 0)
    {
        fps[keywnode[nodeSelected].fpsindex].md->status |=
            FUNCTION_PARAMETER_STRUCT_STATUS_TMUXCONF;
    }
    else
    {
        fps[keywnode[nodeSelected].fpsindex].md->status &=
            ~FUNCTION_PARAMETER_STRUCT_STATUS_TMUXCONF;
    }

    EXECUTE_SYSTEM_COMMAND("tmux has-session -t %s:run 2> /dev/null",
                           fps[keywnode[nodeSelected].fpsindex].md->name);
    if (data.retvalue == 0)
    {
        fps[keywnode[nodeSelected].fpsindex].md->status |=
            FUNCTION_PARAMETER_STRUCT_STATUS_TMUXRUN;
    }
    else
    {
        fps[keywnode[nodeSelected].fpsindex].md->status &=
            ~FUNCTION_PARAMETER_STRUCT_STATUS_TMUXRUN;
    }

    if (fps[keywnode[nodeSelected].fpsindex].md->status &
        FUNCTION_PARAMETER_STRUCT_STATUS_TMUXCTRL)
    {
        screenprint_setcolor(COLOR_OK);
        TUI_printfw("%s:ctrl", fps[keywnode[nodeSelected].fpsindex].md->name);
        screenprint_unsetcolor(COLOR_OK);
    }
    else
    {
        screenprint_setcolor(COLOR_ERROR);
        TUI_printfw("%s:ctrl", fps[keywnode[nodeSelected].fpsindex].md->name);
        screenprint_unsetcolor(COLOR_ERROR);
    }
    TUI_printfw(" ");
    if (fps[keywnode[nodeSelected].fpsindex].md->status &
        FUNCTION_PARAMETER_STRUCT_STATUS_TMUXCONF)
    {
        screenprint_setcolor(COLOR_OK);
        TUI_printfw("%s:conf", fps[keywnode[nodeSelected].fpsindex].md->name);
        screenprint_unsetcolor(COLOR_OK);
    }
    else
    {
        screenprint_setcolor(COLOR_ERROR);
        TUI_printfw("%s:conf", fps[keywnode[nodeSelected].fpsindex].md->name);
        screenprint_unsetcolor(COLOR_ERROR);
    }
    TUI_printfw(" ");
    if (fps[keywnode[nodeSelected].fpsindex].md->status &
        FUNCTION_PARAMETER_STRUCT_STATUS_TMUXRUN)
    {
        screenprint_setcolor(COLOR_OK);
        TUI_printfw("%s:run", fps[keywnode[nodeSelected].fpsindex].md->name);
        screenprint_unsetcolor(COLOR_OK);
    }
    else
    {
        screenprint_setcolor(COLOR_ERROR);
        TUI_printfw("%s:run", fps[keywnode[nodeSelected].fpsindex].md->name);
        screenprint_unsetcolor(COLOR_ERROR);
    }
    TUI_newline();

    DEBUG_TRACEPOINT(" ");
    TUI_printfw("======== NODE info ( # %5ld)", nodeSelected);
    TUI_newline();
    TUI_printfw("%-30s ", keywnode[nodeSelected].keywordfull);

    if (keywnode[nodeSelected].leaf > 0) // If this is not a directory
    {
        char typestring[100];
        functionparameter_GetTypeString(
            fps[fpsindexSelected].parray[pindexSelected].type,
            typestring);
        TUI_printfw("type %s", typestring);
        TUI_newline();

        // print binary flag
        TUI_printfw("FLAG : ");
        uint64_t mask = (uint64_t) 1 << (sizeof(uint64_t) * CHAR_BIT - 1);
        while (mask)
        {
            int digit =
                fps[fpsindexSelected].parray[pindexSelected].fpflag & mask ? 1
                                                                           : 0;
            if (digit == 1)
            {
                screenprint_setcolor(2);
                TUI_printfw("%d", digit);
                screenprint_unsetcolor(2);
            }
            else
            {
                TUI_printfw("%d", digit);
            }
            mask >>= 1;
        }
    }
    else
    {
        TUI_printfw("-DIRECTORY-");
        TUI_newline();
    }
    TUI_newline();
    TUI_newline();
}

inline static void
fpsCTRLscreen_level0node_summary(FUNCTION_PARAMETER_STRUCT *fps, int fpsindex)
{
    pid_t pid;

    pid = fps[fpsindex].md->confpid;
    if ((getpgid(pid) >= 0) && (pid > 0))
    {
        screenprint_setcolor(2);
        TUI_printfw("%07d ", (int) pid);
        screenprint_unsetcolor(2);
    }
    else // PID not active
    {
        if (fps[fpsindex].md->status & FUNCTION_PARAMETER_STRUCT_STATUS_CMDCONF)
        {
            // not clean exit
            screenprint_setcolor(4);
            TUI_printfw("%07d ", (int) pid);
            screenprint_unsetcolor(4);
        }
        else
        {
            // All OK
            TUI_printfw("%07d ", (int) pid);
        }
    }

    if (fps[fpsindex].md->conferrcnt > 99)
    {
        screenprint_setcolor(4);
        TUI_printfw("[XX]");
        screenprint_unsetcolor(4);
    }
    if (fps[fpsindex].md->conferrcnt > 0)
    {
        screenprint_setcolor(4);
        TUI_printfw("[%02d]", fps[fpsindex].md->conferrcnt);
        screenprint_unsetcolor(4);
    }
    if (fps[fpsindex].md->conferrcnt == 0)
    {
        screenprint_setcolor(2);
        TUI_printfw("[%02d]", fps[fpsindex].md->conferrcnt);
        screenprint_unsetcolor(2);
    }

    pid = fps[fpsindex].md->runpid;
    if ((getpgid(pid) >= 0) && (pid > 0))
    {
        screenprint_setcolor(2);
        TUI_printfw("%07d ", (int) pid);
        screenprint_unsetcolor(2);
    }
    else
    {
        if (fps[fpsindex].md->status & FUNCTION_PARAMETER_STRUCT_STATUS_CMDRUN)
        {
            // not clean exit
            screenprint_setcolor(4);
            TUI_printfw("%07d ", (int) pid);
            screenprint_unsetcolor(4);
        }
        else
        {
            // All OK
            TUI_printfw("%07d ", (int) pid);
        }
    }
}

/** @brief runs fpsCTRL GUI
 *
 * ## Purpose
 *
 * Automatically build simple ASCII GUI from function parameter structure (fps) name mask
 *
 *
 *
 */
errno_t functionparameter_CTRLscreen(uint32_t mode,
                                     char    *fpsnamemask,
                                     char    *fpsCTRLfifoname)
{
    int fpsindex;

    FPSCTRL_PROCESS_VARS fpsCTRLvar;

    // function parameters
    long NBpindex = 0;
    long pindex;

    // keyword tree
    KEYWORD_TREE_NODE *keywnode;

    int level;

    int       loopOK  = 1;
    long long loopcnt = 0;

    long NBtaskLaunchedcnt = 0;

    int nodechain[MAXNBLEVELS];

    // What to run ?
    // disable for testing
    int run_display = 1;
    loopOK          = 1;

    struct timespec tnow;
    clock_gettime(CLOCK_REALTIME, &tnow);
    data.FPS_TIMESTAMP = tnow.tv_sec;
    strcpy(data.FPS_PROCESS_TYPE, "ctrl");

    functionparameter_outlog("FPSCTRL", "START\n");

    DEBUG_TRACEPOINT("function start");

    // initialize fpsCTRLvar
    fpsCTRLvar.exitloop              = 0;
    fpsCTRLvar.mode                  = mode;
    fpsCTRLvar.nodeSelected          = 1;
    fpsCTRLvar.run_display           = run_display;
    fpsCTRLvar.fpsindexSelected      = 0;
    fpsCTRLvar.pindexSelected        = 0;
    fpsCTRLvar.directorynodeSelected = 0;
    fpsCTRLvar.currentlevel          = 0;
    fpsCTRLvar.direction             = 1;
    strcpy(fpsCTRLvar.fpsnamemask, fpsnamemask);
    strcpy(fpsCTRLvar.fpsCTRLfifoname, fpsCTRLfifoname);

    fpsCTRLvar.fpsCTRL_DisplayMode = 2;
    // 1: [h]  help
    // 2: [F2] list of conf and run
    // 3: [F3] fpscmdarray

    // allocate memory

    // Array holding fps structures
    //
    //fps = (FUNCTION_PARAMETER_STRUCT *) malloc(sizeof(FUNCTION_PARAMETER_STRUCT) *
    //        NB_FPS_MAX);

    // Initialize file descriptors to -1
    //
    //    for(fpsindex = 0; fpsindex < NB_FPS_MAX; fpsindex++)
    //    {
    //        data.fps[fpsindex].SMfd = -1;
    //    }

    // All parameters held in this array
    //
    keywnode = (KEYWORD_TREE_NODE *) malloc(sizeof(KEYWORD_TREE_NODE) *
                                            NB_KEYWNODE_MAX);
    if (keywnode == NULL)
    {
        PRINT_ERROR("malloc error: can't allocate keywnode");
        abort();
    }
    for (int kn = 0; kn < NB_KEYWNODE_MAX; kn++)
    {
        strcpy(keywnode[kn].keywordfull, "");
        for (int ch = 0; ch < MAX_NB_CHILD; ch++)
        {
            keywnode[kn].child[ch] = 0;
        }
    }

    // initialize nodechain
    for (int l = 0; l < MAXNBLEVELS; l++)
    {
        nodechain[l] = 0;
    }

    // Set up instruction buffer to sequence commands
    //
    FPSCTRL_TASK_ENTRY *fpsctrltasklist;
    fpsctrltasklist = (FPSCTRL_TASK_ENTRY *) malloc(sizeof(FPSCTRL_TASK_ENTRY) *
                                                    NB_FPSCTRL_TASK_MAX);
    if (fpsctrltasklist == NULL)
    {
        PRINT_ERROR("malloc error");
        abort();
    }
    for (int cmdindex = 0; cmdindex < NB_FPSCTRL_TASK_MAX; cmdindex++)
    {
        fpsctrltasklist[cmdindex].status = 0;
        fpsctrltasklist[cmdindex].queue  = 0;
    }

    // Set up task queue list
    //
    FPSCTRL_TASK_QUEUE *fpsctrlqueuelist;
    fpsctrlqueuelist = (FPSCTRL_TASK_QUEUE *) malloc(
        sizeof(FPSCTRL_TASK_QUEUE) * NB_FPSCTRL_TASKQUEUE_MAX);
    if (fpsctrlqueuelist == NULL)
    {
        PRINT_ERROR("malloc error");
        abort();
    }
    for (int queueindex = 0; queueindex < NB_FPSCTRL_TASKQUEUE_MAX;
         queueindex++)
    {
        fpsctrlqueuelist[queueindex].priority = 1; // 0 = not active
    }

    // catch signals (CTRL-C etc)
    //
    set_signal_catch();

    // fifo
    fpsCTRLvar.fpsCTRLfifofd =
        open(fpsCTRLvar.fpsCTRLfifoname, O_RDWR | O_NONBLOCK);
    long fifocmdcnt = 0;

    for (level = 0; level < MAXNBLEVELS; level++)
    {
        fpsCTRLvar.GUIlineSelected[level] = 0;
    }

    functionparameter_scan_fps(fpsCTRLvar.mode,
                               fpsCTRLvar.fpsnamemask,
                               data.fpsarray,
                               keywnode,
                               &fpsCTRLvar.NBkwn,
                               &fpsCTRLvar.NBfps,
                               &NBpindex,
                               0 // quiet
    );

    printf("%d function parameter structure(s) imported, %ld parameters\n",
           fpsCTRLvar.NBfps,
           NBpindex);
    fflush(stdout);

    DEBUG_TRACEPOINT(" ");

    fpsCTRLvar.nodeSelected = 1;
    fpsindex                = 0;

    // default: use ncurses
    TUI_set_screenprintmode(SCREENPRINT_NCURSES);

    if (getenv("MILK_TUIPRINT_STDIO"))
    {
        // use stdio instead of ncurses
        TUI_set_screenprintmode(SCREENPRINT_STDIO);
    }

    if (getenv("MILK_TUIPRINT_NONE"))
    {
        TUI_set_screenprintmode(SCREENPRINT_NONE);
    }

    // INITIALIZE terminal

    if (run_display == 1)
    {
        TUI_init_terminal(&wrow, &wcol);
        DEBUG_TRACEPOINT("returned from TUI init %d %d", wrow, wcol);
    }

    fpsCTRLvar.NBindex = 0;
    char shmdname[200];
    function_parameter_struct_shmdirname(shmdname);

    if (run_display == 0)
    {
        loopOK = 0;
    }

    int getchardt_us_ref = 100000; // how long between getchar probes
    // refresh every 1 sec without input
    int refreshtimeoutus_ref = 1000000;

    int getchardt_us     = getchardt_us_ref;
    int refreshtimeoutus = refreshtimeoutus_ref;

    if (TUI_get_screenprintmode() == SCREENPRINT_NCURSES) // ncurses mode
    {
        refreshtimeoutus_ref = 100000; // 10 Hz
    }

    int refresh_screen = 1; // 1 if screen should be refreshed

    while (loopOK == 1)
    {
        int NBtaskLaunched = 0;

        long icnt = 0;
        int  ch   = -1;

        int timeoutuscnt = 0;

        while (refresh_screen == 0) // wait for input
        {
            // put input commands from fifo into the task queue
            int fcnt =
                functionparameter_read_fpsCMD_fifo(fpsCTRLvar.fpsCTRLfifofd,
                                                   fpsctrltasklist,
                                                   fpsctrlqueuelist);

            DEBUG_TRACEPOINT(" ");

            // execute next command in the queue
            int taskflag =
                function_parameter_process_fpsCMDarray(fpsctrltasklist,
                                                       fpsctrlqueuelist,
                                                       keywnode,
                                                       &fpsCTRLvar,
                                                       data.fpsarray);

            if (taskflag > 0) // task has been performed
            {
                getchardt_us = 1000; // check often
            }
            else
            {
                getchardt_us =
                    (int) (1.01 * getchardt_us); // gradually slow down
                if (getchardt_us > getchardt_us_ref)
                {
                    getchardt_us = getchardt_us_ref;
                }
            }
            NBtaskLaunched += taskflag;

            NBtaskLaunchedcnt += NBtaskLaunched;

            fifocmdcnt += fcnt;

            usleep(getchardt_us);

            // ==================
            // = GET USER INPUT =
            // ==================
            ch = get_singlechar_nonblock();

            if (ch == -1)
            {
                refresh_screen = 0;
            }
            else
            {
                refresh_screen = 2;
            }

            //tcnt ++;
            timeoutuscnt += getchardt_us;
            if (timeoutuscnt > refreshtimeoutus)
            {
                refresh_screen = 1;
            }

            DEBUG_TRACEPOINT(" ");
        }

        if (refresh_screen > 0)
        {
            refresh_screen--; // will wait next time we enter the loop
        }

        TUI_clearscreen(&wrow, &wcol);

        DEBUG_TRACEPOINT(" ");

        loopOK = fpsCTRL_TUI_process_user_key(ch,
                                              data.fpsarray,
                                              keywnode,
                                              fpsctrltasklist,
                                              fpsctrlqueuelist,
                                              &fpsCTRLvar);

        DEBUG_TRACEPOINT(" ");

        if (fpsCTRLvar.exitloop == 1)
        {
            loopOK = 0;
        }

        if (fpsCTRLvar.run_display == 1)
        {
            TUI_ncurses_erase();

            fpsCTRLscreen_print_DisplayMode_status(
                fpsCTRLvar.fpsCTRL_DisplayMode,
                fpsCTRLvar.NBfps);

            DEBUG_TRACEPOINT(" ");

            TUI_printfw(
                "======== FPSCTRL info  ( screen refresh cnt %7ld  "
                "scan interval %7ld us)",
                loopcnt,
                getchardt_us);
            TUI_newline();
            TUI_printfw(
                "    INPUT FIFO       :  %s (fd=%d)    fifocmdcnt = "
                "%ld   NBtaskLaunched = %d -> %d   [NB FPS = %d]",
                fpsCTRLvar.fpsCTRLfifoname,
                fpsCTRLvar.fpsCTRLfifofd,
                fifocmdcnt,
                NBtaskLaunched,
                NBtaskLaunchedcnt,
                fpsCTRLvar.NBfps);
            TUI_newline();

            DEBUG_TRACEPOINT(" ");
            char logfname[STRINGMAXLEN_FULLFILENAME];
            getFPSlogfname(logfname);
            TUI_printfw("    OUTPUT LOG       :  %s", logfname);
            TUI_newline();

            DEBUG_TRACEPOINT(" ");

            if (fpsCTRLvar.fpsCTRL_DisplayMode == 1) // help
            {
                fpsCTRLscreen_print_help();
            }

            if (fpsCTRLvar.fpsCTRL_DisplayMode == 2) // FPS content
            {

                DEBUG_TRACEPOINT("Check that selected node is OK : %d",
                                 fpsCTRLvar.nodeSelected);

                if (fpsCTRLvar.NBfps > 0)
                {
                    /*TUI_printfw("node selected : %d", fpsCTRLvar.nodeSelected);
                    TUI_newline();
                     TUI_printfw("full keyword :  %s", keywnode[fpsCTRLvar.nodeSelected].keywordfull);
                     TUI_newline();*/
                    if (strlen(keywnode[fpsCTRLvar.nodeSelected].keywordfull) <
                        1) // if not OK, set to last valid entry
                    {
                        fpsCTRLvar.nodeSelected = 1;
                        while ((strlen(keywnode[fpsCTRLvar.nodeSelected]
                                           .keywordfull) < 1) &&
                               (fpsCTRLvar.nodeSelected < NB_KEYWNODE_MAX))
                        {
                            fpsCTRLvar.nodeSelected++;
                        }
                    }

                    DEBUG_TRACEPOINT("Get info from selected node");
                    fpsCTRLvar.fpsindexSelected =
                        keywnode[fpsCTRLvar.nodeSelected].fpsindex;
                    fpsCTRLvar.pindexSelected =
                        keywnode[fpsCTRLvar.nodeSelected].pindex;
                    fpsCTRLscreen_print_nodeinfo(data.fpsarray,
                                                 keywnode,
                                                 fpsCTRLvar.nodeSelected,
                                                 fpsCTRLvar.fpsindexSelected,
                                                 fpsCTRLvar.pindexSelected);

                    DEBUG_TRACEPOINT("trace back node chain");
                    nodechain[fpsCTRLvar.currentlevel] =
                        fpsCTRLvar.directorynodeSelected;

                    TUI_printfw("[level %d %d] ",
                                fpsCTRLvar.currentlevel + 1,
                                nodechain[fpsCTRLvar.currentlevel + 1]);

                    if (fpsCTRLvar.currentlevel > 0)
                    {
                        TUI_printfw("[level %d %d] ",
                                    fpsCTRLvar.currentlevel,
                                    nodechain[fpsCTRLvar.currentlevel]);
                    }
                    level = fpsCTRLvar.currentlevel - 1;
                    while (level > 0)
                    {
                        nodechain[level] =
                            keywnode[nodechain[level + 1]].parent_index;
                        TUI_printfw("[level %d %d] ", level, nodechain[level]);
                        level--;
                    }
                    TUI_printfw("[level 0 0]");
                    TUI_newline();
                    nodechain[0] = 0; // root

                    DEBUG_TRACEPOINT("Get number of lines to be displayed");
                    fpsCTRLvar.currentlevel =
                        keywnode[fpsCTRLvar.directorynodeSelected].keywordlevel;
                    int GUIlineMax =
                        keywnode[fpsCTRLvar.directorynodeSelected].NBchild;
                    for (level = 0; level < fpsCTRLvar.currentlevel; level++)
                    {
                        DEBUG_TRACEPOINT(
                            "update GUIlineMax, the "
                            "maximum number of lines");
                        if (keywnode[nodechain[level]].NBchild > GUIlineMax)
                        {
                            GUIlineMax = keywnode[nodechain[level]].NBchild;
                        }
                    }

                    TUI_printfw(
                        "[node %d] level = %d   [%d] NB child "
                        "= %d",
                        fpsCTRLvar.nodeSelected,
                        fpsCTRLvar.currentlevel,
                        fpsCTRLvar.directorynodeSelected,
                        keywnode[fpsCTRLvar.directorynodeSelected].NBchild);

                    TUI_printfw("   fps %d", fpsCTRLvar.fpsindexSelected);

                    TUI_printfw("   pindex %d ",
                                keywnode[fpsCTRLvar.nodeSelected].pindex);

                    TUI_newline();

                    /*      TUI_printfw("SELECTED DIR = %3d    SELECTED = %3d   GUIlineMax= %3d",
                                 fpsCTRLvar.directorynodeSelected,
                                 fpsCTRLvar.nodeSelected,
                                 GUIlineMax);
                        TUI_newline();
                        TUI_newline();
                          TUI_printfw("LINE: %d / %d",
                                 fpsCTRLvar.GUIlineSelected[fpsCTRLvar.currentlevel],
                                 keywnode[fpsCTRLvar.directorynodeSelected].NBchild);
                                 TUI_newline();
                                 TUI_newline();
                    	*/

                    //while(!(fps[fpsindexSelected].parray[pindexSelected].fpflag & FPFLAG_VISIBLE)) { // if invisible
                    //		fpsCTRLvar.GUIlineSelected[fpsCTRLvar.currentlevel]++;
                    //}

                    //if(!(fps[fpsindex].parray[pindex].fpflag & FPFLAG_VISIBLE)) { // if invisible

                    //              if( !(  fps[keywnode[fpsCTRLvar.nodeSelected].fpsindex].parray[keywnode[fpsCTRLvar.nodeSelected].pindex].fpflag & FPFLAG_VISIBLE)) { // if invisible
                    //				if( !(  fps[fpsCTRLvar.fpsindexSelected].parray[fpsCTRLvar.pindexSelected].fpflag & FPFLAG_VISIBLE)) { // if invisible
                    if (!(data.fpsarray[fpsCTRLvar.fpsindexSelected]
                              .parray[0]
                              .fpflag &
                          FPFLAG_VISIBLE)) // if invisible
                    {
                        if (fpsCTRLvar.direction > 0)
                        {
                            fpsCTRLvar
                                .GUIlineSelected[fpsCTRLvar.currentlevel]++;
                        }
                        else
                        {
                            fpsCTRLvar
                                .GUIlineSelected[fpsCTRLvar.currentlevel]--;
                        }
                    }

                    while (fpsCTRLvar.GUIlineSelected[fpsCTRLvar.currentlevel] >
                           keywnode[fpsCTRLvar.directorynodeSelected].NBchild -
                               1)
                    {
                        fpsCTRLvar.GUIlineSelected[fpsCTRLvar.currentlevel]--;
                    }

                    int child_index[MAXNBLEVELS];
                    for (level = 0; level < MAXNBLEVELS; level++)
                    {
                        child_index[level] = 0;
                    }

                    for (int GUIline = 0; GUIline < GUIlineMax;
                         GUIline++) // GUIline is the line number on GUI display
                    {

                        for (level = 0; level < fpsCTRLvar.currentlevel;
                             level++)
                        {

                            if (GUIline < keywnode[nodechain[level]].NBchild)
                            {
                                int snode = 0; // selected node
                                int knodeindex;

                                knodeindex =
                                    keywnode[nodechain[level]].child[GUIline];

                                //TODO: adjust len to string
                                char pword[100];

                                if (level == 0)
                                {
                                    DEBUG_TRACEPOINT(
                                        "provid"
                                        "e a "
                                        "fps "
                                        "status"
                                        " summa"
                                        "ry if "
                                        "at "
                                        "root");
                                    fpsindex = keywnode[knodeindex].fpsindex;
                                    fpsCTRLscreen_level0node_summary(
                                        data.fpsarray,
                                        fpsindex);
                                }

                                // toggle highlight if node is in the chain
                                int v1 =
                                    keywnode[nodechain[level]].child[GUIline];
                                int v2 = nodechain[level + 1];
                                if (v1 == v2)
                                {
                                    snode = 1;
                                    screenprint_setreverse();
                                }

                                // color node if directory
                                if (keywnode[knodeindex].leaf == 0)
                                {
                                    screenprint_setcolor(5);
                                }

                                // print keyword
                                if (snprintf(pword,
                                             10,
                                             "%s",
                                             keywnode[keywnode[nodechain[level]]
                                                          .child[GUIline]]
                                                 .keyword[level]) < 0)
                                {
                                    PRINT_ERROR(
                                        "snprin"
                                        "tf "
                                        "erro"
                                        "r");
                                }
                                TUI_printfw("%-10s ", pword);

                                if (keywnode[knodeindex].leaf == 0) // directory
                                {
                                    screenprint_unsetcolor(5);
                                }

                                screenprint_setreverse();
                                if (snode == 1)
                                {
                                    TUI_printfw(">");
                                }
                                else
                                {
                                    TUI_printfw(" ");
                                }
                                screenprint_unsetreverse();
                                screenprint_setnormal();
                            }
                            else // blank space
                            {
                                if (level == 0)
                                {
                                    TUI_printfw(
                                        "      "
                                        "      "
                                        "      "
                                        "  ");
                                }
                                TUI_printfw("            ");
                            }
                        }

                        int knodeindex;
                        knodeindex = keywnode[fpsCTRLvar.directorynodeSelected]
                                         .child[child_index[level]];
                        if (knodeindex < fpsCTRLvar.NBkwn)
                        {
                            fpsindex = keywnode[knodeindex].fpsindex;
                            pindex   = keywnode[knodeindex].pindex;

                            if (child_index[level] >
                                keywnode[fpsCTRLvar.directorynodeSelected]
                                        .NBchild -
                                    1)
                            {
                                child_index[level] =
                                    keywnode[fpsCTRLvar.directorynodeSelected]
                                        .NBchild -
                                    1;
                            }

                            /*
                                                    if(fpsCTRLvar.currentlevel != 0) { // this does not apply to root menu
                                                        while((!(fps[fpsindex].parray[pindex].fpflag & FPFLAG_VISIBLE)) && // if not visible, advance to next one
                                                                (child_index[level] < keywnode[fpsCTRLvar.directorynodeSelected].NBchild-1)) {
                                                            child_index[level] ++;
                                                            DEBUG_TRACEPOINT("knodeindex = %d  child %d / %d",
                                                                      knodeindex,
                                                                      child_index[level],
                                                                      keywnode[fpsCTRLvar.directorynodeSelected].NBchild);
                                                            knodeindex = keywnode[fpsCTRLvar.directorynodeSelected].child[child_index[level]];
                                                            fpsindex = keywnode[knodeindex].fpsindex;
                                                            pindex = keywnode[knodeindex].pindex;
                                                        }
                                                    }
                            */

                            DEBUG_TRACEPOINT(" ");

                            if (child_index[level] <
                                keywnode[fpsCTRLvar.directorynodeSelected]
                                    .NBchild)
                            {

                                if (fpsCTRLvar.currentlevel > 0)
                                {
                                    screenprint_setreverse();
                                    TUI_printfw(" ");
                                    screenprint_unsetreverse();
                                }

                                DEBUG_TRACEPOINT(" ");

                                if (keywnode[knodeindex].leaf ==
                                    0) // If this is a directory
                                {
                                    DEBUG_TRACEPOINT(" ");
                                    if (fpsCTRLvar.currentlevel ==
                                        0) // provide a status summary if at root
                                    {
                                        DEBUG_TRACEPOINT(" ");

                                        fpsindex =
                                            keywnode[knodeindex].fpsindex;
                                        pid_t pid;

                                        pid =
                                            data.fpsarray[fpsindex].md->confpid;
                                        if ((getpgid(pid) >= 0) && (pid > 0))
                                        {
                                            screenprint_setcolor(2);
                                            TUI_printfw("%07d ", (int) pid);
                                            screenprint_unsetcolor(2);
                                        }
                                        else // PID not active
                                        {
                                            if (data.fpsarray[fpsindex]
                                                    .md->status &
                                                FUNCTION_PARAMETER_STRUCT_STATUS_CMDCONF)
                                            {
                                                // not clean exit
                                                screenprint_setcolor(4);
                                                TUI_printfw("%07d ", (int) pid);
                                                screenprint_unsetcolor(4);
                                            }
                                            else
                                            {
                                                // All OK
                                                TUI_printfw("%07d ", (int) pid);
                                            }
                                        }

                                        if (data.fpsarray[fpsindex]
                                                .md->conferrcnt > 99)
                                        {
                                            screenprint_setcolor(4);
                                            TUI_printfw("[XX]");
                                            screenprint_unsetcolor(4);
                                        }
                                        if (data.fpsarray[fpsindex]
                                                .md->conferrcnt > 0)
                                        {
                                            screenprint_setcolor(4);
                                            TUI_printfw("[%02d]",
                                                        data.fpsarray[fpsindex]
                                                            .md->conferrcnt);
                                            screenprint_unsetcolor(4);
                                        }
                                        if (data.fpsarray[fpsindex]
                                                .md->conferrcnt == 0)
                                        {
                                            screenprint_setcolor(2);
                                            TUI_printfw("[%02d]",
                                                        data.fpsarray[fpsindex]
                                                            .md->conferrcnt);
                                            screenprint_unsetcolor(2);
                                        }

                                        pid =
                                            data.fpsarray[fpsindex].md->runpid;
                                        if ((getpgid(pid) >= 0) && (pid > 0))
                                        {
                                            screenprint_setcolor(2);
                                            TUI_printfw("%07d ", (int) pid);
                                            screenprint_unsetcolor(2);
                                        }
                                        else
                                        {
                                            if (data.fpsarray[fpsindex]
                                                    .md->status &
                                                FUNCTION_PARAMETER_STRUCT_STATUS_CMDRUN)
                                            {
                                                // not clean exit
                                                screenprint_setcolor(4);
                                                TUI_printfw("%07d ", (int) pid);
                                                screenprint_unsetcolor(4);
                                            }
                                            else
                                            {
                                                // All OK
                                                TUI_printfw("%07d ", (int) pid);
                                            }
                                        }
                                    }

                                    if (GUIline ==
                                        fpsCTRLvar.GUIlineSelected
                                            [fpsCTRLvar.currentlevel])
                                    {
                                        screenprint_setreverse();
                                        fpsCTRLvar.nodeSelected = knodeindex;
                                        fpsCTRLvar.fpsindexSelected =
                                            keywnode[knodeindex].fpsindex;
                                    }

                                    if (child_index[level + 1] <
                                        keywnode[fpsCTRLvar
                                                     .directorynodeSelected]
                                            .NBchild)
                                    {
                                        screenprint_setcolor(5);
                                        level =
                                            keywnode[knodeindex].keywordlevel;
                                        TUI_printfw("%-16s",
                                                    keywnode[knodeindex]
                                                        .keyword[level - 1]);
                                        screenprint_unsetcolor(5);

                                        if (GUIline ==
                                            fpsCTRLvar.GUIlineSelected
                                                [fpsCTRLvar.currentlevel])
                                        {
                                            screenprint_unsetreverse();
                                        }
                                    }
                                    else
                                    {
                                        TUI_printfw("%-16s", " ");
                                    }

                                    DEBUG_TRACEPOINT(" ");
                                }
                                else // If this is a parameter
                                {
                                    DEBUG_TRACEPOINT(" ");
                                    fpsindex = keywnode[knodeindex].fpsindex;
                                    pindex   = keywnode[knodeindex].pindex;

                                    DEBUG_TRACEPOINT(" ");
                                    int isVISIBLE = 1;
                                    if (!(data.fpsarray[fpsindex]
                                              .parray[pindex]
                                              .fpflag &
                                          FPFLAG_VISIBLE)) // if invisible
                                    {
                                        isVISIBLE = 0;
                                        screenprint_setdim();
                                        screenprint_setblink();
                                    }

                                    //int kl;

                                    if (GUIline ==
                                        fpsCTRLvar.GUIlineSelected
                                            [fpsCTRLvar.currentlevel])
                                    {
                                        fpsCTRLvar.pindexSelected =
                                            keywnode[knodeindex].pindex;
                                        fpsCTRLvar.fpsindexSelected =
                                            keywnode[knodeindex].fpsindex;
                                        fpsCTRLvar.nodeSelected = knodeindex;

                                        if (isVISIBLE == 1)
                                        {
                                            screenprint_setcolor(10);
                                            screenprint_setbold();
                                        }
                                    }
                                    DEBUG_TRACEPOINT(" ");

                                    if (isVISIBLE == 1)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_WRITESTATUS)
                                        {
                                            screenprint_setcolor(10);
                                            screenprint_setblink();
                                            TUI_printfw("W "); // writable
                                            screenprint_unsetcolor(10);
                                            screenprint_unsetblink();
                                        }
                                        else
                                        {
                                            screenprint_setcolor(4);
                                            screenprint_setblink();
                                            TUI_printfw("NW"); // non writable
                                            screenprint_unsetcolor(4);
                                            screenprint_unsetblink();
                                        }
                                    }
                                    else
                                    {
                                        TUI_printfw("  ");
                                    }

                                    DEBUG_TRACEPOINT(" ");
                                    level = keywnode[knodeindex].keywordlevel;

                                    if (GUIline ==
                                        fpsCTRLvar.GUIlineSelected
                                            [fpsCTRLvar.currentlevel])
                                    {
                                        screenprint_setreverse();
                                    }

                                    TUI_printfw(
                                        " %-"
                                        "20s",
                                        data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .keyword[level - 1]);

                                    if (GUIline ==
                                        fpsCTRLvar.GUIlineSelected
                                            [fpsCTRLvar.currentlevel])
                                    {
                                        screenprint_unsetcolor(10);
                                        screenprint_unsetreverse();
                                    }
                                    DEBUG_TRACEPOINT(" ");
                                    TUI_printfw("   ");

                                    // VALUE

                                    int paramsync =
                                        1; // parameter is synchronized

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .fpflag &
                                        FPFLAG_ERROR) // parameter setting error
                                    {
                                        if (isVISIBLE == 1)
                                        {
                                            screenprint_setcolor(4);
                                        }
                                    }

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_UNDEF)
                                    {
                                        TUI_printfw("  %s", "-undef-");
                                    }

                                    DEBUG_TRACEPOINT(" ");

                                    // integer types
                                    {
                                        long val0 = 0;
                                        long val3 = 0;
                                        int  intflag =
                                            0; // toggles to 1 if int type
                                        switch (data.fpsarray[fpsindex]
                                                    .parray[pindex]
                                                    .type)
                                        {
                                        case FPTYPE_INT32:
                                            val0 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.i32[0];
                                            val3 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.i32[3];
                                            intflag = 1;
                                            break;
                                        case FPTYPE_UINT32:
                                            val0 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.ui32[0];
                                            val3 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.ui32[3];
                                            intflag = 1;
                                            break;
                                        case FPTYPE_INT64:
                                            val0 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.i64[0];
                                            val3 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.i64[3];
                                            intflag = 1;
                                            break;
                                        case FPTYPE_UINT64:
                                            val0 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.ui64[0];
                                            val3 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.ui64[3];
                                            intflag = 1;
                                            break;
                                        }

                                        if (intflag == 1)
                                        {
                                            if (data.fpsarray[fpsindex]
                                                    .parray[pindex]
                                                    .fpflag &
                                                FPFLAG_FEEDBACK)
                                            {
                                                // Check value feedback if available
                                                if (!(data.fpsarray[fpsindex]
                                                          .parray[pindex]
                                                          .fpflag &
                                                      FPFLAG_ERROR))
                                                    if (val0 != val3)
                                                    {
                                                        paramsync = 0;
                                                    }
                                            }

                                            if (paramsync == 0)
                                            {
                                                if (isVISIBLE == 1)
                                                {
                                                    screenprint_setcolor(3);
                                                }
                                            }

                                            TUI_printfw("  %10d", (int) val0);

                                            if (paramsync == 0)
                                            {
                                                if (isVISIBLE == 1)
                                                {
                                                    screenprint_unsetcolor(3);
                                                }
                                            }
                                        }
                                    }

                                    DEBUG_TRACEPOINT(" ");

                                    // float types
                                    {
                                        double val0 = 0.0;
                                        double val3 = 0.0;
                                        int    floatflag =
                                            0; // toggles to 1 if int type
                                        switch (data.fpsarray[fpsindex]
                                                    .parray[pindex]
                                                    .type)
                                        {
                                        case FPTYPE_FLOAT32:
                                            val0 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.f32[0];
                                            val3 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.f32[3];
                                            floatflag = 1;
                                            break;
                                        case FPTYPE_FLOAT64:
                                            val0 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.f64[0];
                                            val3 = data.fpsarray[fpsindex]
                                                       .parray[pindex]
                                                       .val.f64[3];
                                            floatflag = 1;
                                            break;
                                        }

                                        if (floatflag == 1)
                                        {
                                            if (data.fpsarray[fpsindex]
                                                    .parray[pindex]
                                                    .fpflag &
                                                FPFLAG_FEEDBACK) // Check value feedback if available
                                                if (!(data.fpsarray[fpsindex]
                                                          .parray[pindex]
                                                          .fpflag &
                                                      FPFLAG_ERROR))
                                                {
                                                    double absdiff;
                                                    double abssum;
                                                    double epsrel = 1.0e-6;
                                                    double epsabs = 1.0e-10;

                                                    absdiff = fabs(val0 - val3);
                                                    abssum =
                                                        fabs(val0) + fabs(val3);

                                                    if ((absdiff <
                                                         epsrel * abssum) ||
                                                        (absdiff < epsabs))
                                                    {
                                                        paramsync = 1;
                                                    }
                                                    else
                                                    {
                                                        paramsync = 0;
                                                    }
                                                }

                                            if (paramsync == 0)
                                            {
                                                if (isVISIBLE == 1)
                                                {
                                                    screenprint_setcolor(3);
                                                }
                                            }

                                            TUI_printfw("  %10f", (float) val0);

                                            if (paramsync == 0)
                                            {
                                                if (isVISIBLE == 1)
                                                {
                                                    screenprint_unsetcolor(3);
                                                }
                                            }
                                        }
                                    }

                                    DEBUG_TRACEPOINT(" ");
                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_PID)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                if (data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.pid[0] !=
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.pid[1])
                                                {
                                                    paramsync = 0;
                                                }

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(3);
                                            }
                                        }

                                        TUI_printfw(
                                            "  %10d",
                                            (float) data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .val.pid[0]);

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(3);
                                            }
                                        }

                                        TUI_printfw(
                                            "  %10d",
                                            (int) data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .val.pid[0]);
                                    }

                                    DEBUG_TRACEPOINT(" ");

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_TIMESPEC)
                                    {
                                        TUI_printfw(
                                            "  %10f",
                                            1.0 * data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.ts[0]
                                                        .tv_sec +
                                                1e-9 * data.fpsarray[fpsindex]
                                                           .parray[pindex]
                                                           .val.ts[0]
                                                           .tv_nsec);
                                    }

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_FILENAME)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                if (strcmp(
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[0],
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[1]))
                                                {
                                                    paramsync = 0;
                                                }

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(3);
                                            }
                                        }

                                        TUI_printfw("  %10s",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.string[0]);

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(3);
                                            }
                                        }
                                    }
                                    DEBUG_TRACEPOINT(" ");

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_FITSFILENAME)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                if (strcmp(
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[0],
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[1]))
                                                {
                                                    paramsync = 0;
                                                }

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(3);
                                            }
                                        }

                                        TUI_printfw("  %10s",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.string[0]);

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(3);
                                            }
                                        }
                                    }
                                    DEBUG_TRACEPOINT(" ");
                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_EXECFILENAME)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                if (strcmp(
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[0],
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[1]))
                                                {
                                                    paramsync = 0;
                                                }

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(3);
                                            }
                                        }

                                        TUI_printfw("  %10s",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.string[0]);

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(3);
                                            }
                                        }
                                    }
                                    DEBUG_TRACEPOINT(" ");
                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_DIRNAME)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                if (strcmp(
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[0],
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[1]))
                                                {
                                                    paramsync = 0;
                                                }

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(3);
                                            }
                                        }

                                        TUI_printfw("  %10s",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.string[0]);

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(3);
                                            }
                                        }
                                    }

                                    DEBUG_TRACEPOINT(" ");
                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_STREAMNAME)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                //  if(strcmp(fps[fpsindex].parray[pindex].val.string[0], fps[fpsindex].parray[pindex].val.string[1])) {
                                                //      paramsync = 0;
                                                //  }

                                                if (data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .info.stream.streamID >
                                                    -1)
                                                {
                                                    if (isVISIBLE == 1)
                                                    {
                                                        screenprint_setcolor(2);
                                                    }
                                                }

                                        TUI_printfw("[LOC %d]  %6s",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .info.stream
                                                        .stream_sourceLocation,
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.string[0]);

                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .info.stream.streamID > -1)
                                        {

                                            TUI_printfw(" [ %d",
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .info.stream
                                                            .stream_xsize[0]);
                                            if (data.fpsarray[fpsindex]
                                                    .parray[pindex]
                                                    .info.stream
                                                    .stream_naxis[0] > 1)
                                            {
                                                TUI_printfw(
                                                    "x%d",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .info.stream
                                                        .stream_ysize[0]);
                                            }
                                            if (data.fpsarray[fpsindex]
                                                    .parray[pindex]
                                                    .info.stream
                                                    .stream_naxis[0] > 2)
                                            {
                                                TUI_printfw(
                                                    "x%d",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .info.stream
                                                        .stream_zsize[0]);
                                            }

                                            TUI_printfw(" ]");
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(2);
                                            }
                                        }
                                    }
                                    DEBUG_TRACEPOINT(" ");

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_STRING)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                if (strcmp(
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[0],
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[1]))
                                                {
                                                    paramsync = 0;
                                                }

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(3);
                                            }
                                        }

                                        TUI_printfw("  %10s",
                                                    data.fpsarray[fpsindex]
                                                        .parray[pindex]
                                                        .val.string[0]);

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(3);
                                            }
                                        }
                                    }
                                    DEBUG_TRACEPOINT(" ");

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_ONOFF)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_ONOFF)
                                        {
                                            screenprint_setcolor(2);
                                            TUI_printfw("  ON ");
                                            screenprint_unsetcolor(2);
                                            //TUI_printfw(" [ON  :%3s]", data.fpsarray[fpsindex].parray[pindex].val.string[0]);
                                        }
                                        else
                                        {
                                            screenprint_setcolor(1);
                                            TUI_printfw(" OFF ");
                                            screenprint_unsetcolor(1);
                                            //TUI_printfw(" [OFF :%3s]", data.fpsarray[fpsindex].parray[pindex].val.string[1]);
                                        }
                                    }

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .type == FPTYPE_FPSNAME)
                                    {
                                        if (data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .fpflag &
                                            FPFLAG_FEEDBACK) // Check value feedback if available
                                            if (!(data.fpsarray[fpsindex]
                                                      .parray[pindex]
                                                      .fpflag &
                                                  FPFLAG_ERROR))
                                                if (strcmp(
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[0],
                                                        data.fpsarray[fpsindex]
                                                            .parray[pindex]
                                                            .val.string[1]))
                                                {
                                                    paramsync = 0;
                                                }

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(2);
                                            }
                                        }
                                        else
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_setcolor(4);
                                            }
                                        }

                                        TUI_printfw(
                                            " %10s [%ld %ld %ld]",
                                            data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .val.string[0],
                                            data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .info.fps.FPSNBparamMAX,
                                            data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .info.fps.FPSNBparamActive,
                                            data.fpsarray[fpsindex]
                                                .parray[pindex]
                                                .info.fps.FPSNBparamUsed);

                                        if (paramsync == 0)
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(2);
                                            }
                                        }
                                        else
                                        {
                                            if (isVISIBLE == 1)
                                            {
                                                screenprint_unsetcolor(4);
                                            }
                                        }
                                    }

                                    DEBUG_TRACEPOINT(" ");

                                    if (data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .fpflag &
                                        FPFLAG_ERROR) // parameter setting error
                                    {
                                        if (isVISIBLE == 1)
                                        {
                                            screenprint_unsetcolor(4);
                                        }
                                    }

                                    TUI_printfw(
                                        "    "
                                        "%s",
                                        data.fpsarray[fpsindex]
                                            .parray[pindex]
                                            .description);

                                    if (GUIline ==
                                        fpsCTRLvar.GUIlineSelected
                                            [fpsCTRLvar.currentlevel])
                                    {
                                        if (isVISIBLE == 1)
                                        {
                                            screenprint_unsetbold();
                                        }
                                    }

                                    if (isVISIBLE == 0)
                                    {
                                        screenprint_unsetblink();
                                        screenprint_unsetdim();
                                    }
                                    // END LOOP
                                }

                                DEBUG_TRACEPOINT(" ");
                                icnt++;

                                for (level = 0; level < MAXNBLEVELS; level++)
                                {
                                    child_index[level]++;
                                }
                            }
                        }

                        TUI_newline();
                    }

                    DEBUG_TRACEPOINT(" ");

                    fpsCTRLvar.NBindex = icnt;

                    if (fpsCTRLvar.GUIlineSelected[fpsCTRLvar.currentlevel] >
                        fpsCTRLvar.NBindex - 1)
                    {
                        fpsCTRLvar.GUIlineSelected[fpsCTRLvar.currentlevel] =
                            fpsCTRLvar.NBindex - 1;
                    }

                    DEBUG_TRACEPOINT(" ");

                    TUI_newline();

                    if (data.fpsarray[fpsCTRLvar.fpsindexSelected].md->status &
                        FUNCTION_PARAMETER_STRUCT_STATUS_CHECKOK)
                    {
                        screenprint_setcolor(2);
                        TUI_printfw(
                            "[%ld] PARAMETERS OK - RUN "
                            "function good to go",
                            data.fpsarray[fpsCTRLvar.fpsindexSelected]
                                .md->msgcnt);
                        screenprint_unsetcolor(2);
                        TUI_newline();
                    }
                    else
                    {
                        int msgi;

                        screenprint_setcolor(4);
                        TUI_printfw(
                            "[%ld] %d PARAMETER SETTINGS "
                            "ERROR(s) :",
                            data.fpsarray[fpsCTRLvar.fpsindexSelected]
                                .md->msgcnt,
                            data.fpsarray[fpsCTRLvar.fpsindexSelected]
                                .md->conferrcnt);
                        screenprint_unsetcolor(4);
                        TUI_newline();

                        screenprint_setbold();

                        for (msgi = 0;
                             msgi < data.fpsarray[fpsCTRLvar.fpsindexSelected]
                                        .md->msgcnt;
                             msgi++)
                        {
                            pindex = data.fpsarray[fpsCTRLvar.fpsindexSelected]
                                         .md->msgpindex[msgi];
                            TUI_printfw(
                                "%-40s %s",
                                data.fpsarray[fpsCTRLvar.fpsindexSelected]
                                    .parray[pindex]
                                    .keywordfull,
                                data.fpsarray[fpsCTRLvar.fpsindexSelected]
                                    .md->message[msgi]);
                            TUI_newline();
                        }

                        screenprint_unsetbold();
                    }

                    DEBUG_TRACEPOINT(" ");
                }
            }

            DEBUG_TRACEPOINT(" ");

            if (fpsCTRLvar.fpsCTRL_DisplayMode == 3) // Task scheduler status
            {
                struct timespec tnow;
                struct timespec tdiff;

                clock_gettime(CLOCK_REALTIME, &tnow);

                //int dispcnt = 0;

                // Sort entries from most recent to most ancient, using inputindex
                DEBUG_TRACEPOINT(" ");

                double *sort_evalarray;
                sort_evalarray =
                    (double *) malloc(sizeof(double) * NB_FPSCTRL_TASK_MAX);
                if (sort_evalarray == NULL)
                {
                    PRINT_ERROR("malloc error");
                    abort();
                }

                long *sort_indexarray;
                sort_indexarray =
                    (long *) malloc(sizeof(long) * NB_FPSCTRL_TASK_MAX);
                if (sort_indexarray == NULL)
                {
                    PRINT_ERROR("malloc error");
                    abort();
                }

                long sortcnt = 0;
                for (int fpscmdindex = 0; fpscmdindex < NB_FPSCTRL_TASK_MAX;
                     fpscmdindex++)
                {
                    if (fpsctrltasklist[fpscmdindex].status &
                        FPSTASK_STATUS_SHOW)
                    {
                        sort_evalarray[sortcnt] =
                            -1.0 * fpsctrltasklist[fpscmdindex].inputindex;
                        sort_indexarray[sortcnt] = fpscmdindex;
                        sortcnt++;
                    }
                }
                DEBUG_TRACEPOINT(" ");
                if (sortcnt > 0)
                {
                    quick_sort2l(sort_evalarray, sort_indexarray, sortcnt);
                }
                free(sort_evalarray);

                DEBUG_TRACEPOINT(" ");

                TUI_printfw(" showing   %d / %d  tasks", wrow - 8, sortcnt);
                TUI_newline();

                for (int sortindex = 0; sortindex < sortcnt; sortindex++)
                {

                    DEBUG_TRACEPOINT("iteration %d / %ld", sortindex, sortcnt);

                    int fpscmdindex = sort_indexarray[sortindex];

                    DEBUG_TRACEPOINT("fpscmdindex = %d", fpscmdindex);

                    if (sortindex < wrow - 8) // display
                    {
                        int attron2  = 0;
                        int attrbold = 0;

                        if (fpsctrltasklist[fpscmdindex].status &
                            FPSTASK_STATUS_RUNNING) // task is running
                        {
                            attron2 = 1;
                            screenprint_setcolor(2);
                        }
                        else if (fpsctrltasklist[fpscmdindex].status &
                                 FPSTASK_STATUS_ACTIVE) // task is queued to run
                        {
                            attrbold = 1;
                            screenprint_setbold();
                        }

                        // measure age since submission
                        tdiff = timespec_diff(
                            fpsctrltasklist[fpscmdindex].creationtime,
                            tnow);
                        double tdiffv =
                            1.0 * tdiff.tv_sec + 1.0e-9 * tdiff.tv_nsec;
                        TUI_printfw("%6.2f s ", tdiffv);

                        if (fpsctrltasklist[fpscmdindex].status &
                            FPSTASK_STATUS_RUNNING) // run time (ongoing)
                        {
                            tdiff = timespec_diff(
                                fpsctrltasklist[fpscmdindex].activationtime,
                                tnow);
                            tdiffv =
                                1.0 * tdiff.tv_sec + 1.0e-9 * tdiff.tv_nsec;
                            TUI_printfw(" %6.2f s ", tdiffv);
                        }
                        else if (!(fpsctrltasklist[fpscmdindex].status &
                                   FPSTASK_STATUS_ACTIVE)) // run time (past)
                        {
                            tdiff = timespec_diff(
                                fpsctrltasklist[fpscmdindex].activationtime,
                                fpsctrltasklist[fpscmdindex].completiontime);
                            tdiffv =
                                1.0 * tdiff.tv_sec + 1.0e-9 * tdiff.tv_nsec;
                            screenprint_setcolor(3);
                            TUI_printfw(" %6.2f s ", tdiffv);
                            screenprint_unsetcolor(3);
                            // age since completion
                            tdiff = timespec_diff(
                                fpsctrltasklist[fpscmdindex].completiontime,
                                tnow);
                            double tdiffv = tdiffv =
                                1.0 * tdiff.tv_sec + 1.0e-9 * tdiff.tv_nsec;
                            //printfw("<%6.2f s>      ", tdiffv);

                            //if(tdiffv > 30.0)
                            //fpsctrltasklist[fpscmdindex].status &= ~FPSTASK_STATUS_SHOW;
                        }
                        else
                        {
                            TUI_printfw("          ", tdiffv);
                        }

                        if (fpsctrltasklist[fpscmdindex].status &
                            FPSTASK_STATUS_ACTIVE)
                        {
                            TUI_printfw(">>");
                        }
                        else
                        {
                            TUI_printfw("  ");
                        }

                        if (fpsctrltasklist[fpscmdindex].flag &
                            FPSTASK_FLAG_WAITONRUN)
                        {
                            TUI_printfw("WR ");
                        }
                        else
                        {
                            TUI_printfw("   ");
                        }

                        if (fpsctrltasklist[fpscmdindex].flag &
                            FPSTASK_FLAG_WAITONCONF)
                        {
                            TUI_printfw("WC ");
                        }
                        else
                        {
                            TUI_printfw("   ");
                        }

                        TUI_printfw(
                            "[Q:%02d P:%02d] %4d",
                            fpsctrltasklist[fpscmdindex].queue,
                            fpsctrlqueuelist[fpsctrltasklist[fpscmdindex].queue]
                                .priority,
                            fpscmdindex);

                        if (fpsctrltasklist[fpscmdindex].status &
                            FPSTASK_STATUS_RECEIVED)
                        {
                            TUI_printfw(" R");
                        }
                        else
                        {
                            TUI_printfw(" -");
                        }

                        if (fpsctrltasklist[fpscmdindex].status &
                            FPSTASK_STATUS_CMDNOTFOUND)
                        {
                            screenprint_setcolor(3);
                            TUI_printfw(" NOTCMD");
                            screenprint_unsetcolor(3);
                        }
                        else if (fpsctrltasklist[fpscmdindex].status &
                                 FPSTASK_STATUS_CMDFAIL)
                        {
                            screenprint_setcolor(4);
                            TUI_printfw(" FAILED");
                            if (fpsctrltasklist[fpscmdindex].status &
                                FPSTASK_STATUS_ERR_NBARG)
                            {
                                TUI_printfw(" NBARG");
                            }
                            if (fpsctrltasklist[fpscmdindex].status &
                                FPSTASK_STATUS_ERR_ARGTYPE)
                            {
                                TUI_printfw(" ARGTYPE");
                            }
                            if (fpsctrltasklist[fpscmdindex].status &
                                FPSTASK_STATUS_ERR_TYPECONV)
                            {
                                TUI_printfw(" TYPECOV");
                            }
                            if (fpsctrltasklist[fpscmdindex].status &
                                FPSTASK_STATUS_ERR_NOFPS)
                            {
                                TUI_printfw(" NOFPS");
                            }
                            screenprint_unsetcolor(4);
                        }
                        else if (fpsctrltasklist[fpscmdindex].status &
                                 FPSTASK_STATUS_CMDOK)
                        {
                            screenprint_setcolor(2);
                            TUI_printfw(" PROCOK");
                            screenprint_unsetcolor(2);
                        }
                        else if (fpsctrltasklist[fpscmdindex].status &
                                 FPSTASK_STATUS_RECEIVED)
                        {
                            screenprint_setcolor(2);
                            TUI_printfw(" RECVD ");
                            screenprint_unsetcolor(2);
                        }
                        else if (fpsctrltasklist[fpscmdindex].status &
                                 FPSTASK_STATUS_WAITING)
                        {
                            screenprint_setcolor(5);
                            TUI_printfw("WAITING");
                            screenprint_unsetcolor(5);
                        }
                        else
                        {
                            screenprint_setcolor(3);
                            TUI_printfw(" ????  ");
                            screenprint_unsetcolor(3);
                        }

                        TUI_printfw("  %s",
                                    fpsctrltasklist[fpscmdindex].cmdstring);
                        TUI_newline();

                        if (attron2 == 1)
                        {
                            screenprint_unsetcolor(2);
                        }
                        if (attrbold == 1)
                        {
                            screenprint_unsetbold();
                        }
                    }
                }
                free(sort_indexarray);
            }

            DEBUG_TRACEPOINT(" ");

            TUI_ncurses_refresh();

            DEBUG_TRACEPOINT(" ");

        } // end run_display

        DEBUG_TRACEPOINT("exit from if( fpsCTRLvar.run_display == 1)");

        fpsCTRLvar.run_display = run_display;

        loopcnt++;

        if ((data.signal_TERM == 1) || (data.signal_INT == 1) ||
            (data.signal_ABRT == 1) || (data.signal_BUS == 1) ||
            (data.signal_SEGV == 1) || (data.signal_HUP == 1) ||
            (data.signal_PIPE == 1))
        {
            printf("Exit condition met\n");
            loopOK = 0;
        }
    }

    if (run_display == 1)
    {
        TUI_exit();
    }

    functionparameter_outlog("FPSCTRL", "STOP");

    DEBUG_TRACEPOINT("Disconnect from FPS entries");
    for (fpsindex = 0; fpsindex < fpsCTRLvar.NBfps; fpsindex++)
    {
        function_parameter_struct_disconnect(&data.fpsarray[fpsindex]);
    }

    // free(fps);
    free(keywnode);

    free(fpsctrltasklist);
    free(fpsctrlqueuelist);
    functionparameter_outlog("LOGFILECLOSE", "close log file");

    DEBUG_TRACEPOINT("normal exit from function");

    return RETURN_SUCCESS;
}
