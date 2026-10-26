#include <stdio.h>

#define int MAX_FRAME_SIZE = 16; //max size of frame including sync and length

static void read_data(char fileName[]);
static void processFrame(int8_t sync[], int8_t frame[]);

int main(void) {
    read_data("capture.bin");
}

void read_data(char fileName[]) {
    FILE *f = fopen(fileName, "rb"); //open file in read binary mode

    if (f==NULL) {
        printf("Could not open file %s", fileName);
        return 1;
    }

    int8_t sync[3]; 
    int8_t frame[MAX_FRAME_SIZE - 3];

    while (1) {
        fread(sync, 1, 3, f);
        int8_t length = sync[2];
        fread(frame, 1, length, f);
        processFrame(sync, frame);
    }
    
    fclose(f);
}

static void processFrame(int8_t sync[], int8_t frame[]) {

}
