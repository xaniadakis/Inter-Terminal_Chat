# Inter-Terminal Chat

In this project I implement a multi-level messaging architecture according to the figure below. 

Five processes are available: P1, P2, ENC1, ENC2 and CHAN.
Processes P{1|2} accept and present data to the user. 
The input to processes P{1|2} is a line of text which is entered as a message with
the prospect of sending to the other process (i.e. P1 → P2 or P2 → P1).
If the message is entered in P1 it will be "transmitted" from the underlying levels to
P2 and if the correctness of the message is found, it will be presented to the user
(of P2). Processes ENC1 and ENC2 receive messages from P{1|2},
calculate the hash value of the given string (checksum message) and 
then channel the augmented message (simple + checksum)
in the CHAN process which simulates the transmission network.

![image](https://user-images.githubusercontent.com/75081526/118978364-e641f700-b97f-11eb-9fae-71ef40a4dc9b.png)

The CHAN process replaces the message characters with others
randomly based on a probability (change) provided externally as the parameter of the
overall program. If the message-string in its path from P1 to
P2 (and vice versa) is found in ENC{1|2} altered, checksum control leads
to rejection of the incoming message. If the message, despite the random
replacement process is received unchanged, it gets unwrapped (from
control information) and is provided in the overhead application (for visualization
to the user). If the message (eg from P1), due to its intentional alteration,
omitted with discrepancies between string and checksum, the process
ENC2 requests the retransmission of the relevant information.

Sending the message "bye" through a process P{1|2} and passing it through
the rest (ENC ?, CHAN, etc.) marks the end of all
processes and the final release of synchronization structures. The edges between
of the above processes are implemented with shared memory spaces (in/out
structures) and are framed by appropriate semaphores.

The application gets compiled as follows:

    make clean all
    
The application can be run as follows:

    ./p1 <probability_of_fault>   #from one terminal tab
    
    ./p2 <probability_of_fault>   #from another terminal tab

probability_of_fault ranges from 0-100 

At runtime, P1 and P2 alternately receive control and wait for user input from the keyboard, which will then be transmitted to the opposing process through the mechanism that is elaborated upon in the above section.
