void setup() {
    Serial.begin(9600);
    Serial.println("Initializing...");
    Serial1.begin(9600); // According to the Teensy Docs, this is the RX1 & TX1 on my board.
    // Serial itself corrosponds to the micro-usb port
}
String msg = "";      

void loop() {
    if(Serial1.available() > 0) {
      //Serial.println("Serial 1 available");
      msg = "";
      while(Serial1.available() > 0) {
          char rd = Serial1.read();
          msg += rd;
      }
      Serial1.write('X'); // Acknowledge with reply
      Serial.println(msg); // Output to console for debugging
    }

    // Should be a number 1-9
}
