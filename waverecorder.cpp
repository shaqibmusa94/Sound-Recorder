#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <stdint.h>


#define PLAYPINBUTTON            4
#define STOPPINBUTTON            7
#define PINPOT                  A0
#define PLAY_BTN_PRESSED        LOW
#define PLAY_BTN_NOT_PRESSED   HIGH
#define SD_SELECT               9
#define DURATION                60
#define STOP_BTTN_PRESSED       LOW
#define STOP_BTTN_NOT_PRESSED      HIGH

uint32_t bitDepth = 8;
uint32_t sampleRate = 16000UL;
int playButtonValue = HIGH;
int stopButtonValue = HIGH;
uint32_t length_audio = (sampleRate * DURATION);


char Riff[4] = {'R','I','F','F'};
char totalSize[4] = {'-', '-', '-', '-'};
char wave[4] = {'W','A','V','E'};
char fmt[4] = {'f','m','t',' '};
char audiodata[4] = {'d','a','t','a'};
char sizeAudioData[4] = {'-','-','-','-'};
char fileName[] = "mak1.wav";
File myFile;


void write_to_File(File &file ,uint32_t value, uint32_t size);

void setup() {

  Serial.begin(9600);
  pinMode(PLAYPINBUTTON, INPUT);
  pinMode(STOPPINBUTTON, INPUT);
  pinMode(PINPOT, INPUT);

  //increase the ADC conversion speed
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2));                   // clear prescaler bits
  ADCSRA |= bit (ADPS2);                                                  // set prescaler bit to 16            

  Serial.println("Initializing SD card.....");
  if (SD.begin(SD_SELECT))
  {
    Serial.println("SD is recognized");
  } else
  {
    Serial.println("SD found error");
  }
}


void loop() 
{
  playButtonValue = digitalRead(PLAYPINBUTTON);
  

  if(playButtonValue == PLAY_BTN_PRESSED)
  {
    Serial.println("Button is pressed");
    myFile = SD.open(fileName, FILE_WRITE);
    
    //HEADER CHUNK
    myFile.write(Riff, 4);                                    //the RIFF must be stored using byte array not string. String occupies 5 bytes.
    myFile.write(totalSize, 4);                               //The size of the data is unknown at first. Put it at the end 
    myFile.write(wave, 4);                                    //WAVE characters

    //FORMAT CHUNK
    myFile.write(fmt, 4);                                     //fmt characters with NULL 
    write_to_File(myFile , 16, 4);                            //this must be 16, it shows the length of format data
    write_to_File(myFile , 1 , 2);                            //type of data. 1 - PCM
    write_to_File(myFile , 1 , 2);                            //number of channel
    write_to_File(myFile , sampleRate, 4);                    //sample rate 
    write_to_File(myFile , (sampleRate * bitDepth)/8 , 4);    //sampleRate*bitspersample*no.ofchannel/8
    write_to_File(myFile , bitDepth/8, 2);                    //bitspersample*channel/8.  1 - 8 bit mono
    write_to_File(myFile , bitDepth, 2);                      //bit depth

    //DATA CHUNK
    myFile.write(audiodata, 4);                               //data characters
    myFile.write(sizeAudioData, 4);                           //size of audio data.put it later

    uint32_t preAudioposition = myFile.position();            //create marker before audio data enter

    Serial.println("Audio Start!");   

    //AUDIO DATA READ                

    while(stopButtonValue == STOP_BTTN_NOT_PRESSED)
    {
      byte data = analogRead(PINPOT);
      stopButtonValue = digitalRead(STOPPINBUTTON);
      myFile.write(data);
    } 

    Serial.println("Recording Done");

    uint32_t postAudioposition = myFile.position();           //marker after audio data enter

    myFile.close();

    myFile = SD.open(fileName, O_RDWR);                       //reopen to write size of audio data and total size

    myFile.seek(preAudioposition - 4);                        //locate the marker at size of audio data
    write_to_File(myFile , postAudioposition - preAudioposition, 4); //write the size

    myFile.seek(4);                                           //locate the marker at the total size of file
    write_to_File(myFile , postAudioposition - 8, 4);         //write the total size

    Serial.println("Writing is done");
    Serial.println(postAudioposition);                        //check the size of the file

    myFile.close();

    playButtonValue = HIGH;
    stopButtonValue = HIGH;
    delay(1000);

  }
}


void write_to_File(File &file ,uint32_t value, uint32_t size)
{
  file.write(reinterpret_cast<const char*> (&value), size);
}
