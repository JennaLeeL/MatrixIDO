#include <stdio.h>
#include <pigpio.h>

// Adresses 
#define ADR_I2C 0x70
#define ADR_SYS_MATRICE 0x20
#define ADR_AFFICHAGE_MATRICE 0x80

int main() {
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

    // Initialiser la matrice
    i2cWriteByteData(handle, ADR_SYS_MATRICE | 1, 0x00);
    i2cWriteByteData(handle, ADR_AFFICHAGE_MATRICE | 1, 0x00);

    /*
    Chaque rangée de la matrice 8x8 correspond à une adresse. 
    Chaque octet dans ces adresses correspond à une LED.
    Donc si on écrit '00011000' ('24' en décimal ou  0x18)
    à la première adresse (0x00), les 2 LED du milieu de la 
    première rangée s'allument.
    voir https://otardi.gitlab.io/420-314/i2c/mat8x8/#exemple-1
    */



    // // Allumer puis éteindre la 1ère rangée (0)
    // i2cWriteByteData(handle, 0x00, 0xFF);
    // time_sleep(1);
    // i2cWriteByteData(handle, 0x00, 0x00);

    // // Allumer puis éteindre la dernière rangée (14 ou 0e)
    // i2cWriteByteData(handle, 0x0e, 0xFF);
    // time_sleep(1);
    // i2cWriteByteData(handle, 0x0e, 0x00);

    // // Allumer puis éteindre les 4 LED des coins (10000001 = 128 = 0x81)
    // i2cWriteByteData(handle, 0x00, 0x81);
    // i2cWriteByteData(handle, 0x0e, 0x81);
    // time_sleep(1);
    // i2cWriteByteData(handle, 0x00, 0x00);
    // i2cWriteByteData(handle, 0x0e, 0x00);
    // time_sleep(1);

    // 
    i2cWriteByteData(handle, 0x00, 128);
        time_sleep(1);
    i2cWriteByteData(handle, 0x00, 64);
        time_sleep(1);
    i2cWriteByteData(handle, 0x00, 32);
        time_sleep(1);
    i2cWriteByteData(handle, 0x00, 16);
        time_sleep(1);  
    i2cWriteByteData(handle, 0x00, 8);
        time_sleep(1);  
    i2cWriteByteData(handle, 0x00, 4);
        time_sleep(1);  
    i2cWriteByteData(handle, 0x00, 2);
        time_sleep(1);
    i2cWriteByteData(handle, 0x00, 1);

        time_sleep(1);
    i2cWriteByteData(handle, 0x0e, 0x00);   

        // Tout éteindre
    for (int i=0;i<0x0E;i+=2) {
        i2cWriteByteData(handle, i, 0x00);
    }      


    // // Fermer et terminer
    i2cClose(handle);
    gpioTerminate();

    return 0;
}
