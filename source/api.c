#include "../header/api.h"     // Include API layer header
#include "../header/halGPIO.h" // Include HAL layer header

//--------------------------------------------------------------------
//State 1 - Motor with joystick
//--------------------------------------------------------------------
void Motor_JS_CTL(){
    char state1_welcome1[] = "Joistick Motor";
    char state1_welcome2[] = "Control State";
    state1_stage = '0';
    JS_angle = 0;
    lcd_puts(state1_welcome1);
    lcd_new_line;
    lcd_puts(state1_welcome2);
    EnableUart();

    if (state == state1) sleep_with_interrups();
    EnableUart();

    while (state1_stage == '1' && state == state1){
        Rotate_clockwise(500);
    }
    lcd_clear();
    motor_angle = 0;

    while(state == state1){
        sample_JS();
        if(!( x_avg > 300 && x_avg < 700 && y_avg > 300 && y_avg < 700)){
           JS_angle = get_JS_angle();
           intToStr(JS_angle_LCD, JS_angle);
           lcd_clear();
           lcd_puts("JS angle: ");
           lcd_puts(JS_angle_LCD);
        }
        if(abs(motor_angle-JS_angle) > step_size){
           rotate_motor_to_angle(JS_angle);
           if(abs(motor_angle-JS_angle) < step_size){
               lcd_clear();
               lcd_puts("JS angle: ");
               lcd_puts(JS_angle_LCD);
               lcd_new_line;
               lcd_puts("Motor: ");
               floatToStr(motor_angle_LCD, motor_angle, 2);
               lcd_puts(motor_angle_LCD);
           }
        }

    }
}

//--------------------------------------------------------------------
//State 2 - Joystick based PC painter
//--------------------------------------------------------------------
void JS_painter(){
    state2_stage = '0';
    char state2_welcome[] = "Painter State";
    lcd_puts(state2_welcome);
    JS_angle = 0;

    while(state == state2){
        sample_JS();
        if(!( x_avg > 300 && x_avg < 700 && y_avg > 300 && y_avg < 700)){
            JS_angle = get_JS_angle();
            intToStr(JS_angle_LCD, JS_angle);
            if(state == state2) send_str_to_PC(JS_angle_LCD);
            timer_delay_HZ(15);
        }
    }
}

//--------------------------------------------------------------------
//State 3 - Motor Calibration
//--------------------------------------------------------------------
void Motor_calibration(){
    calib_counter = 0;
    state3_stage = '0';
    char state3_welcome1[] = "Motor Calibration";
    char state3_welcome2[] = "State";
    lcd_puts(state3_welcome1);
    lcd_new_line;
    lcd_puts(state3_welcome2);
    if (state == state3) sleep_with_interrups();
    if(state == state3) send_stage_to_PC(state3_stage); // send stage to Uart

    while(state3_stage == '1' && state == state3){
        Rotate_clockwise(500);
    }   
    if(state == state3) send_stage_to_PC(state3_stage); // send stage to Uart
    timer_delay_HZ(1);

    while(state3_stage == '2' && state == state3){
        Rotate_clockwise(500);
        intToStr(calib_counter_LCD, calib_counter);
        lcd_clear();
        lcd_puts(calib_counter_LCD);
    }
    intToStr(calib_counter_LCD, calib_counter);
    lcd_clear();
    lcd_puts(calib_counter_LCD);
    if(state == state3) send_str_to_PC(calib_counter_LCD);
    floatToStr(temp_step_size, step_size, 2);
    intToStr(temp_calib_counter, calib_counter);
    FCTL2 = FWKEY + FSSEL0 + FN1;
    write_step_size();
    timer_delay_HZ(1);
    state = state0;
}
//--------------------------------------------------------------------
//State 4 - script mode
//--------------------------------------------------------------------
void Script(){
    char MSP_ACK = '&';
    while(state == state4){
        EnableUart();
        if (state == state4) sleep_with_interrups();
        if(Script_burn_or_execute_choise == 1 && state == state4){ // if Script -> burn
            FCTL2 = FWKEY + FSSEL0 + FN1;
            script_burn_file(file_Seg);
            Script_burn_or_execute_choise = 0;
            send_stage_to_PC(MSP_ACK);
        } else if(Script_burn_or_execute_choise == 2 && state == state4){ // if Script -> Execute
             script_execute_file(file_Seg);
             Script_burn_or_execute_choise = 0;
             send_stage_to_PC(MSP_ACK);
        }
    }
}
