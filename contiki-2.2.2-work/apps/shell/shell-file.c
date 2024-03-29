/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: shell-file.c,v 1.6 2008/07/06 10:34:44 oliverschmidt Exp $
 */

/**
 * \file
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell-file.h"
#include "cfs/cfs.h"

#include <stdio.h>
#include <string.h>

#define MAX_FILENAME_LEN 40

/*---------------------------------------------------------------------------*/
PROCESS(shell_ls_process, "ls");
SHELL_COMMAND(ls_command,
	      "ls",
	      "ls: list files",
	      &shell_ls_process);
PROCESS(shell_append_process, "append");
SHELL_COMMAND(append_command,
	      "append",
	      "append <filename>: append to file",
	      &shell_append_process);
PROCESS(shell_write_process, "write");
SHELL_COMMAND(write_command,
	      "write",
	      "write <filename>: write to file",
	      &shell_write_process);
PROCESS(shell_read_process, "read");
SHELL_COMMAND(read_command,
	      "read",
	      "read <filename> [offset]: read from file, with an optional offset",
	      &shell_read_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_ls_process, ev, data)
{
  static struct cfs_dir dir;
  static int totsize;
  struct cfs_dirent dirent;
  char buf[32];
  PROCESS_BEGIN();
  
  if(cfs_opendir(&dir, "/") != 0) {
    shell_output_str(&ls_command, "Cannot open directory", "");
  } else {
    totsize = 0;
    while(cfs_readdir(&dir, &dirent) == 0) {
      totsize += dirent.size;
      sprintf(buf, "%3d ", dirent.size);
      /*      printf("'%s'\n", dirent.name);*/
      shell_output_str(&ls_command, buf, dirent.name);
    }
    cfs_closedir(&dir);
    sprintf(buf, "%d", totsize);
    shell_output_str(&ls_command, "Total size: ", buf);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_append_process, ev, data)
{
  static int fd = 0;

  PROCESS_EXITHANDLER(cfs_close(fd));
  
  PROCESS_BEGIN();

  fd = cfs_open(data, CFS_WRITE | CFS_APPEND);

  if(fd < 0) {
    shell_output_str(&append_command,
		     "append: could not open file for writing: ", data);
  } else {
    while(1) {
      struct shell_input *input;
      PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
      input = data;
      /*    printf("cat input %d %d\n", input->len1, input->len2);*/
      if(input->len1 + input->len2 == 0) {
	cfs_close(fd);
	PROCESS_EXIT();
      }
      
      cfs_write(fd, input->data1, input->len1);
      cfs_write(fd, input->data2, input->len2);
      
      shell_output(&append_command,
		   input->data1, input->len1,
		   input->data2, input->len2);
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_write_process, ev, data)
{
  static int fd = 0;

  PROCESS_EXITHANDLER(cfs_close(fd));
  
  PROCESS_BEGIN();

  fd = cfs_open(data, CFS_WRITE);

  if(fd < 0) {
    shell_output_str(&write_command,
		     "write: could not open file for writing: ", data);
  } else {
    while(1) {
      struct shell_input *input;
      PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
      input = data;
      /*    printf("cat input %d %d\n", input->len1, input->len2);*/
      if(input->len1 + input->len2 == 0) {
	cfs_close(fd);
	PROCESS_EXIT();
      }
      
      cfs_write(fd, input->data1, input->len1);
      cfs_write(fd, input->data2, input->len2);
      
      shell_output(&write_command,
		   input->data1, input->len1,
		   input->data2, input->len2);
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_read_process, ev, data)
{
  static int fd = 0;
  char *next;
  char filename[MAX_FILENAME_LEN];
  int len;
  int offset = 0;
  PROCESS_EXITHANDLER(cfs_close(fd));
  PROCESS_BEGIN();

  if(data != NULL) {
    next = strchr(data, ' ');
    if(next == NULL) {
      strncpy(filename, data, sizeof(filename));
    } else {
      len = (int)(next - (char *)data);
      if(len <= 0) {
	shell_output_str(&read_command,
		       "read: filename too short: ", data);
	PROCESS_EXIT();
      }
      if(len > MAX_FILENAME_LEN) {
	shell_output_str(&read_command,
		       "read: filename too long: ", data);
	PROCESS_EXIT();
      }
      memcpy(filename, data, len);
      filename[len] = 0;

      offset = shell_strtolong(next, NULL);
    }
    
    fd = cfs_open(filename, CFS_READ);
    cfs_seek(fd, offset);
    
    if(fd < 0) {
      shell_output_str(&read_command,
		       "read: could not open file for reading: ", filename);
    } else {
      
      while(1) {
	char buf[40];
	int len;
	struct shell_input *input;
	
	len = cfs_read(fd, buf, sizeof(buf));
	if(len <= 0) {
	  cfs_close(fd);
	  PROCESS_EXIT();
	}
	shell_output(&read_command,
		     buf, len, "", 0);
	
	process_post(&shell_read_process, PROCESS_EVENT_CONTINUE, NULL);
	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE ||
				 ev == shell_event_input);
	
	if(ev == shell_event_input) {
	  input = data;
	  /*    printf("cat input %d %d\n", input->len1, input->len2);*/
	  if(input->len1 + input->len2 == 0) {
	    cfs_close(fd);
	    PROCESS_EXIT();
	  }
	}
      }
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_file_init(void)
{
  shell_register_command(&ls_command);
  shell_register_command(&write_command);
  shell_register_command(&append_command);
  shell_register_command(&read_command);
}
/*---------------------------------------------------------------------------*/
