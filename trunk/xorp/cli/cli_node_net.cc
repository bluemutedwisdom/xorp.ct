// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

// Copyright (c) 2001-2003 International Computer Science Institute
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software")
// to deal in the Software without restriction, subject to the conditions
// listed in the XORP LICENSE file. These conditions include: you must
// preserve this copyright notice, and you cannot mention the copyright
// holders in advertising related to the Software without their permission.
// The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
// notice is a summary of the XORP LICENSE file; the license in that file is
// legally binding.

#ident "$XORP: xorp/cli/cli_node_net.cc,v 1.10 2003/04/02 22:48:37 pavlin Exp $"


//
// CLI network-related functions
//


#include "cli_module.h"
#include "cli_private.hh"
#include <errno.h>
#include <termios.h>
#include <arpa/telnet.h>
#include "libxorp/c_format.hh"
#include "libxorp/token.hh"
#include "libcomm/comm_api.h"
#include "cli_client.hh"


//
// Exported variables
//

//
// Local constants definitions
//

//
// Local structures/classes, typedefs and macros
//


//
// Local variables
//

//
// Local functions prototypes
//


/**
 * CliNode::sock_serv_open:
 * 
 * Open a socket for the CLI to listen on for connections.
 * 
 * Return value: The new socket to listen on success, othewise %XORP_ERROR.
 **/
int
CliNode::sock_serv_open()
{
    // Open the socket
    switch (family()) {
    case AF_INET:
	_cli_socket = comm_bind_tcp4(NULL, _cli_port);
	break;
#ifdef HAVE_IPV6
    case AF_INET6:
	_cli_socket = comm_bind_tcp6(NULL, _cli_port);
	break;
#endif // HAVE_IPV6
    default:
	assert(false);
	return (XORP_ERROR);
    }
    
    if (_cli_socket < 0)
	return (XORP_ERROR);
    
    return (_cli_socket);
}

void
CliNode::accept_connection(int fd, SelectorMask mask)
{
    debug_msg("Received connection on socket = %d, family = %d\n",
	      fd, family());
    
    XLOG_ASSERT(mask == SEL_RD);
    
    int client_socket = comm_sock_accept(fd);
    
    if (client_socket >= 0)
	add_connection(client_socket);
}

CliClient *
CliNode::add_connection(int client_socket)
{
    CliClient *cli_client = NULL;
    
    debug_msg("Added connection on socket = %d, family = %d\n",
	      client_socket, family());
    
    cli_client = new CliClient(*this, client_socket);
    _client_list.push_back(cli_client);
    
    //
    // Set peer address (for network connection only)
    //
    if (! cli_client->is_stdio()) {
	union {
	    struct sockaddr sa;
#ifdef HAVE_IPV6
	    char data[sizeof(struct sockaddr_in6)];
#endif
	} un;
	struct sockaddr *sa = (struct sockaddr *)&un.sa;
	socklen_t len = sizeof(un);
	if (getpeername(cli_client->cli_fd(), sa, &len) < 0) {
	    // Error getting peer address
	    delete_connection(cli_client);
	    return (NULL);
	}
	IPvX peer_addr = IPvX::ZERO(family());
	switch (sa->sa_family) {
	case AF_INET:
	{
	    struct sockaddr_in *s_in = (struct sockaddr_in *)sa;
	    peer_addr.copy_in(*s_in);
	}
	break;
#ifdef HAVE_IPV6
	case AF_INET6:
	{
	    struct sockaddr_in6 *s_in6 = (struct sockaddr_in6 *)sa;
	    peer_addr.copy_in(*s_in6);
	}
	break;
#endif // HAVE_IPV6
	default:
	    // Invalid address family
	    delete_connection(cli_client);
	    return (NULL);
	}
	cli_client->set_cli_session_from_address(peer_addr);
    }
    
    //
    // Check access control for this peer address
    //
    if (! is_allow_cli_access(cli_client->cli_session_from_address())) {
	delete_connection(cli_client);
	return (NULL);
    }
    
    if (cli_client->start_connection() < 0) {
	// Error connecting to the client
	delete_connection(cli_client);
	return (NULL);
    }
    
    //
    // Connection OK
    //
    
    //
    // Set user name
    //
    cli_client->set_cli_session_user_name("guest");	// TODO: get user name
    
    //
    // Set terminal name
    //
    {
	string term_name = "cli_unknown";
	uint32_t i = 0;
	
	for (i = 0; i < CLI_MAX_CONNECTIONS; i++) {
	    term_name = c_format("cli%u", i);
	    if (find_cli_by_term_name(term_name.c_str()) == NULL)
		break;
	}
	if (i >= CLI_MAX_CONNECTIONS) {
	    // Too many connections
	    delete_connection(cli_client);
	    return (NULL);
	}
	cli_client->set_cli_session_term_name(term_name.c_str());
    }
    
    //
    // Set session id
    //
    {
	uint32_t session_id = ~0;	// XXX: ~0 has no particular meaning
	uint32_t i = 0;
	
	for (i = 0; i < CLI_MAX_CONNECTIONS; i++) {
	    session_id = _next_session_id++;
	    if (find_cli_by_session_id(session_id) == NULL)
		break;
	}
	if (i >= CLI_MAX_CONNECTIONS) {
	    // This should not happen: there are available session slots,
	    // but no available session IDs.
	    assert(0 && "Cannot assign CLI session ID");
	    return (NULL);
	}
	cli_client->set_cli_session_session_id(session_id);
    }
    
    //
    // Set session start time
    //
    {
	TimeVal now;
	
	eventloop().current_time(now);
	cli_client->set_cli_session_start_time(now);
    }
    cli_client->set_is_cli_session_active(true);
    
    return (cli_client);
}

