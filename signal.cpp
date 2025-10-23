#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

enum Color
{
    VERMELHO,
    AMARELO,
    VERDE
};

Color currentColor = VERMELHO;
pid_t childPid;
bool isParent = true;
Color nextColor = VERDE;

void sigint_handler(int sig)
{
    if (!isParent)
    {
        currentColor = VERMELHO;
        cout << "\n[EMERGÊNCIA] Mudando para VERMELHO por Ctrl+C" << endl;
    }
    else
    {
        kill(childPid, SIGUSR1);
        nextColor = VERDE;
        alarm(3);
    }
}

void sigtstp_handler(int sig)
{
    cout << "\n[SISTEMA] Finalizando semáforo..." << endl;
    if (isParent)
    {
        kill(childPid, SIGKILL);
        wait(NULL);
    }
    exit(0);
}

void sigusr1_handler(int sig)
{
    if (!isParent)
    {
        currentColor = VERMELHO;
    }
}

void sigusr2_handler(int sig)
{
    if (!isParent)
    {
        currentColor = AMARELO;
    }
}

void sigalrm_handler(int sig)
{
    if (isParent)
    {
        switch (nextColor)
        {
        case VERMELHO:
            kill(childPid, SIGUSR1);
            nextColor = VERDE;
            alarm(3);
            break;
        case VERDE:
            kill(childPid, SIGALRM);
            nextColor = AMARELO;
            alarm(4);
            break;
        case AMARELO:
            kill(childPid, SIGUSR2);
            nextColor = VERMELHO;
            alarm(2);
            break;
        }
    }
    else
    {
        currentColor = VERDE;
    }
}

void processoFilho()
{
    isParent = false;

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGALRM, sigalrm_handler);

    cout << "[FILHO] Processo filho iniciado (PID: " << getpid() << ")" << endl;

    while (true)
    {
        switch (currentColor)
        {
        case VERMELHO:
            cout << "\033[2K\033[31mVERMELHO\033[0m" << endl;
            break;
        case AMARELO:
            cout << "\033[2K\033[33mAMARELO\033[0m" << endl;
            break;
        case VERDE:
            cout << "\033[2K\033[32mVERDE\033[0m" << endl;
            break;
        }
        sleep(1);
    }
}

void processoPai(pid_t pid)
{
    isParent = true;
    childPid = pid;

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGALRM, sigalrm_handler);

    cout << "[PAI] Processo pai iniciado (PID: " << getpid() << ")" << endl;
    cout << "[PAI] Controlando processo filho (PID: " << pid << ")" << endl;
    cout << "\nControles:" << endl;
    cout << "  Ctrl+C: Muda para VERMELHO (emergência)" << endl;
    cout << "  Ctrl+Z: Finaliza o programa" << endl;
    cout << "\nTemporização: VERMELHO (3s) → VERDE (4s) → AMARELO (2s)\n"
         << endl;

    kill(pid, SIGUSR1);
    nextColor = VERDE;
    alarm(3);

    while (true)
    {
        pause();
    }
}

int main()
{
    cout << "========================================" << endl;
    cout << "   SIMULADOR DE SEMÁFORO DE TRÂNSITO   " << endl;
    cout << "========================================" << endl;

    pid_t pid = fork();

    if (pid < 0)
    {
        cerr << "Erro ao criar processo filho!" << endl;
        return 1;
    }
    else if (pid == 0)
    {
        processoFilho();
    }
    else
    {
        processoPai(pid);
    }

    return 0;
}
