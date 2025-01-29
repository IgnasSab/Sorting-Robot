#include "RPI_class.h"
#include <Arduino.h>
using namespace std;

RPI_COM::RPI_COM() {
    /**
     * @param black number of black disks sorted
     * @param white number of white disks sorted
     * @param misc number of misc disks sorted
     * @param running whether the conveyor belt is running or not
     * @param sorting whether the sorting mode is off, automatic, or manual
     * @pre @code{black, white, misc >= 0}
     * @post packet is sent to the RPI 
    */
    this->black = 0;
    this->white = 0;
    this->misc = 0;
    this->running = false;
    this->sorting = 0;
}

void RPI_COM::resetValues() {
  black = 0;
  white = 0;
  misc = 0;
  running = false;
  sorting = 0;
}

/** 
 * This function sends a packet to the RPI containing all the useful information that will
 * be displayed on the website status
 */
void RPI_COM::sendPacket() {
    char buffer[20];
    sprintf(buffer, "%d,%d,%d,%d,%d", black, white, misc, 
        (running ? 1 : 0), sorting);    
    Serial.println(buffer);
}

/**
 * This functions sends an error to the raspberry pi
 * 
 * @param error the error code as integer (1-3)
 */
void RPI_COM::sendError(int error) {
    char buffer[1];
    sprintf(buffer, "%d", error);    
    Serial.println(buffer);
}
/**
 * CURRENTLY NOT USED.
 */
void RPI_COM::receivePacket() {

}
/**
 * Destructor.
 */
RPI_COM::~RPI_COM() {
    
}