#include <ncurses.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <netdb.h> 
#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "message_enc.h"

#define MAX_CH 80
#define PORT 8080
#define PID_SIZE 7
#define DELAY 35000
#define MIDDLE 9
#define BUFFER_SIZE 1024

#ifndef MAX
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#endif

#define USE_BANNER 0

/* OSX3c = OSlab eXercise 3 chat */
char *banner = 
"  ___  ______  _______           \n"     
" / _ \\/ ___\\ \\/ /___ /  ___   \n" 
"| | | \\___ \\\\  /  |_ \\ / __| \n"
"| |_| |___) /  \\ ___) | (__     \n"
" \\___/|____/_/\\_\\____/ \\___| \n";

int msg_stack_top = USE_BANNER*6;
bool window_full = 0;

void connect_to_server(int* sockfd, char* ip) {
	pid_t client_pid;
	int pid_str_size;
	char * pid_str;
	int recv;
	char rcv[BUFFER_SIZE];
	struct sockaddr_in servaddr;

	/* Get the PID and convert it to a string
	 * to be sent to the server and be used as
	 * an identifier */
	client_pid = getpid();
	pid_str = calloc(PID_SIZE, sizeof(char));
	pid_str_size = sprintf(pid_str, "%d", client_pid);

	/* Create the needed socket */
	*sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (*sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0);
	} 
	else {
		printf("Socket successfully created..\n"); 
	}

	/* We set a time limit for the read operations in case the pid
	 authentication fails */
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	if( setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,
				sizeof tv) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Assign the server arguments to the struct
	 * used as the parameter of connect */	
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	(servaddr.sin_addr).s_addr = inet_addr(ip); 
	servaddr.sin_port = htons(PORT);

	/* Attemp to connect to the requested server */
	if (connect(*sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	}
	else {
		printf("connected to the server..\n");
	}

	/* The server will be waiting for the client
	 * to send his PID */
	send(*sockfd, pid_str, pid_str_size, 0);

	recv = read(*sockfd, rcv, BUFFER_SIZE);

	if (strncmp(rcv, "ACK", 3)) {
		printf("Registration must have failed\n");
		exit(EXIT_FAILURE);
	}

	free(pid_str);
}

WINDOW* initialize_window (int* max_x, int* max_y) {
	WINDOW* WIN;
	initscr();
	setlocale(LC_ALL, "");
	use_default_colors();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	start_color();
	init_pair(1, -1, -1);
	init_pair(2, COLOR_RED, -1);
	init_pair(3, COLOR_CYAN, -1);

	getmaxyx(stdscr, *max_y, *max_x);
	WIN = newwin(*max_y - 3, *max_x - 3, 1, 2);
	scrollok(WIN, TRUE);

	border('|', '|', '-', '-', '+', '+', '+', '+');

	/* Print Banner */
	if (USE_BANNER) {
		wprintw(WIN, "%s", banner);
	}

	mvprintw(*max_y - 2, 2, "> ");

	refresh();
	wrefresh(WIN);

	return WIN;
}

void place_in_window (WINDOW* WIN, char buffer[], int buff_lim, int max_y) {
	int i = 0;
	if (msg_stack_top == max_y - 4 && window_full)
		scroll(WIN);

	int extracted_pid = strtol(buffer + 1, NULL, 10);
	bool is_same_pid = (extracted_pid == getpid());

	if (buffer[0] == '\0') {
		wmove(WIN, msg_stack_top, 0);
		wprintw(WIN, "[");

		wattron(WIN, COLOR_PAIR(3 - is_same_pid));

		i = 0;
		for (i = 1; i <= AES_BLOCK_SIZE; i++) {
			if (buffer[i] == 0x00) break;
			wprintw(WIN, "%c", buffer[i]);
		}

		wattron(WIN, COLOR_PAIR(1));
		wprintw(WIN, "]");

		wmove(WIN, msg_stack_top, MIDDLE);

		for (i = AES_BLOCK_SIZE; i < buff_lim; i++)
			if (buffer[i] != 0x00)
				wprintw(WIN, "%c", buffer[i]);

	} else {
		msg_stack_top--;
		for (i = 0; i < buff_lim; i++)
			if (buffer[i] != 0x00)
				wprintw(WIN, "%c", buffer[i]);
	}


	if (msg_stack_top != max_y - 4)
		msg_stack_top++;
	else
		window_full = true;
	wrefresh(WIN);
}

