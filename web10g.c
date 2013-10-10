#include "web10g.h"

// web10g variables
estats_val_data* data = NULL;
struct estats_nl_client* cl = NULL;
struct estats_connection_list* clist = NULL;
struct estats_connection_tuple_ascii tuple_ascii;
struct estats_record* record = NULL;
int cid=-1; // local cid value
#define UREAD 500000 /* read web10g stats every N microseconds (1E6 usecs = 1 sec) */
pthread_t web10g_readthread; // thread that periodcally gathers web10g stats.
char web10g_close=0;
char data_notnull=0; // flag indicating if data has been written to yet.

// File Handling
#define WRITETIME 1 /* write to file every N seconds; set to 0 to disable */
FILE* fp_web10g; // the file associated with the web10g var dump
pthread_t writethread; // posix thread for writing to the file
char loopisdone=0; // set to 1 when the ssh conncetion is closed 


/* 
 * (private)
 * performs a 'readvars' and then dumps the results
 *   into the fp_web10g file.
 */
static estats_error*
dump_vars()
{
  struct estats_error* err = NULL;
  char* str;
  int i;
  
  for (i = 0; i < data->length; i++) {
    Chk(estats_val_as_string(&str, &data->val[i], estats_var_array[i].valtype));
    fprintf(fp_web10g, "%s,", str);
  }
  fprintf(fp_web10g,"\n");
  
Cleanup:
  return err;
}


/* 
 * (private)
 * Main function for the posix thread responsible for
 *   periodically reading web10g stats.
 *   Runs until the 'web10g_close' flag
 *   is set or when the read returns an error.
 * IMPORTANT - the web10g 'cid' var must be discovered
 *   before this function is invoked.
 */
static void 
web10gthread_handle(void *arg)
{
  struct estats_error* err = NULL;
  
  // read web10g stats periodcally
  while(!web10g_close) {
    Chk(estats_read_vars(data, cid, cl));
    data_notnull=1;
    usleep(UREAD);
  }

Cleanup:
  return;
}


/* 
 * (private)
 * Main function for the posix thread responsible for
 *   periodically writing to the fp_web10g file.
 *   Writes to the file until the 'loopisdone' flag
 *   is set or until dump_vars returns an error.
 */
static void 
writethread_handle(void *arg)
{
  struct estats_error* err = NULL;
  
  // open the file
  // TODO: graceful permissions
  fp_web10g=fopen("/tmp/test-file.txt", "w");
  
  // write to the file periodically
  while(!loopisdone) {
      if (data_notnull)
	Chk(dump_vars());
      sleep(WRITETIME);
    // start over
  }

Cleanup:
  // close the file
  fclose(fp_web10g);
  
  return;
}


/*
 * Initializes web10g variables.
 */
estats_error* web10g_init() {
  struct estats_error* err = NULL;
  Chk(estats_nl_client_init(&cl));
  Chk(estats_connection_list_new(&clist));
  Chk(estats_val_data_new(&data));

Cleanup:
  return err;
}


/*
 * Spawns a thread that periodically invokes the web10g readvars command.
 *   The main purpose of using a thread is so that the process does not
 *   need to call the web10g 'readvars' command repeatedly.
 * IMPORTANT - This command must be issued in order to obtain web10g stats.
 */
void web10g_start_readvars() {
#if UREAD
  pthread_create(&web10g_readthread, NULL, web10gthread_handle, NULL);
#else
  error("Error spawning thread - uread set to 0");
#endif
}


/*
 * Spawns a posix thread to periodically write to the
 *   fp_web10g file.
 */
void web10g_thread_file() {
#if WRITETIME
  // starts posix thread writehead, which runs function
  // writethread_handle and passes args NULL
  pthread_create(&writethread, NULL, writethread_handle, NULL);
#else
  error("Error creating file - writetime set to 0");
#endif
}

/* 
 * verifies that the kernel is web10g aware
 * by determining if net.ipv4.tcp_estats is
 * found. Just do a file check in /proc/sys
 * return int
 */
int web10g_check_kernel() 
{
	if (access("/proc/sys/net/ipv4/tcp_estats", F_OK) == 0)
		return 1;
	else
		return 0;
}

/*
 * verifies that the tcp_estats_nl kernel module is loaded
 * basically run lsmod and grep for tcp_estats_nl
 * return int
 */
