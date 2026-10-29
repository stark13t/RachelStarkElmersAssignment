#include <stdio.h>
#include <stdint.h>

#define MAX_FRAME_SIZE 17 //max size of frame including sync and length
#define MIN_FRAME_SIZE 16
#define MAX_TIME 65535 //point where time wraps

static void read_data(char fileName[], char outfileName[]);
int16_t getWeight(int16_t rawData, int16_t offset);
int16_t combineBytes(int8_t byte1, int8_t byte2);

int main(void) {
    remove("dataOut.csv");
    read_data("capture.bin", "dataOut.csv");
}

void read_data(char fileName[], char outfileName[]) {
    FILE *f = fopen(fileName, "rb"); //open file in read binary mode
    FILE *fo = fopen(outfileName, "w+"); //csv output file

    if (f==NULL) {
        printf("Could not open file %s", fileName);
        return 1;
    }
    if (fo == NULL) {
        printf("Could not open file %s", outfileName);
        return 1;
    }

    int8_t sync[3]; 
    int8_t frame[MAX_FRAME_SIZE - 3];

    int16_t offset1, offset2, offset3, offset4;
    int16_t weight1, weight2, weight3, weight4;

    uint16_t timeMs = 0;
    uint16_t timeSec = 0;
    uint16_t timeMultiple = 0;

    int8_t faults;

    int32_t total_kg;

    fprintf(fo,"t_s, total_kg\n");

    uint16_t frameCount = 0;
    uint16_t corruptFrameCount = 0;

    uint8_t endOfFile = 0;

    while (endOfFile != 1) {
        int syncRead = fread(sync, 1, 3, f); //fread(where to, bytes/element, num elements, stream)

        int8_t length = sync[2];

        int frameRead = fread(frame, 1, length+1, f); //read in rest of the frame, +1 because of checksum

        if (syncRead != 3 | frameRead < MIN_FRAME_SIZE - 3) {
            endOfFile = 1;
        }
        else {

        //first question: what type of message is this?
        if (frame[0] == 0x01) { //is weight

            timeMs = (frame[3] | frame[2] << 8);

            if (timeMs + (timeSec*1000) - timeMultiple*MAX_TIME > MAX_TIME) { //checking to see if the time wrapped
                timeMultiple ++;
            }

            timeSec = (timeMultiple*MAX_TIME + timeMs)/1000; //ms and wraps

            if ((faults & 0x01) != 1) {
                weight1 = getWeight(combineBytes(frame[4], frame[5]), offset1);
            }
            if ((faults & 0x02) != 2) {
                weight2 = getWeight(combineBytes(frame[6], frame[7]), offset2);
            }
            if ((faults & 0x04) != 4) {
                weight3 = getWeight(combineBytes(frame[8], frame[9]), offset3);
            }
            if ((faults & 0x08) != 8) {
                weight3 = getWeight(combineBytes(frame[8], frame[9]), offset3);
            }

            weight4 = getWeight(combineBytes(frame[10], frame[11]), offset4);
            total_kg = weight1+weight2+weight3+weight4;
            if ((faults & 0b00001111) == 0x00) {
                fprintf(fo, "%d, %d\n", timeSec, total_kg);
            }
            else {
                fprintf(fo, "%d, %d, total kg is an estimate\n", timeSec, total_kg);
            }
  
        }
        else if (frame[0] == 0x02) { //status
            //faulted raw count should not be trusted
            //5th byte is faults
            faults = frame[4];
            offset1 = combineBytes(frame[5], frame[6]);
            offset2 = combineBytes(frame[7], frame[8]);
            offset3 = combineBytes(frame[9], frame[10]);
            offset4 = combineBytes(frame[11], frame[12]);
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
    printf("- frames that were likely dropped: %d\n");
    printf("- cart unloading time range: %d - %d");
}

int16_t getWeight(int16_t rawData, int16_t offset) {    
   // int32_t weight_kg = (raw_count - zero_offset) * 0.5;
   return (rawData-offset)*0.5;
}

int16_t combineBytes(int8_t byte1, int8_t byte2) {
    int16_t combined = byte2 << 8 | byte1;
    return combined;
}