void reset_input_prompt(int* buff_ptr, int max_y, int max_x, int* x) {
	*buff_ptr = 0;

	move(max_y - 2, 0);
	clrtoeol();
	mvprintw(max_y - 2, 0, "|");
	mvprintw(max_y - 2, max_x - 1, "|");
	*x = 2;
	mvprintw(max_y - 2, *x, ">");
	move(max_y - 2, ++(*x) + 1);
	refresh();
}

int main(int argc, char *argv[]) {
	int x, y, max_x, max_y, ch, i, err;
	int msg_stack_top = 0, buff_ptr = 0;
	int out_of_lim = 0;
	char buff[BUFFER_SIZE], rcv[BUFFER_SIZE];
	int recv;
	bool exit_pending = false, server_up = true;
	WINDOW* WIN;
	/* Variables needed for the connection to
	 * the server */
	int sockfd; 
	struct sockaddr_in servaddr;

	int cfd = -1;

	if (argc == 1) {
		printf("Please specify the server address\n");
		exit(0);
	}

	#if (USE_ENC == 1)
	/* Open the crypto device */
	cfd = open("/dev/crypto", O_RDWR, 0);

	if (cfd < 0) {
		err = errno;
		if (err == ENOENT)
			printf("The cryptodev module is not installed\n");
		else
			printf("file open failed: error code %d: %s\n", err, strerror(err));

		exit(0);
	}
	#endif

	/* Connect to the server */
	connect_to_server(&sockfd, argv[1]);
	/* Initialize everything related to ncurses */	
	WIN = initialize_window(&max_x, &max_y);
	reset_input_prompt(&buff_ptr, max_y, max_x, &x);

	y = max_y - 2;
	x = 3;

	fd_set fds;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;

	int max_fd, status;

	max_fd = MAX(sockfd, STDIN_FILENO);

	while(!exit_pending && server_up) {
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		FD_SET(sockfd, &fds);

		status = select(max_fd + 1, &fds, NULL, NULL, &tv);

		if (FD_ISSET(STDIN_FILENO, &fds)) {
			switch ((ch = getch())) {
			case KEY_F(1):
				exit_pending = true;
				break;
			case '\n':
				#if (USE_ENC == 1)
				encrypt_decrypt_message(ENCRYPT, buff, &buff_ptr, cfd);
				#endif
				send(sockfd, buff, buff_ptr, 0);
				reset_input_prompt(&buff_ptr, max_y, max_x, &x);
				break;
			case KEY_BACKSPACE:
				if (buff_ptr == 0)
					break;
				move(y, x--);
				printw(" ");
				buff_ptr--;
				move(y,x+1);
				refresh();
				break;
			default:
				if (!isprint(ch) || buff_ptr == MAX_CH)
					break;
				buff[buff_ptr++] = ch;
				move(y,++x);
				printw("%c", ch);
				refresh();
				break;
			}
		}

		if (FD_ISSET(sockfd, &fds)) {
			recv = read(sockfd, rcv, BUFFER_SIZE);
			if (recv == 0) {
				server_up = false;
				break;
			}
			#if (USE_ENC == 1)
			encrypt_decrypt_message(DECRYPT, rcv, &recv, cfd);
			#endif
			place_in_window (WIN, rcv, recv, max_y);
			/* Restore the cursor */
			move(y, x + 1);
			refresh();
		}
		usleep(DELAY);
		
	}

	endwin();

	if (!server_up)
		printf("Something is wrong with the server...\n");

	close(sockfd);
	close(cfd);
	return 0;
}
