# Decisions

## What I built
I built a program that reads in the binary data frames from capture.bin, processes that data, and saves the time and weight given by each weight frame in dataOut.csv. For each frame, the program reads in the first 3 bytes, then uses the information from length to read in the rest of the frame. Then, it uses the Type byte to identify what type of frame it is. If the incoming frame is a status frame, the program updates the faults and offsets. If it is a weight frame the raw data is used to calculate the total weight, which is then saved to dataOut.csv along with the time of that frame. Data I don't trust because it comes from a faulted cells is replaced with an estimate in dataOut.csv, data I don't trust for other reasons is not included in dataOut, and instead increments a corrupt frame count. 

## Choices I made (and what I rejected)
- I initially planned to read and divide all the data as one step, test it as a second step and write it to the csv file as a third sepearate step, and have each step divided into its own function. I rejected this idea because of how much data there was. Storing the frames in memory would have taken a large amount of storage, and it would have been slow since I would be going through all of the data three times. Instead I test and store the data as it comes in.

## Things in the brief or data that didn't add up, and what I did
- Initially the bytes being little endian confused me, because I assumed that that meant the bits were little endian, so the frame 0x12 would become 0x48. This yeilded confusing data, and after reading the brief more carefully I corrected the approach. Now 0x12 is read as 0x21.

## Tools I used
I used chat gpt to set up the Dockerfile program and trouble shoot problems I encountered with git. I tried to also use it to test if I was reading in bits correctly, but chat gpt was repeatedly incorrect or circular when asked about bit logic.

## Tests
- I tested to make sure the length and type bytes matched, because reading a weight frame as a status frame would change the faults and offsets, making all future weights incorrect until a true status frame is received. I also tested to make sure each scale was not faulted before reading in the weight from that cell. If there is a fault, the program does not update that cell's weight and marks the total kg as an estimate.

Would test next:
    - check that checksum matches with the received frame
    - check to make sure any large jumps in time are accompanied by a jump in sequence and true in future frames so that time multiple is not falsely incremented.
    - check that sync is always at the start of sync[]

## With another day I would
- With another day I would complete the print out at the end of the program. I would check sequence to see if it was increasing by more than 1 to find when frames were dropped, and find when the total weight begins decreasing for the cart unloading time range.

I would also add more tests to check if the frames are corrupted, as described in the testing section.