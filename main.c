#include <stdio.h>
#include <stdint.h>

#define MAX_FRAME_SIZE 17 //max size of frame including sync and length
#define MIN_FRAME_SIZE 16
#define MAX_TIME 65535 //point where time wraps

struct weight {
    uint8_t mask;
    float currWeight;
    int16_t offset;
};

static void read_data(char fileName[], char outfileName[]);
float getWeight(int16_t rawData, int16_t offset);
int16_t combineBytes(uint8_t byte1, uint8_t byte2);
void updateWeight(uint8_t faults, int16_t rawWeight, struct weight *w);
uint8_t checkSum(uint8_t length, uint8_t frame[]);

int main(void) {
    remove("dataOut.csv");
    read_data("capture.bin", "dataOut.csv");
}

void read_data(char fileName[], char outfileName[]) {
    FILE *f = fopen(fileName, "rb"); //open file in read binary mode
    FILE *fo = fopen(outfileName, "w+"); //csv output file

    if (f==NULL) {
        printf("Could not open file %s", fileName);
        return;
    }
    if (fo == NULL) {
        printf("Could not open file %s", outfileName);
        return;
    }

    uint8_t sync[3]; 
    uint8_t frame[MAX_FRAME_SIZE - 3];

    struct weight weight1 = {0b00000001, 0, 0};
    struct weight weight2 = {0b00000010, 0, 0};
    struct weight weight3 = {0b00000100, 0, 0};
    struct weight weight4 = {0b00001000, 0, 0};

    uint8_t sequence;

    uint16_t timeMs = 0;
    uint16_t initialTime = 0;
    uint8_t initialTimeSet = 0;
    float timeSec = 0;
    uint16_t timeMultiple = 0;

    int8_t faults;

    int32_t total_kg;

    fprintf(fo,"t_s, total_kg\n");

    uint16_t frameCount = 0;
    uint16_t corruptFrameCount = 0;
    uint16_t droppedFrames = 0;

    uint8_t endOfFile = 0;

    while (endOfFile != 1) {
        int syncRead = fread(sync, 1, 3, f); //fread(where to, bytes/element, num elements, stream)

        uint8_t length = sync[2];

        int frameRead = fread(frame, 1, length+1, f); //read in rest of the frame, +1 because of checksum

        if (initialTimeSet == 0) {
            initialTime = frame[3] << 8 | frame[2];
            initialTimeSet = 1;
        }

        uint8_t cs = checkSum(length, frame);

        if (syncRead != 3 | frameRead < MIN_FRAME_SIZE - 3) {
            endOfFile = 1;
        }
        else if (frame[length] != cs) {
            corruptFrameCount ++;
        }
        else {

            if (frame[1] - sequence > 1) {
                droppedFrames ++;
            }
        
        sequence = frame[1];

        //first question: what type of message is this?
        if (frame[0] == 0x01 && length == 12) { //is weight

            timeMs = (frame[3] << 8| frame[2]) - initialTime; 

            if (timeMs + (timeSec*1000) - timeMultiple*MAX_TIME > MAX_TIME) { //checking to see if the time wrapped
                timeMultiple ++;
            }

            timeSec = (timeMultiple*MAX_TIME + timeMs)/1000.0; //ms and wraps

            updateWeight(faults, combineBytes(frame[4],frame[5]), &weight1);
            updateWeight(faults, combineBytes(frame[6],frame[7]), &weight2);
            updateWeight(faults, combineBytes(frame[8],frame[9]), &weight3);
            updateWeight(faults, combineBytes(frame[10],frame[11]), &weight4);

            total_kg = weight1.currWeight+weight2.currWeight+weight3.currWeight+weight4.currWeight;

            if ((faults & 0b00001111) == 0x00) {
                fprintf(fo, "%.1f, %d\n", timeSec, total_kg);
            }
            else {
                fprintf(fo, "%.1f, %d, total kg is an estimate\n", timeSec, total_kg);
            }
  
        }
        else if (frame[0] == 0x02 && length == 13) { //status
            //faulted raw count should not be trusted
            //5th byte is faults
            faults = frame[4];
            weight1.offset = combineBytes(frame[5], frame[6]);
            weight2.offset = combineBytes(frame[7], frame[8]);
            weight3.offset = combineBytes(frame[9], frame[10]);
            weight4.offset = combineBytes(frame[11], frame[12]);
        }
        else {
            corruptFrameCount ++;
        }

        frameCount ++;
        }

    }
    
    fclose(f);
    fclose(fo);

    printf("Summary:\n - number of frames receive: %d\n", frameCount);
    printf("- frames that were likely corrupt: %d\n", corruptFrameCount);
    printf("- frames that were likely dropped: %d\n", droppedFrames);
    printf("- cart unloading time range: %d - %d");
}

float getWeight(int16_t rawData, int16_t offset) { 
    return (rawData-offset)*0.5;
}

void updateWeight(uint8_t faults, int16_t rawWeight, struct weight *w) {
    if ((faults & w->mask) == 0) {
        w->currWeight = getWeight(rawWeight, w->offset);
    }
}

int16_t combineBytes(uint8_t byte1, uint8_t byte2) {
    int16_t combined = byte1 | byte2 << 8;
    return combined;
}

uint8_t checkSum(uint8_t length, uint8_t frame[]) {
    int8_t calculatedSum = length;
    for (int i = 0; i < length; i++ ) {
        calculatedSum = calculatedSum^frame[i];
    }
    return calculatedSum;
}