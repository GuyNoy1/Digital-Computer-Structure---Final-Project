#include "../header/halGPIO.h"  // Include the HAL layer header
#include "string.h"
#include "stdlib.h"

#define PI 3.14159265
#define SEG_FLASH_START_ADDRESS 0x1000
#define CALIB_COUNTER_FLASH_ADDRESS 0x1034
#define STEP_SIZE_FLASH_ADDRESS 0x103a

Folder folder;
float step_size;
int d_delay = 50;
unsigned char state1_stage = '0';
unsigned char state2_stage = '0';
unsigned char state3_stage = '0'; //state 3 stage
unsigned int calib_counter = 0;
char calib_counter_LCD[5] = {'\0'};
char JS_angle_LCD[5] = {'\0'};
char step_size_LCD[8] = {'\0'};
int y_avg = 0;
int x_avg = 0;
int adc_samples[]={0, 0};
int JS_angle = 0;
float motor_angle = 0;
char motor_angle_LCD[7] = {'\0'};
int Tx_idx = 0;
int Rx_idx = 0;
int JS_PB_IFG = 0;
int Script_burn_or_execute_choise = 0;
char file_from_PC[80];
char temp_buffer[64];
char temp_step_size[5];
char temp_calib_counter[5];
int file_Seg;


//--------------------------------------------------------------------
//             system configuration
//--------------------------------------------------------------------
void sysConfig(void){
    GPIOconfig();
    TIMER_0_config();
    ADCconfig();
    UART_config();
}
//--------------------------------------------------------------------
//             sleep with interrupts
//--------------------------------------------------------------------
void sleep_with_interrups(){
    __bis_SR_register(LPM0_bits + GIE);

}
//---------------------------------------------------------------------
//            enable interrupts
//---------------------------------------------------------------------
void enable_interrupts()
{
    _BIS_SR(GIE);  // Enable global interrupts
}

//---------------------------------------------------------------------
//            disable interrupts
//---------------------------------------------------------------------
void disable_interrupts()
{
    _BIC_SR(GIE);  // Disable global interrupts
}
//---------------------------------------------------------------------
//            polling based delay functions
//---------------------------------------------------------------------
void delay(unsigned int t)
{  // t[msec]
    volatile unsigned int i;  // Declare volatile variable
    for (i = t; i > 0; i--)
        ;  // Delay loop
}

void DelayMs(unsigned int cnt)
{
    unsigned char i;           // Declare variable
    for (i = cnt; i > 0; i--)  // Loop for delay in milliseconds
        DelayUs(1000);         // Delay for 1000 microseconds
}

void DelayUs(unsigned int cnt)
{
    unsigned char i;           // Declare variable
    for (i = cnt; i > 0; i--)  // Loop for delay in microseconds
        __delay_cycles(1);     // Delay for 1 cycle
}
//---------------------------------------------------------------------
//            timer based delay functions
//---------------------------------------------------------------------
void timer_delay_HZ(int freq) {
    //linear transformation to recieve freq 5<f<50
    int counter;
    counter = 125000/freq; //1M/8 = 125000 -> 125000/f=counter
    TACCR0 = counter;
    TACTL |= MC_1 + TACLR;
     __bis_SR_register(LPM0_bits);
}

//******************************************************************
// motor rotate functions
//******************************************************************
void Rotate_clockwise(int rotate_freq){
 
    Motor_arr = 0x08; // t0 - phase A = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    Motor_arr = 0x01; // t1 - phase d = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    Motor_arr = 0x02; // t2 - phase d = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    Motor_arr = 0x04; // t3 - phase d = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    calib_counter += 1;

}

void Rotate_counter_clockwise(int rotate_freq){

    Motor_arr = 0x01; // t0 - phase A = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    Motor_arr = 0x08; // t1 - phase d = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    Motor_arr = 0x04; // t2 - phase d = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    Motor_arr = 0x02; // t3 - phase d = 1
//    calib_counter++;
    timer_delay_HZ(rotate_freq);
    calib_counter += 1;

}

