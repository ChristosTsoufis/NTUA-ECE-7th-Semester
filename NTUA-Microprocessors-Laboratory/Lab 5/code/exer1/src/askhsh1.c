#define GPIO_SWs    0x80001400 // defines the memory position of the switches
#define GPIO_LEDs   0x80001404 // defines the memory position of the LEDs
#define GPIO_INOUT  0x80001408 // defines the memory position of input (switches) / output (leds)

#define READ_GPIO(dir) (*(volatile unsigned *)dir) // reads from dir and returns the memory position of dir
#define WRITE_GPIO(dir, value) {(*(volatile unsigned *)dir) = (value);} // finds the memory position of dir and writes on it the new value

int main (void){
    // initializations
    int En_Value = 0xFFFF;
    int switches_val, first, second;
    // configuration part
    WRITE_GPIO(GPIO_INOUT, En_Value); // it is used to declare that LEDs will be used for ouput and switches as input

    while (1){
        switches_val = READ_GPIO(GPIO_SWs); // read the value of switches
        // it is shifted to the right because the value of switches is in MSB
        first = (0x000f0000 & switches_val)>>16; // mask
        second = (0xf0000000 & switches_val)>>28; // mask
        switches_val = first + second; // add 4 LSB with 4 MSB

        // write to LEDs
        if (switches_val <= 15){WRITE_GPIO(GPIO_LEDs, switches_val);}
        else{WRITE_GPIO(GPIO_LEDs, 16);} // overflow
    }
    return(0);
}