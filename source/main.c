#include "../header/api.h"  // private library - API layer
#include "../header/app.h"  // private library - APP layer

enum FSMstate state;
enum SYSmode lpm_mode;

void main(void) {

    state = state0;    // start in idle state on RESET
    lpm_mode = mode0;  // start in idle state on RESET
    read_calib_data_from_flash();

    sysConfig();
    lcd_init();
    while (1) {

        switch (state) {
            case state0:
                lcd_clear();
                lcd_puts("Sleep State");
                EnableUart();
                sleep_with_interrups();

            case state1: //motor rotate with joistic
                lcd_clear();
                EnableUart();
                Motor_JS_CTL();
                break;

            case state2: //painter
                lcd_clear();
                EnableUart();
                JS_painter();
                break;

            case state3: //clabirate
                lcd_clear();
                EnableUart();
                Motor_calibration();
                break;

            case state4: //script
               lcd_clear();
               EnableUart();
               Script();
                break;
        }
    }
}
