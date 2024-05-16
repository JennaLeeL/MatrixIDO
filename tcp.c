#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>

// Adresses 
#define ADR_I2C 0x70
#define ADR_SYS_MATRICE 0x20
#define ADR_AFFICHAGE_MATRICE 0x80

// TCP 
#define PORT 9991
//#define DEST_IP "10.10.0.236"
#define BUFFER_SIZE 1024

#define BUTTON_PIN 17 

void buttonPressed(int gpio, int level, uint32_t tick, int socket_dist) {
    char buffer[2] = {0};
    if (level == 0) {
        buffer[0] = '0'; 
    } else {
        buffer[0] = '1'; 
    }
    send(socket_dist, buffer, 1, 0);
}


int main() {

    int socket_local, socket_dist;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    int handle;

    // Initialiser pigpio
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

    int globalNum = 0x00;

    // Set the button pin as input
    gpioSetMode(BUTTON_PIN, PI_INPUT);

    gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);


    while(1) {
        int datalen;

        // Stocker le message
        datalen = read(socket_dist, buffer, BUFFER_SIZE);   
        printf("Reçu: %s\n", buffer);

        char *message2;
        char *message1 = strtok ( buffer, ":");
        printf("Reçu: %s\n", message1);
        message2 = strtok (NULL, ":");
        printf("Reçu: %s\n", message2);
        

        int number = atoi(buffer);
        int number1 = pow(2, number-1);

        if (strcmp(message2, "0") == 0 ) {
            if ((number >> atoi(message1)) & 1) {
            printf("Already on");
            }
            else {
            printf("Now on");
            globalNum  ^= number1;
            i2cWriteByteData(handle, 0x00, globalNum); 
            }
        }
        else {

        }

        // globalNum  ^= number1;a
        // printf("Received number: %d\n", number);
        // i2cWriteByteData(handle, 0x00, globalNum); 
        fflush(stdout); 

    }

    close(socket_dist);  
    close(socket_local);

    i2cClose(handle);

    gpioTerminate();
    return 0;
}