int
CliNode::delete_connection(CliClient *cli_client)
{
    debug_msg("Delete connection on socket = %d, family = %d\n",
	      cli_client->cli_fd(), family());
    cli_client->cli_flush();
    
    // The callback when deleting this client
    if (! _cli_client_delete_callback.is_empty())
	_cli_client_delete_callback->dispatch(cli_client);

    _client_list.remove(cli_client);
    delete cli_client;
    
    return (XORP_OK);
}

int
CliClient::start_connection(void)
{
    if (cli_node().eventloop().add_selector(cli_fd(), SEL_RD,
					     callback(this, &CliClient::client_read))
	== false)
	return (XORP_ERROR);
    
    //
    // Setup the telnet options
    //
    if (! is_stdio()) {
	char will_echo_cmd[] = { IAC, WILL, TELOPT_ECHO, '\0' };
	char will_sga_cmd[]  = { IAC, WILL, TELOPT_SGA, '\0'  };
	char dont_linemode_cmd[] = { IAC, DONT, TELOPT_LINEMODE, '\0' };
	char do_window_size_cmd[] = { IAC, DO, TELOPT_NAWS, '\0' };
	char do_transmit_binary_cmd[] = { IAC, DO, TELOPT_BINARY, '\0' };
	char will_transmit_binary_cmd[] = { IAC, WILL, TELOPT_BINARY, '\0' };
	
	write(cli_fd(), will_echo_cmd, sizeof(will_echo_cmd));
	write(cli_fd(), will_sga_cmd, sizeof(will_sga_cmd));
	write(cli_fd(), dont_linemode_cmd, sizeof(dont_linemode_cmd));
	write(cli_fd(), do_window_size_cmd, sizeof(do_window_size_cmd));
	write(cli_fd(), do_transmit_binary_cmd, sizeof(do_transmit_binary_cmd));
	write(cli_fd(), will_transmit_binary_cmd, sizeof(will_transmit_binary_cmd));
    }
    
    //
    // Put the terminal in non-canonical and non-echo mode
    //
    if (is_stdio()) {
	struct termios termios;
	
	while (tcgetattr(cli_fd(), &termios) != 0) {
	    if (errno != EINTR) {
		XLOG_ERROR("start_connection(): tcgetattr() error: %s", 
			   strerror(errno));
		return (XORP_ERROR);
	    }
	}
	termios.c_lflag &= ~(ICANON | ECHO);
	while (tcsetattr(cli_fd(), TCSADRAIN, &termios) != 0) {
	    if (errno != EINTR) {
		XLOG_ERROR("start_connection(): tcsetattr() error: %s", 
			   strerror(errno));
		return (XORP_ERROR);
	    }
	}
    }
    
    //
    // Setup the read/write file descriptors
    //
    if (! is_stdio()) {
	// Network connection
	_cli_fd_file_read = fdopen(cli_fd(), "r");
	_cli_fd_file_write = fdopen(cli_fd(), "w");
    } else {
	// Stdin connection
	_cli_fd_file_read = stdin;
	_cli_fd_file_write = stdout;
    }
    
    _gl = new_GetLine(1024, 2048);		// TODO: hardcoded numbers
    if (_gl == NULL)
	return (XORP_ERROR);
    
    // XXX: always set to network type
    gl_set_is_net(_gl, 1);
    
    // Set the terminal
    string term_name = "vt100";		// Default for network connection
    if (is_stdio()) {
	term_name = getenv("TERM");
	if (term_name.empty())
	    term_name = "vt100";	// Set to default
    }
    // Change the input and output streams for libtecla
    if (gl_change_terminal(_gl, _cli_fd_file_read, _cli_fd_file_write,
			   term_name.c_str())
	!= 0) {
	_gl = del_GetLine(_gl);
	return (XORP_ERROR);
    }
    
    // Add the command completion hook
    if (gl_customize_completion(_gl, this, command_completion_func) != 0) {
	_gl = del_GetLine(_gl);
	return (XORP_ERROR);
    }
    
    // Print the welcome message and show the prompt
    cli_print(c_format("%s\n%s", XORP_CLI_WELCOME, current_cli_prompt()));
    
    return (XORP_OK);
}