int web10g_check_module()
{
	FILE *fp;
	char line[80];
	
	/* redirect stderr if the commands are not in the */
	/* right place we want to know */
	/* just get the one line we care about from lsmod */
	fp = popen("/bin/lsmod 2>&1  | /bin/grep tcp_estats_nl 2>&1", "r");
	fgets(line, sizeof line, fp);
	pclose(fp);
	char *f = strstr(line, "ound"); /* command not f'ound' */
	char *p = strstr(line, "tcp_estats_nl");
	/* we can't run the command properly so let someone know */
	if (f) 
		return -1;
	if (p)
		return 1;
	else
		return 0;
}

/*
 * Returns this process's local cid.
 */
int web10g_get_cid() 
{
  return cid;
}


/*
 * Set the value of the web10g local cid variable
 */
void web10g_set_cid(int tcid) 
{
  cid=tcid;
}


/*
 * Given a process's PID, find its CID.
 */
estats_error* web10g_find_cid(pid_t pid) 
{
  struct estats_error* err = NULL;
  struct estats_connection_info* ci;
  
  // set the value of cid<0
  cid=-1;
  
  // populate the connections list
  Chk(estats_list_conns(clist, cl));
  Chk(estats_connection_list_add_info(clist));
  
  // for each connection, compare its pid with the parameter's pid
  list_for_each(&clist->connection_info_head, ci, list) {
    Chk(estats_connection_tuple_as_strings(&tuple_ascii, &ci->tuple)); 
    // grab the cid whose pid matches the parameter
    if (ci->pid==pid) {
      cid=ci->cid;
      break;
    }
  }
  
  // if cid was never assigned a value, there was an error.
  if (cid<0) {
    error("CID Not Found. Did you modprobe tcp_estats_nl?");
  }
  
Cleanup:
  return err;
}


/*
 * Writes a record based on the process's current web10g statistics
 */
estats_error* web10g_writerecord(int cid) 
{
  struct estats_error* err = NULL;
  int i;
  char* str;
  
  // IMPORTANT - The file-writing thread must be
  //   terminated before record writing can continue.
#if WRITETIME
  loopisdone=1;
  pthread_join(writethread, NULL);
#endif
#if UREAD
  web10g_close=1;
  pthread_join(web10g_readthread, NULL);
#endif
  
  // This code closely mirors the code laid out in record_write.c
  //TODO: graceful premissions
  Chk(estats_record_open(&record, "/tmp/test-record", "w"));
  Chk(estats_read_vars(data, cid, cl));
  Chk(estats_connection_tuple_as_strings(&tuple_ascii, &data->tuple));
  for (i = 0; i < data->length; i++) {
    Chk(estats_val_as_string(&str, &data->val[i], estats_var_array[i].valtype));
  }
  Chk(estats_record_write_data(record, data));
  estats_record_close(&record);
  
Cleanup:
  return err;
}


/*
 * Assign the value of parameter to be the web10g 'LimRwin' variable's value
 */
void web10g_get_LimRwin(uint32_t* uv32) 
{
  if (data_notnull)
    *uv32 = data->val[PERF_INDEX_MAX+PATH_INDEX_MAX+STACK_INDEX_MAX+APP_INDEX_MAX+LIMRWIN].uv32;
  else
    *uv32 = 0;
}


/*
 * Assign the value of parameter to be the web10g 'OctetsRetrans' variable's value
 */
void web10g_get_OctetsRetrans(uint32_t* uv32) {
  if (data_notnull)
    *uv32 = data->val[OCTETSRETRANS].uv32;
  else
    *uv32 = 0;
}


/*
 * Assign the value of parameter to be the web10g 'SampleRTT' variable's value
 */
void web10g_get_SampleRTT(uint32_t* uv32) {
  if (data_notnull)
    *uv32 = data->val[PERF_INDEX_MAX+SAMPLERTT].uv32;
  else
    *uv32 = 0;
}


/*
 * Set the value of the web10g 'LimRwin' variable to the value of the parameter
 */
estats_error* web10g_set_limRwin(uint32_t* val) 
{
  struct estats_error* err = NULL;
  Chk(estats_write_var("LimRwin", val, cid, cl));
  
Cleanup:
  return err;
}


/*
 * Free and destroy web10g variables
 */
void web10g_free() 
{
  estats_connection_list_free(&clist);
  estats_val_data_free(&data);
  estats_nl_client_destroy(&cl);
}
