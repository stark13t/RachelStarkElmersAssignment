#include <stdio.h>
#include <stdint.h>

#define int MAX_FRAME_SIZE = 17; //max size of frame including sync and length

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
    }

    int8_t sync[3]; 
    int8_t frame[MAX_FRAME_SIZE - 3];

    int16_t offset1, offset2, offset3, offset4;
    int16_t weight1, weight2, weight3, weight4;

    int16_t time = 0;

    int32_t total_kg;

    fprintf(fo,"t_s, total_kg\n");

    uint8_t i = 0;

    while (i < 10) {
        fread(sync, 1, 3, f); //fread(where to, bytes/element, num elements, stream)

        int8_t length = sync[2];

        fread(frame, 1, length+1, f); //read in rest of the frame, +1 because of checksum

        //first question: what type of message is this?
        if (frame[0] == 0x01) { //is weight
            time = time + combineBytes(frame[2],frame[3])/1000; //ms and wraps will deal with wrapping later

            weight1 = getWeight(combineBytes(frame[4], frame[5]), offset1);
            weight2 = getWeight(combineBytes(frame[6], frame[7]), offset2);
            weight3 = getWeight(combineBytes(frame[8], frame[9]), offset3);
            weight4 = getWeight(combineBytes(frame[10], frame[11]), offset4);
            total_kg = weight1+weight2+weight3+weight4;

            fprintf(fo, "%d, %d\n", time, total_kg);
        }
        else if (frame[0] == 0x02) { //status
            //faulted raw count should not be trusted
            //5th byte is faults
            offset1 = combineBytes(frame[5], frame[6]);
            offset2 = combineBytes(frame[7], frame[8]);
            offset3 = combineBytes(frame[9], frame[10]);
            offset4 = combineBytes(frame[11], frame[12]);
        }

        //frame thoughts: sequence can tell us if a frame was dropped
        //time will be in timestamp
        //payload has the weight
        i++;
    }
    
    fclose(f);
}

int16_t getWeight(int16_t rawData, int16_t offset) {    
   // int32_t weight_kg = (raw_count - zero_offset) * 0.5;
   return (rawData-offset)*0.5;
}

int16_t combineBytes(int8_t byte1, int8_t byte2) {
    int16_t combined = byte1 << 8 | byte2;
    return combined;
}
