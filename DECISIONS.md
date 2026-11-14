# Decisions

## What I built
I built a program that reads in the binary data frames from capture.bin, processes that data, and saves the time and weight given by each weight frame in dataOut.csv. For each frame, the program reads in the first 3 bytes, then uses the information from length to read in the rest of the frame. Then, it uses the Type byte to identify what type of frame it is. If the incoming frame is a status frame, the program updates the faults and offsets. If it is a weight frame the raw data is used to calculate the total weight, which is then saved to dataOut.csv along with the time of that frame. Data I don't trust because it comes from a faulted cells is replaced with an estimate in dataOut.csv, data I don't trust for other reasons is not included in dataOut, and instead increments a corrupt frame count. 

## Choices I made (and what I rejected)
- I initially planned to read and divide all the data as one step, test it as a second step and write it to the csv file as a third sepearate step, and have each step divided into its own function. I rejected this idea because of how much data there was. Storing the frames in memory would have taken a large amount of storage, and it would have been slow since I would be going through all of the data three times. Instead I test and store the data as it comes in.

## Things in the brief or data that didn't add up, and what I did
- The current data coming out has negative weights. I have checked that the bytes are being processed correctly, which removed some negative weights but not all of them. I checked some of the remaining negative weights by hand and the ones I checked appear to be coming from the data itself. Since all the negative values are near zero, I assumed they are coming from noise with the data itself, and chose to leave the negative weights in dataOut.csv.

## Tools I used
I used chat gpt to set up the Dockerfile program and trouble shoot problems I encountered with git. I tried to also use it to test if I was reading in bits correctly, but chat gpt was repeatedly incorrect or circular when asked about bit logic.

## Tests
- I tested to ensure my program is adding up the checksum as expected using the example frame provided in the handout. I also tested that the weight calculations for both a faulted and unfaulted cell were as expected.

Would test next:
    - test that the time is correctly calculated before and after the time bytes wrap back to 0

## With another day I would
- With another day I would update the way I am reading in the frames to protect against lengths that are not expected. I would also create a check to ensure that the sync bytes are always where they are expected to be, and work on reducing the amount that the readdata function is responsable for, so the program is easier to test.