//
// If @v is true, block the client terminal, otherwise unblock it.
//
void
CliClient::set_is_waiting_for_data(bool v)
{
    _is_waiting_for_data = v;
    // block_connection(v);
}

//
// If @block_bool is true, block the connection (by not select()-ing
// on its socket), otherwise add its socket back to the pool of select()-ed
// sockets.
//
// Return: %XORP_OK on success, otherwise %XORP_ERROR.
int
CliClient::block_connection(bool block_bool)
{
    if (cli_fd() < 0)
	return (XORP_ERROR);
    
    if (block_bool) {
	// Un-select()
	cli_node().eventloop().remove_selector(cli_fd(), SEL_ALL);
	return (XORP_OK);
    }

    // Do-select()
    if (cli_node().eventloop().add_selector(cli_fd(), SEL_RD,
					     callback(this, &CliClient::client_read))
	== false)
	return (XORP_ERROR);

    return (XORP_OK);
}

void
CliClient::client_read(int fd, SelectorMask mask)
{
    char *line = NULL;
    char buf[1024];		// TODO: 1024 size must be #define
    int i, n, ret_value;
    
    XLOG_ASSERT(mask == SEL_RD);
    
    n = read(fd, buf, sizeof(buf));
    
    debug_msg("client_read %d octet(s)\n", n);
    if (n <= 0) {
	cli_node().delete_connection(this);
	return;
    }
    
    // Scan the input data and filter-out the Telnet commands
    for (i = 0; i < n; i++) {
	uint8_t val = buf[i];
	
	if (! is_stdio()) {
	    int ret = process_telnet_option(val);
	    if (ret < 0) {
		// Kick-out the client
		// TODO: print more informative message about the client:
		// E.g. where it came from, etc.
		XLOG_WARNING("Removing client (socket = %d family = %d): "
			     "error processing telnet option",
			     fd,
			     cli_node().family());
		cli_node().delete_connection(this);
		return;
	    }
	    if (ret == 0) {
		// Telnet option processed
		continue;
	    }
	}
	
	if (is_waiting_for_data()) {
	    // Waiting for data for previous command.
	    if (val == CHAR_TO_CTRL('c')) {
		// Cancel the command
		set_pipe_mode(false);	// XXX: disable piping
		delete_pipe_all();
		cli_flush();
		cli_print(c_format("Command interrupted!\n"));
		set_is_waiting_for_data(false);
		waiting_for_result_timer().unschedule();
		post_process_command(false);
	    } else {
		continue;
	    }
	}
	
	preprocess_char(val);
	
	//
	// Get a character and process it
	//
	do {
	    line = gl_get_line_net(gl(),
				   current_cli_prompt(),
				   (char *)command_buffer().data(),
				   buff_curpos(),
				   val);
	    ret_value = XORP_ERROR;
	    if (line == NULL) {
		ret_value = XORP_ERROR;
		break;
	    }
	    if (is_page_mode()) {
		ret_value = process_char_page_mode(val);
		break;
	    }
	    ret_value = process_char(line, val);
	    break;
	} while (false);
	
	if (ret_value != XORP_OK) {
	    // Either error or end of input
	    cli_print("\nEnd of connection.\n");
	    cli_node().delete_connection(this);
	    return;
	}
    }
    cli_flush();		// Flush-out the output
}

