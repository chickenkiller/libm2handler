/* 
 * File:   m2daemon.c
 * Author: Xavier Lange <xrlange@gmail.com>
 *
 * Created on May 23, 2011, 11:32 AM
 *
 * Description: Large parts copied from mongrel2's
 * src/unixy.c. Not using all the functions
 * because a C handler should never need
 * to drop privileges.
 *
 */

#include <m2handler.h>
#include <unistd.h>

int mongrel2_pid_file(bstring path){
    return 0;
}

int mongrel2_daemonize(){
    int rc = daemon(0, 1);
    // TODO: Add check throughout code!
    // check(rc == 0, "Failed to daemonize");
    return rc;
}