#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

const int SF_MESSAGE_OFFSET = 5; //offset for start of message in Single frame
const int FF_MESSAGE_START = 7; //start index of message in First frame
const int CF_MESSAGE_START = 5; //start index of message in Consecutive frame
const int FF_MESSAGE_LEN = 12; //length of message part in First frame(in characters)
const int CF_MESSAGE_LEN = 14; //length of message part in Consecutive frame(in characters)
const int ECU_HEADER_START = 0;
const int ECU_HEADER_LEN = 3;
const int FRAME_TYPE_INDEX = 3;
const int FF_MSGLEN_START = 4; //start index of First frame message length
const int FF_MSGLEN_LEN = 3; //length of First frame message length
const int FCF_ALLOWED_IND = 4; //index of flow control frame's 'transfer allowed' flag
const int FCF_BLOCKSIZE_IND = 5;
const int FCF_BLOCKSIZE_LEN = 2;
const int FCF_ST_IND = 5; //index of separation time in flow control frame
const int FCF_ST_LEN = 2;

// Convert hexadecimal string into decimal int
int hexStringToInt(const std::string& str)
{
    int res;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> res;
    
    return res;
}

// Parses and prints Single frame
void parseSingleFrame(const std::string& frame)
{
    std::string ecuHeader = frame.substr(0, 3);
    int messageLenChars = std::stoi(frame.substr(4, 1)) * 2; //message length in characters = message length in bytes(from 0 to 7) * 2
    
    std::cout << ecuHeader << ": ";
    for (int i = 0; i < messageLenChars; i ++)
    {
        std::cout << frame[i + SF_MESSAGE_OFFSET];
    }
    std::cout << std::endl;
}

// Parses First frame. Returns multiframe message length left to parse after the first frame (in characters, not bytes).
// Note - Here I'm assuming that they always come in order first->flow->consecutives, if not I would probably do it with
//   a hash table with key-value 'ECU header-message'
int parseFirstFrame(const std::string& frame, std::string& message)
{
    //convert hexadecimal message length to int
    int messageLen = hexStringToInt(frame.substr(FF_MSGLEN_START, FF_MSGLEN_LEN));
    int messageLenCharsLeft = messageLen * 2; //message length in characters = message length in bytes * 2

    message = frame.substr(FF_MESSAGE_START, FF_MESSAGE_LEN); //put this first frame's part in the message
    messageLenCharsLeft -= FF_MESSAGE_LEN;

    return messageLenCharsLeft;
}

// Parses Flow control frame
void parseFlowControl(const std::string& frame)
{
    //Note - Not sure that we care about these in my assignment(when allowed flag is not 0) so I'm just gonna take the info to use if needed
    int blockSize = hexStringToInt(frame.substr(FCF_BLOCKSIZE_IND, FCF_BLOCKSIZE_LEN));
    int separationTime = hexStringToInt(frame.substr(FCF_ST_IND, FCF_ST_LEN));
    
    // 0 = Continue To Send, 1 = Wait, 2 = Overflow/abort
    switch (frame[FCF_ALLOWED_IND])
    {
    case '0':
        break;
    case '1':
        break;
    case '2':
        break;
    }
}

// Parses Consecutive frame and adds its message part to 'message'
void parseConsecutiveFrame(const std::string& frame, std::string& message, int& messageLenLeft)
{
    if (messageLenLeft > CF_MESSAGE_LEN)
    {
        message += frame.substr(CF_MESSAGE_START, CF_MESSAGE_LEN);
        messageLenLeft -= CF_MESSAGE_LEN;
    }
    else
    {
        message += frame.substr(CF_MESSAGE_START, messageLenLeft);
        messageLenLeft = 0;
    }
}


// Parses and prints all messages from file 'transcript' in the format 'ECU_HEADER: CAN_MESSAGE'. Returns 0 on success, -1 on failure.
int parseFramesAndPrintMessages(std::ifstream& transcript)
{
    std::string frame;
    std::string message;
    int messageLenLeft = -1; //how many characters left in the message 

    while (std::getline(transcript, frame))
    {
        //Check type of frame: 0 - Single frame, 1 - First frame, 2 - Consecutive frame, 3 - Flow control frame
        switch (frame[FRAME_TYPE_INDEX])
        {
        case '0':
            parseSingleFrame(frame);
            break;
        case '1':
            messageLenLeft = parseFirstFrame(frame, message);
            break;
        case '2':
            parseConsecutiveFrame(frame, message, messageLenLeft);
            if (0 == messageLenLeft)
                std::cout << frame.substr(ECU_HEADER_START, ECU_HEADER_LEN) << ": " << message << std::endl;
            break;
        case '3':
            parseFlowControl(frame);
            break;
        default:
            std::cerr << "Invalid type of frame!" << std::endl;
            return -1;
        }
    }

    return 0;
}

int main()
{
    std::ifstream transcript;
    transcript.open("transcript.txt");
    if (!transcript)
        std::cerr << "Failed opening file!" << std::endl;
    else
        parseFramesAndPrintMessages(transcript);

    transcript.close();
    return 0;
}