void rotate_motor_to_angle(int desired_angle) {
    int angle_difference = desired_angle - motor_angle;
    
    // Normalize the angle difference to be within -360 to 360
    if (angle_difference > 180) {
        angle_difference -= 360;
    } else if (angle_difference < -180) {
        angle_difference += 360;
    }

    // int steps_to_move = abs(angle_difference) / step_size;
    if (angle_difference > 0) {
        Rotate_clockwise(500);  // Move clockwise if desired angle is greater
        motor_angle += step_size;
    } else if (angle_difference < 0) {
        Rotate_counter_clockwise(500);  // Move counterclockwise if desired angle is less
        motor_angle -= step_size;
    }
    if (motor_angle < 0) motor_angle += 360;
    else if(motor_angle > 360) motor_angle -= 360;
}



//******************************************************************
// JS functions
//******************************************************************
void sample_JS() {
    x_avg = 0;
    y_avg = 0;
    int index;
    for (index = 0; index < 16; index++){
        ADC10CTL0 &= ~ENC;
        while (ADC10CTL1 & ADC10BUSY);
        ADC10SA = &adc_samples;
        ADC10CTL0 |= ENC + ADC10SC;  // Enable and start conversion
        __bis_SR_register(LPM0_bits + GIE);  // Enter LPM0 with interrupts enabled
        x_avg += adc_samples[0];
        y_avg += adc_samples[1];
    }
    x_avg /= 16;
    y_avg /= 16;
}

float simple_atan_degrees(float z) {
    // Constants for different ranges
    const float a = 0.0776509570923569f;  // Coefficient for z^3 term (small z)
    const float b = 0.287434475393028f;   // Coefficient for z term (intermediate z)
    const float c = 0.636619772367581f;   // for large z approximation

    if (z < 0) return -simple_atan_degrees(-z);
    if (z > 1.0) {
        if (z > 5.0) {
            return 90.0f - (c / z);  // Approximation for very large z
        }
        return 90.0f - simple_atan_degrees(1.0f / z);  // Use reciprocal for moderate z
    }
    float zz = z * z;
    return z * (180.0f / PI) / (1.0f + a * zz + b * zz * zz);
}



int simple_atan2(int y, int x) {
    float angle;

    if (x > 0) {
        angle = simple_atan_degrees((float)y / x);
    } else if (x < 0 && y >= 0) {
        angle = simple_atan_degrees((float)y / x) + 180.0f;
    } else if (x < 0 && y < 0) {
        angle = simple_atan_degrees((float)y / x) - 180.0f;
    } else if (x == 0 && y > 0) {
        angle = 90.0f;
    } else if (x == 0 && y < 0) {
        angle = -90.0f;
    } else {
        angle = 0.0f; // x = 0, y = 0 (undefined, but set to 0)
    }

    // Normalize angle to be between 0 and 360 degrees
    if (angle < 0) {
        angle += 360.0f;
    }

    return (int)angle;
}

int get_JS_angle(){
    int Vx = x_avg - 512;
    int Vy = y_avg - 512;
    int alpha = simple_atan2(Vy, Vx);
    return alpha;
}

