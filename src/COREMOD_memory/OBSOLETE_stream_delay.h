/** @file stream_delay,h
 */

errno_t stream_delay_addCLIcmd();

errno_t COREMOD_MEMORY_streamDelay(const char *IDin_name, const char *IDout_name, long delayus, long dtus);
