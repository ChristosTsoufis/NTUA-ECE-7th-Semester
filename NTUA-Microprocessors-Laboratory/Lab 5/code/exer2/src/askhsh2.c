#define GPIO_SWs    0x80001400 // defines the memory position of the switches
#define GPIO_LEDs   0x80001404 // defines the memory position of the LEDs
#define GPIO_INOUT  0x80001408 // defines the memory position of input (switches) / output (leds)

#define READ_GPIO(dir) (*(volatile unsigned *)dir) // reads from dir and returns the memory position of dir
#define WRITE_GPIO(dir, value) { (*(volatile unsigned *)dir) = (value);} // finds the memory position of dir and writes on it the new value

// function that lights ON/OFF the LEDs
void leds(int complem_val, int ones){ // the arguments are the switches that will switch on and the sum
    int cntr = 65000; // virtual delay
    while(ones > 0){
        WRITE_GPIO(GPIO_LEDs, complem_val); // switch on the LEDs of complement
        while(cntr > 0){cntr--;} // loop that reduces counter
        WRITE_GPIO(GPIO_LEDs, 0); // switch off LEDs due to 0
        cntr = 6500; // virtual delay
        while(cntr > 0){cntr--;} // loop that reduces counter
        cntr = 6500; // virtual delay
        ones--; // reduce sum
    }
}

int main (void){   
    // initializations
    int En_Value = 0xFFFF; 
    WRITE_GPIO(GPIO_INOUT, En_Value);
    unsigned int comp_val, switches_val, prev_val = 0, switches_val_initial = 0;
    int cntr = 0, sum = 0, last_bit = 0;

    while(1){ // for continuous operation
        switches_val = READ_GPIO(GPIO_SWs); // read the value of switches & place them accordingly for the LEDs
        switches_val >>= 16; // shift what you read 16 positions to the right (LSB) so that LEDs can 'understand'
        switches_val_initial = (switches_val << 16); // shift 16 positions to the left to take the initial value
        comp_val = ~switches_val; // calculate the complement value of switches
        
        // calculation of ones
        while(cntr <= 15){ // it counts 16 times since the value of the switches is on the 16 LSB
            if((last_bit = (comp_val >> cntr) & 1) == 1){ // initially it does 0 shifts, then 1 shift etc
                sum++; // calculats the number of ones
            }
            cntr++; // counter is used for the number of loops
        }

        leds(comp_val, sum); // go to the function above

        prev_val = switches_val_initial;
        switches_val = switches_val_initial;
        // for as long as the MSB of the switches remains the same, just run
        // but if the MSB of the switches value changes, repeat the whole process
        while((switches_val & 0x80000000) == (prev_val & 0x80000000)){ // mask these values
            
            switches_val &= 0x80000000;
        }

        sum = 0 ;
        cntr = 0;
    }
    return(0);
}