//
// Preprocess a character before 'libtecla' get its hand on it
//
int
CliClient::preprocess_char(uint8_t val)
{
    //
    // XXX: Bind/unbind the 'SPACE' to complete-word so it can
    // complete a half-written command.
    // TODO: if the beginning of the line, shall we explicitly unbind as well?
    //
    if (val == ' ') {
	int tmp_buff_curpos = buff_curpos();
	char *tmp_line = (char *)command_buffer().data();
	string token_line = string(tmp_line, tmp_buff_curpos);
	string token = pop_token(token_line);
	if (token.empty()
	    || (multi_command_find(tmp_line, tmp_buff_curpos) != NULL)) {
	    // Un-bind the "SPACE" to complete-word
	    // Don't ask why we need six '\' to specify the ASCII value
	    // of 'SPACE'...
	    gl_configure_getline(gl(),
				 "bind \\\\\\040 ",
				 NULL, NULL);
	} else {
	    // Bind the "SPACE" to complete-word
	    gl_configure_getline(gl(),
				 "bind \\\\\\040   complete-word",
				 NULL, NULL);
	}
    }
    
    return (XORP_OK);
}

//
// Process octet that may be part of a telnet option.
// Parameter 'val' is the value of the next octet.
// Return -1 if the caller should remove this client.
// Return 0 if @val is part of a telnet option, and the telnet option was
// processed.
// Return 1 if @val is not part of a telnet option and should be processed
// as input data.
//
int
CliClient::process_telnet_option(int val)
{
    if (val == IAC) {
	// Probably a telnet command
	if (! telnet_iac()) {
	    set_telnet_iac(true);
	    return (0);
	}
	set_telnet_iac(false);
    }
    if (telnet_iac()) {
	switch (val) {
	case SB:
	    // Begin subnegotiation of the indicated option.
	    telnet_sb_buffer().reset();
	    set_telnet_sb(true);
	    break;
	case SE:
	    // End subnegotiation of the indicated option.
	    if (! telnet_sb())
		break;
	    switch(telnet_sb_buffer().data(0)) {
	    case TELOPT_NAWS:
		// Telnet Window Size Option
		if (telnet_sb_buffer().data_size() < 5)
		    break;
		{
		    uint16_t new_window_width, new_window_height;
		    
		    new_window_width   = 256*telnet_sb_buffer().data(1);
		    new_window_width  += telnet_sb_buffer().data(2);
		    new_window_height  = 256*telnet_sb_buffer().data(3);
		    new_window_height += telnet_sb_buffer().data(4);
		    set_window_width(new_window_width);
		    set_window_height(new_window_height);
		    debug_msg("Client window size changed to width = %u "
			      "height = %u\n",
			      (uint32_t)window_width(),
			      (uint32_t)window_height());
		}
		break;
	    default:
		break;
	    }
	    telnet_sb_buffer().reset();
	    set_telnet_sb(false);
	    break;
	case DONT:
	    // you are not to use option
	    set_telnet_dont(true);
	    break;
	case DO:
	    // please, you use option
	    set_telnet_do(true);
	    break;
	case WONT:
	    // I won't use option
	    set_telnet_wont(true);
	    break;
	case WILL:
	    // I will use option
	    set_telnet_will(true);
	    break;
	case GA:
	    // you may reverse the line
	    break;
	case EL:
	    // erase the current line
	    break;
	case EC:
	    // erase the current character
	    break;
	case AYT:
	    // are you there
	    break;
	case AO:
	    // abort output--but let prog finish
	    break;
	case IP:
	    // interrupt process--permanently
	    break;
	case BREAK:
	    // break
	    break;
	case DM:
	    // data mark--for connect. cleaning
	    break;
	case NOP:
	    // nop
	    break;
	case EOR:
	    // end of record (transparent mode)
	    break;
	case ABORT:
	    // Abort process
	    break;
	case SUSP:
	    // Suspend process
	    break;
	case xEOF:
	    // End of file: EOF is already used...
	    break;
	case TELOPT_BINARY:
	    if (telnet_do())
		set_telnet_binary(true);
	    else
		set_telnet_binary(false);
	    break;
	default:
	    break;
	}
	set_telnet_iac(false);
	return (0);
    }
    //
    // Cleanup the telnet options state
    //
    if (telnet_sb()) {
	// A negotiated option value
	if (telnet_sb_buffer().add_data(val) < 0) {
	    // This client is sending too much options. Kick it out!
	    return (XORP_ERROR);
	}
	return (0);
    }
    if (telnet_dont()) {
	// Telnet DONT option code
	set_telnet_dont(false);
	return (0);
    }
    if (telnet_do()) {
	// Telnet DO option code
	set_telnet_do(false);
	return (0);
    }
    if (telnet_wont()) {
	// Telnet WONT option code
	set_telnet_wont(false);
	return (0);
    }
    if (telnet_will()) {
	// Telnet WILL option code
	set_telnet_will(false);
	return (0);
    }
    
    return (1);
}
