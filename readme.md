## Project Architecture
Client A
   ↓
Encrypt + Hash
   ↓
TCP Socket
   ↓
Server Routing
   ↓
Client B
   ↓
Integrity Verification
   ↓
Decrypt
   ↓
ML Analysis
   ↓
Moderation

## Project Structure
ChatCrypt/
│
├── server.cpp
├── client.cpp
├── ml_server.py
├── train_spam_model.py
├── spam_model.pkl
├── spam.csv
│
└── README.md

## Technologies Used
Component	        Technology
Networking	        C++ TCP Sockets
Multiplexing        select()
ML Backend	        Python
Sentiment Analysis	HuggingFace Transformers
Spam Detection	    Scikit-learn
Cryptography	    XOR
Encoding            HEX
## Protocol Design
JOIN  : JOIN|username|room

Group Message :
MSG|room|sender|hash|hex_encoded_encrypted_message

Private Message :
PMSG|room|sender|receiver|hash|hex_encoded_encrypted_message

Moderation Report :
REPORT|reporter|target|reason

## Security Design
plaintext
   ↓
XOR Encryption
   ↓
HEX Encoding
   ↓
send via TCP
   ↓
HEX Decoding
   ↓
XOR Decryption

## Privacy Model

The server acts only as a routing intermediary and cannot view plaintext messages.

All encryption and decryption operations occur on client devices.

Machine learning moderation is also performed locally on clients after decryption, preserving end-to-end privacy.

## Integrity Verification
Each encrypted message is hashed before sending.
Receiver recomputes the hash and compares it with the received hash.

If mismatch occurs: ⚠️ MESSAGE TAMPERING DETECTED

## Machine Learning Integration
->Sentiment Analysis :
Implemented using transformer-based NLP models.
Detects:
-POSITIVE
-NEGATIVE

->Spam Detection

Trained using:
-TF-IDF Vectorization
-Naive Bayes Classifier

Dataset:
-SMS Spam Collection Dataset

## Moderation System
If a message is:
-spam AND
-negative
then the sender loses reputation points.
Score Action
-10   Warning
-20   Muted
-30   Permanently Blocked

## Installation & Setup
1. Clone Project
    git clone <repo-url>
    cd ChatCrypt

2. Linux / WSL Setup
-Install dependencies
sudo apt update
sudo apt install python3-pip python3-venv g++ curl

3. Create virtual environment
python3 -m venv venv
source venv/bin/activate

4. Install Python packages
pip install flask transformers torch --index-url https://download.pytorch.org/whl/cpu
pip install pandas scikit-learn joblib

5. Train Spam Model
Make sure: spam.csv  is inside project directory.
Then run:
python3 train_spam_model.py
This creates:
spam_model.pkl

6. Compile C++ Files
g++ server.cpp -o server
g++ client.cpp -o client

## Running the Project

1. Terminal 1 — ML Server
python3 ml_server.py

2. Terminal 2 — Chat Server
./server

3. Terminal 3 — Client A
./client

## Usage Examples
1. Group Message
Simply type: hello everyone

2. Private Message
/pm B hello privately
only user B recieves it.

A sends encrypted msg
↓
Server forwards blindly
↓
B decrypts locally
↓
B ML detects toxicity
↓
B sends WARN|A
↓
Server routes WARN packet
↓
A receives:
"You received a warning"




