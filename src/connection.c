/*
** EPITECH PROJECT, 2024
** Connection
** File description:
** Handle the connection for the navy
*/

#include <signal.h>
#include <unistd.h>
#include <ucontext.h>
#include "player.h"
#include "my.h"
#include "my_macros.h"

void display_pid(player_t *player)
{
    pid_t pid = getpid();

    my_putstr("my_pid: ");
    my_put_nbr(pid);
    my_putchar('\n');
    if (player->id == 1)
        my_putstr("waiting for enemy connection...\n");
}

static
int get_info(int increment, siginfo_t *info, int signal)
{
    static int last_signal = NO_SIGNAL;
    static pid_t last_pid_other = 0;

    if (increment == GET_SIGNAL)
        return last_signal;
    if (increment == GET_PID)
        return (int)last_pid_other;
    if (increment == ADD) {
        last_signal = signal;
        if (info == NULL)
            return FAILURE;
        last_pid_other = info->si_pid;
    }
    return last_signal;
}

static
void handle_signal(int signal, siginfo_t *info, UNUSED void *context)
{
    get_info(ADD, info, signal);
}

int init_sigaction(struct sigaction *sig_action,
    void (*handler)(int, siginfo_t *, void *))
{
    if (sig_action == NULL)
        return display_error("Unable to use sigaction\n");
    sig_action->sa_flags = SA_SIGINFO;
    sig_action->sa_sigaction = handler;
    sigemptyset(&sig_action->sa_mask);
    return SUCCESS;
}

static
int wait_connection(player_t *player)
{
    struct sigaction sig_action;
    int received_signal = NO_SIGNAL;

    if (player == NULL)
        return display_error("Wrong player entered\n");
    if (init_sigaction(&sig_action, &handle_signal) == FAILURE)
        return FAILURE;
    if (sigaction(SIGUSR1, &sig_action, NULL) == -1)
        return display_error("Invalid use of sigaction\n");
    while (received_signal != SIGUSR1) {
        pause();
        received_signal = get_info(GET_SIGNAL, NULL, NO_SIGNAL);
    }
    player->enemy_pid = get_info(GET_PID, NULL, NO_SIGNAL);
    if (kill(player->enemy_pid, player->signal_send) == -1)
        return FAILURE;
    my_putstr("enemy connected\n\n");
    return SUCCESS;
}

static
int request_connection(char const *pid_str)
{
    struct sigaction sig_action;
    int received_signal = NO_SIGNAL;
    int pid_to_ping = 0;

    if (pid_str == NULL || my_str_isnum(pid_str) == FALSE)
        return display_error("Wrong value of pid\n");
    pid_to_ping = my_getnbr(pid_str);
    if (pid_to_ping < 0 ||
        init_sigaction(&sig_action, handle_signal) == FAILURE)
        return FAILURE;
    if (sigaction(SIGUSR1, &sig_action, NULL) == -1)
        return display_error("Invalid use of sigaction\n");
    usleep(1000);
    if (kill(pid_to_ping, SIGUSR1) == -1)
        return display_error("Wrong value of PID entered\n");
    while (received_signal != SIGUSR1) {
        pause();
        received_signal = get_info(GET_SIGNAL, NULL, NO_SIGNAL);
    }
    return my_putstr("successfully connected\n\n");
}

int connect_player(player_t *player, int argc, char **argv)
{
    if (argv == NULL || argc < 2 || argv[1] == NULL || player == NULL)
        return FAILURE;
    if (PLAYER1) {
        if (wait_connection(player) == FAILURE)
            return FAILURE;
    }
    if (PLAYER2) {
        if (request_connection(argv[1]) == FAILURE)
            return FAILURE;
    }
    return SUCCESS;
}