//******************************************************************
// Script Mode functions
//******************************************************************
void intToStr(char *str, unsigned int num) {
    char temp[10]; // Temporary array to hold characters
    int i = 0, j;

    // Convert each digit to character and store in temp array
    do {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    // Reverse the characters and store in the final string
    for (j = 0; j < i; j++) {
        str[j] = temp[i - j - 1];
    }

    str[i] = '\0';
}

void floatToStr(char *str, float num, int precision) {
    char temp[20];  // Temporary array to hold characters
    int i = 0, j;

    // Handle negative numbers
    if (num < 0) {
        str[i++] = '-';
        num = -num;
    }
    // Extract the integer part
    unsigned int intPart = (unsigned int)num;

    // Convert integer part to string
    do {
        temp[i++] = (intPart % 10) + '0';
        intPart /= 10;
    } while (intPart > 0);

    // Reverse the characters and store in the final string
    for (j = 0; j < i; j++) {
        str[j] = temp[i - j - 1];
    }

    str[i] = '.';  // Add the decimal point
    i++;           // Move past the decimal point

    // Extract and convert the fractional part
    float fractionalPart = num - (int)(num);
    for (j = 0; j < precision; j++) {
        fractionalPart *= 10;
        int digit = (int)fractionalPart;
        str[i++] = digit + '0';
        fractionalPart -= digit;
    }
    str[i] = '\0';  // Null-terminate the string
}

void floatToStrWithPrefix(char *str, float num, int precision) {
    char temp[20];  // Temporary buffer to hold the number as a string
    floatToStr(temp, num, precision);  // Use the original function to convert the float to a string
    // Add the 'A' prefix to the start of str
    str[0] = 'A';
    // Copy the result of floatToStr into str after the 'A'
    int i = 0;
    while (temp[i] != '\0') {
        str[i + 1] = temp[i];
        i++;
    }
    str[i + 1] = '\0';  // to mark end of string
}


void timer_delay_d() {
    const int max_d_per_cycle = 52;  // Max d that can be handled in one timer cycle
    int cycles = 0;
    int remaining_d = d_delay;
    int i;

    while (remaining_d > max_d_per_cycle) {
        cycles++;
        remaining_d -= max_d_per_cycle;
    }

    for (i = 0; i < cycles; i++) {
        TACCR0 = 1250 * max_d_per_cycle;  // Set CCR0 for the max cycle delay
        TACTL |= MC_1 + TACLR;
        __bis_SR_register(LPM0_bits);
    }

    if (remaining_d > 0) {
        TACCR0 = 1250 * remaining_d;      // Set CCR0 for the remaining delay
        TACTL |= MC_1 + TACLR;
        __bis_SR_register(LPM0_bits);
    }
}


void inc_lcd(int x){
    int counter = 0;
    char counter_str[8] = {'\0'};

    while(counter <= x){
        lcd_clear();
        intToStr(counter_str, counter);
        lcd_puts(counter_str);
        timer_delay_d();
        counter++;
    }
}

void dec_lcd(int x){
    int counter = x;
    char counter_str[8] = {'\0'};

    while(counter >= 0){
        lcd_clear();
        intToStr(counter_str, counter);
        lcd_puts(counter_str);
        timer_delay_d();
        counter--;
    }
}

void rra_lcd(char x){
    lcd_clear();
    int counter;
    for(counter = 0; counter <= 31; counter++){
        cursor_off;
        lcd_data(x);
        timer_delay_d();
        lcd_cursor_left();
        lcd_data(' ');
        if(counter == 15) lcd_new_line;
    }
    cursor_on;

}

void set_delay(int d){
    d_delay = d;
}

void clear_all_leds(){
    lcd_clear();
}

void stepper_deg(int p){
    int counter = 0;
    while(abs(motor_angle-p) > step_size){
       rotate_motor_to_angle(p);
       if(counter % 5 == 0){
           floatToStrWithPrefix(motor_angle_LCD, motor_angle, 2);
           send_str_to_PC(motor_angle_LCD);
       }
       counter++;
       timer_delay_HZ(50);
    }
    floatToStrWithPrefix(motor_angle_LCD, motor_angle, 2);
    send_str_to_PC(motor_angle_LCD);
}

void stepper_scan(int l, int r){
    lcd_clear();
    while(abs(motor_angle-l) > step_size){
       rotate_motor_to_angle(l);
       timer_delay_HZ(50);
    }
    lcd_puts("1st Angle:");
    floatToStr(motor_angle_LCD, motor_angle, 2);
    lcd_puts(motor_angle_LCD);

    while(abs(motor_angle-r) > step_size){
       rotate_motor_to_angle(r);
       timer_delay_HZ(50);
    }
    lcd_new_line;
    lcd_puts("2st Angle:");
    floatToStr(motor_angle_LCD, motor_angle, 2);
    lcd_puts(motor_angle_LCD);
}

void sleep(){
    __bis_SR_register(LPM0_bits);
}

int Hex_str_to_int(const char *hex_str) {
    return (int)strtol(hex_str, NULL, 16);
}

void script_burn_file(){
    FCTL2 = FWKEY + FSSEL0 + FN1;
    if (folder.num_of_files <= 3)folder.num_of_files ++;
    folder.file_size[file_Seg] = strlen(file_from_PC)-1;   
    write_Seg();
}

void script_execute_file(){

    char *file_pointer;
    int file_row;
    char OP_CODE[3] = {'\0'};
    char OPERAND_1_HEX_str[3] = {'\0'};
    char OPERAND_2_HEX_str[3] = {'\0'};
    char character;
    int OPERAND_1_int = 0;
    int OPERAND_2_int = 0;
    file_pointer = folder.file_ptr[file_Seg];

    for(file_row = 0; file_row < 10; file_row++){ //file max rows = 10
        OP_CODE[0] = *file_pointer++;   // fetch opcode from row
        OP_CODE[1] = *file_pointer++;
        switch(OP_CODE[1]){            // fetch operands from row
            case '1':                  // inc LCD
                OPERAND_1_HEX_str[0] = *file_pointer++;
                OPERAND_1_HEX_str[1] = *file_pointer++;
                OPERAND_1_int = Hex_str_to_int(OPERAND_1_HEX_str);
                inc_lcd(OPERAND_1_int);
                break;

            case '2':                  // dec LCD
                OPERAND_1_HEX_str[0] = *file_pointer++;
                OPERAND_1_HEX_str[1] = *file_pointer++;
                OPERAND_1_int = Hex_str_to_int(OPERAND_1_HEX_str);
                dec_lcd(OPERAND_1_int);
                break;

            case '3':                  // rra LCD
                OPERAND_1_HEX_str[0] = *file_pointer++;
                OPERAND_1_HEX_str[1] = *file_pointer++;
                OPERAND_1_int = Hex_str_to_int(OPERAND_1_HEX_str);
                character = (char)OPERAND_1_int;
                rra_lcd(character);
                break;

            case '4':                  // set delay - d
                OPERAND_1_HEX_str[0] = *file_pointer++;
                OPERAND_1_HEX_str[1] = *file_pointer++;
                OPERAND_1_int = Hex_str_to_int(OPERAND_1_HEX_str);
                set_delay((char)OPERAND_1_int);
                break;

            case '5':                 // clear LCD
                lcd_clear();
                break;

            case '6':
                OPERAND_1_HEX_str[0] = *file_pointer++;
                OPERAND_1_HEX_str[1] = *file_pointer++;
                OPERAND_1_int = Hex_str_to_int(OPERAND_1_HEX_str);
                stepper_deg(OPERAND_1_int);
                break;

            case '7':
                OPERAND_1_HEX_str[0] = *file_pointer++;
                OPERAND_1_HEX_str[1] = *file_pointer++;
                OPERAND_2_HEX_str[0] = *file_pointer++;
                OPERAND_2_HEX_str[1] = *file_pointer++;
                OPERAND_1_int = Hex_str_to_int(OPERAND_1_HEX_str);
                OPERAND_2_int = Hex_str_to_int(OPERAND_2_HEX_str);
                stepper_scan(OPERAND_1_int, OPERAND_2_int);
                break;

            case '8':
                break;
        }
    }
}

//******************************************************************
// Memory FUNCTIONS
//******************************************************************
void write_Seg(){ // 3 segments - 0 ,1 ,2

  char *Flash_ptr;                          // Flash pointer
  unsigned int i;
//   Flash_ptr = (char *)(0xFF00 + ((Seg-1)*0x40));      // Initialize Flash pointer
  Flash_ptr = (char *)(0x1000 + ((file_Seg)*0x40));      // Initialize Flash pointer
  folder.file_ptr[file_Seg] = Flash_ptr;        // update pointers array
  if (file_Seg == 0){
    read_calib_data_from_flash();
  }
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
 *Flash_ptr = 0;                   // Dummy write to erase Flash segment

  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  for (i = 0; i < folder.file_size[file_Seg]; i++){
      if (file_from_PC[i] == 0x0A){
                  continue;
      }
      *Flash_ptr++ = file_from_PC[i];            // Write value to flash
  }
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;
  if(file_Seg == 0){
    write_step_size();
  }
  memset(file_from_PC,0,strlen(file_from_PC));
}


void write_step_size() {
    unsigned int i=0;
    char *Flash_ptr;
    char *Flash_ptr1;
    char *Flash_ptr2;
    Flash_ptr = (char *)SEG_FLASH_START_ADDRESS;
    Flash_ptr1 = (char *)CALIB_COUNTER_FLASH_ADDRESS;
    Flash_ptr2 = (char *)STEP_SIZE_FLASH_ADDRESS;
    int length_temp;
    int length1 = strlen(temp_calib_counter);
    int length2 = strlen(temp_step_size);

    // to not overrun the file in the flash segment (if exists)
    while (Flash_ptr < Flash_ptr1 - 1) {
        temp_buffer[i++] = *Flash_ptr++;
    }
    temp_buffer[i] = '\0';
    length_temp = strlen(temp_buffer);
    Flash_ptr = (char *)SEG_FLASH_START_ADDRESS;

    FCTL1 = FWKEY + ERASE;        // Set erase mode
    FCTL3 = FWKEY;                // Clear lock bit
    *Flash_ptr = 0;
    FCTL1 = FWKEY + WRT;           // Set write mode

    for(i = 0; i < length_temp; i++){
        *Flash_ptr++ = temp_buffer[i];
    }
//   Write step_size to flash
    for(i = 0; i < length1; i++){
        *Flash_ptr1 = temp_calib_counter[i];
        Flash_ptr1++;
    }

    for(i = 0; i < length2; i++){
        *Flash_ptr2 = temp_step_size[i];
        Flash_ptr2++;
    }
    memset(temp_buffer,0,strlen(temp_buffer));
  // Lock flash memory
    FCTL1 = FWKEY;                 // Clear write mode
    FCTL3 = FWKEY + LOCK;          // Set lock bit
}

void read_calib_data_from_flash() {
    int temp;
    char *Flash_ptr1 = (char *)CALIB_COUNTER_FLASH_ADDRESS; // Start address where the float string is stored
    char *Flash_ptr2 = (char *)STEP_SIZE_FLASH_ADDRESS;
    unsigned int i;

    for (i = 0; i < sizeof(temp_calib_counter) - 1; i++) {
        temp_calib_counter[i] = *Flash_ptr1++;
        // Stop reading if we hit the end of the string
        if (temp_calib_counter[i] == '\0') {
            break;
        }
    }
    temp_step_size[i] = '\0'; // Ensure null-termination    
    // Read the string from flash memory
    for (i = 0; i < sizeof(temp_step_size) - 1; i++) {
        temp_step_size[i] = *Flash_ptr2++;
        // Stop reading if we hit the end of the string
        if (temp_step_size[i] == '\0') {
            break;
        }
    }
    temp_step_size[i] = '\0'; // Ensure null-termination
    temp = atoi(temp_calib_counter);
    step_size = 360.0/temp;
}

//******************************************************************
// UART FUNCTIONS
//******************************************************************
void EnableUart(void){
    IE2 |= UCA0RXIE;
}

void send_stage_to_PC(unsigned char stage){
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = stage;
    IE2 |= UCA0TXIE;
}

void send_str_to_PC(char* str_to_send){
    Tx_idx = 0;
    UCA0TXBUF = str_to_send[Tx_idx++];
    IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
//    __bis_SR_register(LPM0_bits + GIE); // Sleep
    timer_delay_HZ(50);
}
//******************************************************************
// send a command to the LCD
//******************************************************************
void lcd_cmd(unsigned char c)
{
    LCD_WAIT;  // May check LCD busy flag, or just delay a little, depending on
               // lcd.h

    if (LCD_MODE == FOURBIT_MODE)
    {
        LCD_DATA_WRITE &= ~OUTPUT_DATA;  // Clear bits before new write
        LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET; // Write upper nibble
        lcd_strobe();                          // Strobe to latch data
        LCD_DATA_WRITE &= ~OUTPUT_DATA;        // Clear bits before new write
        LCD_DATA_WRITE |= (c & (0x0F)) << LCD_DATA_OFFSET; // Write lower nibble
        lcd_strobe();                          // Strobe to latch data
    }
    else
    {
        LCD_DATA_WRITE = c;  // Write command directly in 8-bit mode
        lcd_strobe();        // Strobe to latch data
    }
}

//******************************************************************
// send data to the LCD
//******************************************************************
void lcd_data(unsigned char c)
{
    LCD_WAIT;  // May check LCD busy flag, or just delay a little, depending on
               // lcd.h

    LCD_DATA_WRITE &= ~OUTPUT_DATA;  // Clear data bits
    LCD_RS(1);                       // Set RS for data

    if (LCD_MODE == FOURBIT_MODE)
    {
        LCD_DATA_WRITE &= ~OUTPUT_DATA;  // Clear bits before new write
        LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET; // Write upper nibble
        lcd_strobe();                          // Strobe to latch data
        LCD_DATA_WRITE &= (0xF0 << LCD_DATA_OFFSET)
                | (0xF0 >> 8 - LCD_DATA_OFFSET);  // Mask lower nibble
        LCD_DATA_WRITE &= ~OUTPUT_DATA;  // Clear bits before new write
        LCD_DATA_WRITE |= (c & 0x0F) << LCD_DATA_OFFSET;  // Write lower nibble
        lcd_strobe();  // Strobe to latch data
    }
    else
    {
        LCD_DATA_WRITE = c;  // Write data directly in 8-bit mode
        lcd_strobe();        // Strobe to latch data
    }

    LCD_RS(0);  // Clear RS after data
}

//******************************************************************
// write a string of chars to the LCD
//******************************************************************
void lcd_puts(const char *s)
{
    while (*s)           // Loop through string
        lcd_data(*s++);  // Send each character
}

//******************************************************************
// initialize the LCD
//******************************************************************
void lcd_init()
{
    char init_value;  // Declare initialization value

    if (LCD_MODE == FOURBIT_MODE)
        init_value = 0x3 << LCD_DATA_OFFSET;  // Set 4-bit mode
    else
        init_value = 0x3F;  // Set 8-bit mode

    LCD_RS_DIR(OUTPUT_PIN);       // Set RS direction
    LCD_EN_DIR(OUTPUT_PIN);       // Set EN direction
    LCD_RW_DIR(OUTPUT_PIN);       // Set RW direction
    LCD_DATA_DIR |= OUTPUT_DATA;  // Set data direction
    LCD_RS(0);                    // Clear RS
    LCD_EN(0);                    // Clear EN
    LCD_RW(0);                    // Clear RW

    DelayMs(15);                     // Delay 15 ms
    LCD_DATA_WRITE &= ~OUTPUT_DATA;  // Clear data bits
    LCD_DATA_WRITE |= init_value;    // Set initialization value
    lcd_strobe();                    // Strobe to latch data
    DelayMs(5);                      // Delay 5 ms
    LCD_DATA_WRITE &= ~OUTPUT_DATA;  // Clear data bits
    LCD_DATA_WRITE |= init_value;    // Set initialization value
    lcd_strobe();                    // Strobe to latch data
    DelayUs(200);                    // Delay 200 us
    LCD_DATA_WRITE &= ~OUTPUT_DATA;  // Clear data bits
    LCD_DATA_WRITE |= init_value;    // Set initialization value
    lcd_strobe();                    // Strobe to latch data

    if (LCD_MODE == FOURBIT_MODE)
    {
        LCD_WAIT;  // May check LCD busy flag, or just delay a little, depending
                   // on lcd.h
        LCD_DATA_WRITE &= ~OUTPUT_DATA;            // Clear data bits
        LCD_DATA_WRITE |= 0x2 << LCD_DATA_OFFSET;  // Set 4-bit mode
        lcd_strobe();                              // Strobe to latch data
        lcd_cmd(0x28);                             // Function set
    }
    else
    {
        lcd_cmd(0x3C);  // Set 8-bit, two lines, 5x10 dots
    }

    lcd_cmd(0xF);   // Display on, cursor on, cursor blink
    lcd_cmd(0x1);   // Display clear
    lcd_cmd(0x6);   // Entry mode
    lcd_cmd(0x80);  // Initialize DDRAM address to zero
}

//******************************************************************
// lcd strobe functions
//******************************************************************
void lcd_strobe()
{
    LCD_EN(1);          // Set EN
    __delay_cycles(2);  // Delay for 2 cycles
    LCD_EN(0);          // Clear EN
}

//---------------------------------------------------------------------
//            enter from LPM0 mode
//---------------------------------------------------------------------
void enterLPM(unsigned char LPM_level)
{
    if (LPM_level == 0x00)
        _BIS_SR(LPM0_bits);  // Enter low power mode 0
    else if (LPM_level == 0x01)
        _BIS_SR(LPM1_bits);  // Enter low power mode 1
    else if (LPM_level == 0x02)
        _BIS_SR(LPM2_bits);  // Enter low power mode 2
    else if (LPM_level == 0x03)
        _BIS_SR(LPM3_bits);  // Enter low power mode 3
    else if (LPM_level == 0x04)
        _BIS_SR(LPM4_bits);  // Enter low power mode 4
}

//*********************************************************************
//            JS PB - PORT 1 interrupt service routine
//*********************************************************************
#pragma vector = PORT1_VECTOR
__interrupt void JS_handler(void)
{
    delay(0xFFFF);  // Debounce delay
    //---------------------------------------------------------------------
    //            selector of transition between states
    //---------------------------------------------------------------------
        if(state == state2){
            JS_PB_IFG = 1;
            send_stage_to_PC('^'); //to change between neutral, pen and erase.
        }

        else if (state == state3){
            if (state3_stage == '0'){
                state3_stage = '1';
            }
            else if(state3_stage == '1'){
                state3_stage = '2';
                calib_counter = 0;
            }
            else if(state3_stage == '2'){
                state3_stage = '3';
                step_size = 360.0/calib_counter;
            }
        }

    Joys_PB_INTPEND &= ~BIT0;  // Clear interrupt flag
    //---------------------------------------------------------------------
    //            exit from a given LPM
    //---------------------------------------------------------------------
    switch (lpm_mode)
    {
    case mode0:
        LPM0_EXIT;  // Must be called from ISR only
        break;
    case mode1:
        LPM1_EXIT;  // Must be called from ISR only
        break;
    case mode2:
        LPM2_EXIT;  // Must be called from ISR only
        break;
    case mode3:
        LPM3_EXIT;  // Must be called from ISR only
        break;
    case mode4:
        LPM4_EXIT;  // Must be called from ISR only
        break;
    }
}

//*********************************************************************
//            timers interrupt service routine
//*********************************************************************

#pragma vector = TIMER0_A0_VECTOR // For delay
__interrupt void TIMER0_A0_ISR(void)
{
    __bic_SR_register_on_exit(LPM0_bits);
    TACTL &= ~MC_1;
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
        __bic_SR_register_on_exit(LPM0_bits);
}
//*********************************************************************
//            UART - INTERUPT RUTINES
//*********************************************************************


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCI0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    // Handle TX interrupt based on current state
    if(state == state0){
            IE2 &= ~UCA0TXIE;
        }
    else if(state == state2){
        if(JS_PB_IFG == 1){
            IE2 &= ~UCA0TXIE;
        }
        else{
            if(Tx_idx < sizeof(JS_angle_LCD)-1){
                UCA0TXBUF = JS_angle_LCD[Tx_idx++];
            }else{
                Tx_idx = 0;
                IE2 &= ~UCA0TXIE;
            }
        }
        JS_PB_IFG = 0;
    }
    else if(state == state3){
        if(state3_stage == '1') IE2 &= ~UCA0TXIE;
        else if(state3_stage == '2') IE2 &= ~UCA0TXIE;
        else if(state3_stage == '3'){
                if(Tx_idx < sizeof(calib_counter_LCD)-1){
                    UCA0TXBUF = calib_counter_LCD[Tx_idx++];
                }else{
                    Tx_idx = 0;
                    IE2 &= ~UCA0TXIE;
                }
        }
    }
    else if(state == state4){
        if(Script_burn_or_execute_choise == 0)IE2 &= ~UCA0TXIE;
        else if(Script_burn_or_execute_choise == 2){
            if(Tx_idx < sizeof(motor_angle_LCD)-1){
                UCA0TXBUF = motor_angle_LCD[Tx_idx++];
            }else{
                Tx_idx = 0;
                IE2 &= ~UCA0TXIE;
            }
        }
    }

}

