#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <math.h>

// gcc -o tcp tcp.c -lpigpio -lm  -lpthread

// Addresses
#define ADR_I2C 0x70
#define ADR_SYS_MATRICE 0x20
#define ADR_AFFICHAGE_MATRICE 0x80

// TCP
#define PORT 9991
#define BUFFER_SIZE 1024

#define BUTTON_PIN 17

int handle;
int globalNum = 0x00;
int socket_dist;

void buttonPressed(int gpio, int level, uint32_t tick, void *user_data) {
    char buffer[2] = {0};
    if (level == 0) {
        buffer[0] = '0';
    } else {
        buffer[0] = '1';
    }
    send(socket_dist, buffer, 1, 0);
    printf("Boutton : %c\n", buffer[0]);
}

void *buttonThread(void *arg) {
    // https://abyz.me.uk/rpi/pigpio/cif.html
    
    gpioSetAlertFuncEx(BUTTON_PIN, buttonPressed, NULL);
    while (1) {
        sleep(1); 
    }
    return NULL;
}

int main() {
    int socket_local;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t btnThread;
    char buffer[BUFFER_SIZE] = {0};

    // Initialize pigpio
    if (gpioInitialise() < 0) {
        printf("Erreur d'initialisation pigpio\n");
        return 1;
    }

    // Récupérer le référence ("handle") de la matrice
    handle = i2cOpen(1, ADR_I2C, 0);
    
    if (handle < 0) {
        printf("Erreur d'accès à la matrice LED\n");
        gpioTerminate();
        return 1;
    }

    // Créer le socket et initialiser l'adresse
    socket_local = socket(AF_INET, SOCK_STREAM, 0);
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Associer le socket à l'adresse de l'interface
    bind(socket_local, (struct sockaddr *)&address, sizeof(address));

    // Attendre une connexion entrante
    listen(socket_local, 3);
    socket_dist = accept(socket_local, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    // Initialiser la matrice
    i2cWriteByteData(handle, ADR_SYS_MATRICE | 1, 0x00);
    i2cWriteByteData(handle, ADR_AFFICHAGE_MATRICE | 1, 0x00);

    // Bouton PIN
    gpioSetMode(BUTTON_PIN, PI_INPUT);
    gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);

    // Thread
    pthread_create(&btnThread, NULL, buttonThread, NULL);


    while (1) {
        int datalen = read(socket_dist, buffer, BUFFER_SIZE);
        if (datalen <= 0) {
            printf("Connection OFF\n");
            break;
        }

        printf("Reçu: %s\n", buffer);

        char *message2;
        char *message1 = strtok(buffer, ":");
        message2 = strtok(NULL, ":");

        int number = atoi(message1);
        int number1 = pow(2, number - 1);

        if (strcmp(message2, "0\n") == 0) {
            if ((globalNum >> (number - 1)) & 1) {
                printf("Already on\n");
            } else {
                globalNum ^= number1;
                i2cWriteByteData(handle, 0x00, globalNum);
            }
        } else {
            if ((globalNum >> (number -1)) & 1 ) {
                globalNum ^= number1;
                i2cWriteByteData(handle, 0x00, globalNum);
            }
            else {
                printf("Already off\n");
            }
        }

        fflush(stdout);
    }

    // Tout fermer
    close(socket_dist);
    close(socket_local);
    i2cClose(handle);
    gpioTerminate();
    
    return 0;
}
