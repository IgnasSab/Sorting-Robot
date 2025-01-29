#pragma once
/**
 * This class gives a clear ability to stored data and send data packets (including sorting status and errors)
 * from Arduino to RPI.
 */
class RPI_COM {
    public:
        int black;
        int white;
        int misc;
        bool running;
        int sorting;
        RPI_COM();
        void resetValues();
        void sendPacket();
        void receivePacket();
        void sendError(int error);
        ~RPI_COM();
    private:
};