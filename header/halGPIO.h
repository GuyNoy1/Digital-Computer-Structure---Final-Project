#ifndef _halGPIO_H_
#define _halGPIO_H_

#include  "../header/app.h"    		// private library - APP layer
#include "../header/bsp.h" //lab kit

extern enum FSMstate state;               // global variable
extern enum SYSmode lpm_mode;             // global variable

extern int d_delay;
extern unsigned char state1_stage;
extern unsigned char state2_stage;
extern unsigned char state3_stage;
extern unsigned int calib_counter;
extern float step_size;
extern float* step_size_ptr;
extern char step_size_LCD[];
extern char calib_counter_LCD[];
extern char JS_angle_LCD[5];
extern int sample_index;
extern int adc_samples[];
extern int y_avg;
extern int x_avg;
extern int JS_angle;
extern float motor_angle;
extern char motor_angle_LCD[7];
extern int Tx_idx;
extern int Rx_idx;
extern int JS_PB_IFG;
extern int Script_burn_or_execute_choise;
extern int Transmitter_idx;
extern char file_from_PC[80];
extern char temp_buffer[64];
extern char temp_step_size[5];
extern char temp_calib_counter[5];
extern int file_Seg;

typedef struct {
  unsigned int num_of_files;
  char *file_names[10];  // Array of sizes of each record
  char *file_ptr[3];     // Array of pointers to memory segments
  unsigned int file_size[3];   
} Folder;

extern Folder folder;

extern void write_Seg();
extern void write_step_size();
extern void read_calib_data_from_flash();
extern int Hex_str_to_int(const char *hex_str);

extern void sysConfig(void);
extern void sleep_with_interrups(void);
extern void delay(unsigned int);
extern void enterLPM(unsigned char);
extern void enable_interrupts();
extern void disable_interrupts();
extern void DelayMs(unsigned int cnt);
extern void DelayUs(unsigned int cnt);
extern void timer_delay_HZ(int freq);
extern void EnableUart(void);

extern void Rotate_clockwise(int rotate_freq);
extern void Rotate_counter_clockwise(int rotate_freq);
extern void sample_JS();
extern float simple_atan_degrees(float z);
extern int simple_atan2(int y, int x);
extern int get_JS_angle();
extern void rotate_motor_to_angle(int desired_angle);
extern void send_stage_to_PC(unsigned char stage);
extern void send_str_to_PC(char* str_to_send);
extern void script_burn_file();
extern void script_execute_file();

extern void intToStr(char *str, unsigned int num);
extern void floatToStr(char *str, float num, int precision);
extern void floatToStrWithPrefix(char *str, float num, int precision);
extern void timer_delay_d();
extern void inc_lcd(int x);
extern void dec_lcd(int x);
extern void rra_lcd(char x);
extern void set_delay(int d);
extern void clear_all_leds();
extern void stepper_deg(int p);
extern void stepper_scan(int l, int r);
extern void sleep();

extern __interrupt void JS_handler(void);
extern __interrupt void TIMER0_A0_ISR(void);
extern __interrupt void Timer1_A0_ISR(void);
extern __interrupt void ADC10_ISR(void);
extern __interrupt void USCI0TX_ISR(void);
extern __interrupt void USCI0RX_ISR(void);

#ifdef CHECKBUSY
    #define LCD_WAIT lcd_check_busy()
#else
    #define LCD_WAIT DelayMs(5)
#endif

/*----------------------------------------------------------
  CONFIG: change values according to your port pin selection
------------------------------------------------------------*/
#define LCD_EN(a)   (!a ? (P1OUT&=~0X20) : (P1OUT|=0X20)) // P2.5 is lcd enable pin
#define LCD_EN_DIR(a)   (!a ? (P1DIR&=~0X20) : (P1DIR|=0X20)) // P2.5 pin direction

#define LCD_RS(a)   (!a ? (P1OUT&=~0X40) : (P1OUT|=0X40)) // P2.6 is lcd RS pin
#define LCD_RS_DIR(a)   (!a ? (P1DIR&=~0X40) : (P1DIR|=0X40)) // P2.6 pin direction

#define LCD_RW(a)   (!a ? (P1OUT&=~0X80) : (P1OUT|=0X80)) // P2.7 is lcd RW pin
#define LCD_RW_DIR(a)   (!a ? (P1DIR&=~0X80) : (P1DIR|=0X80)) // P2.7 pin direction

#define LCD_DATA_OFFSET 0x04 //data pin selection offset for 4 bit mode, variable range is 0-4, default 0 - Px.0-3, no offset

#define FOURBIT_MODE    0x0
#define EIGHTBIT_MODE   0x1
#define LCD_MODE        FOURBIT_MODE

#define OUTPUT_PIN      1
#define INPUT_PIN       0
#define OUTPUT_DATA     (LCD_MODE ? 0xFF : (0x0F << LCD_DATA_OFFSET))
#define INPUT_DATA      0x00

//#define LCD_STROBE_READ(value)  LCD_EN(1), \
//                asm("nop"), asm("nop"), \
//                value=LCD_DATA_READ, \
//                LCD_EN(0)

#define lcd_cursor(x)       lcd_cmd(((x)&0x7F)|0x80)
#define lcd_clear()         lcd_cmd(0x01)
#define lcd_putchar(x)      lcd_data(x)
#define lcd_goto(x)         lcd_cmd(0x80+(x))
#define lcd_cursor_right()  lcd_cmd(0x14)
#define lcd_cursor_left()   lcd_cmd(0x10)
#define lcd_display_shift() lcd_cmd(0x1C)
#define lcd_home()          lcd_cmd(0x02)
#define cursor_off          lcd_cmd(0x0C)
#define cursor_on           lcd_cmd(0x0F)
#define lcd_function_set    lcd_cmd(0x3C) // 8bit,two lines,5x10 dots
#define lcd_new_line        lcd_cmd(0xC0)

extern void lcd_cmd(unsigned char);
extern void lcd_data(unsigned char);
extern void lcd_puts(const char * s);
extern void lcd_init();
extern void lcd_strobe();

#endif