//-------------------------------------------------------------
//                UART RX ISR - Handle received data
//-------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    // Process received characters and set states accordingly
      if(UCA0RXBUF == ')' ){
        state = state0;
      }
      else if (UCA0RXBUF == '!' ) {  // motor with JS control
        state = state1;
//            IE2 |= UCA0TXIE;  // Enable TX interrupt
    } else if (UCA0RXBUF == '@') {  // JS Painter
        state = state2;
//        IE2 |= UCA0TXIE;  // Enable TX interrupt
    } else if (UCA0RXBUF == '#') {  // Motor calibration
        state = state3;
//        IE2 |= UCA0TXIE;  // Enable TX interrupt
    } else if (UCA0RXBUF == '$') {   // Script mode
        state = state4;
//        IE2 |= UCA0TXIE;
    }
     else if(UCA0RXBUF == '%'){
          if (state == state1){
               if(state1_stage == '0'){
                   state1_stage = '1';
               }
               else if(state1_stage == '1'){
                   state1_stage = '2';
               }
            }

     }
     else if(state == state4){

        if(UCA0RXBUF == 'G' && state == state4){     // Recieve & Burn file 1
                Script_burn_or_execute_choise = 1;
                file_Seg = 0;
                Rx_idx = 0;
         }
         else if(UCA0RXBUF == 'H' && state == state4){     // Execute file 1
            Script_burn_or_execute_choise = 2;
            file_Seg = 0;
         }
         else if(UCA0RXBUF == 'J' && state == state4){     // Recieve & Burn file 2
            Script_burn_or_execute_choise = 1;
            file_Seg = 1;
            Rx_idx = 0;
         }
         else if(UCA0RXBUF == 'K' && state == state4){     // Execute file 2
            Script_burn_or_execute_choise = 2;
            file_Seg = 1;
         }
         else if(UCA0RXBUF == 'L' && state == state4){     // Recieve & Burn file 3
            Script_burn_or_execute_choise = 1;
            file_Seg = 2;
            Rx_idx = 0;
         }
         else if(UCA0RXBUF == 'Q' && state == state4){     // Execute file 3
            Script_burn_or_execute_choise = 2;
            file_Seg = 2;
         }
         else{
             file_from_PC[Rx_idx++] = UCA0RXBUF;
         }
     }

    // Exit low power mode based on lpm_mode
    switch (lpm_mode) {
        case mode0:
            LPM0_EXIT;  // Exit LPM0
            break;
        case mode1:
            LPM1_EXIT;  // Exit LPM1
            break;
        case mode2:
            LPM2_EXIT;  // Exit LPM2
            break;
        case mode3:
            LPM3_EXIT;  // Exit LPM3
            break;
        case mode4:
            LPM4_EXIT;  // Exit LPM4
            break;
    }
}
