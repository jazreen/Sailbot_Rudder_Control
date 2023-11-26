import.necessary.libraries

void pid() {
    float ki = 0.075;
    float kp = 0.011;
    float kd = 1.022;

    int ADC_max = 255; //100% duty cycle in digital form
    int ADC_min = -255;

    int P_term, I_term, D_term, I_temp = 0, D_temp = 0;
    int error, adjusted_error;
    int PWM_value;
    int PWM_temp = 100; //random initial value to be changed 
    int small_angle = 2; //some small angle we think is reasonable


    int error_max = 100;
    int error_min = -100;

    float current_ADC = read_from_eCompass_ADC();  //get reading from eCompass ADC (in digital form)
    error = get_desired_from_SOFT() - current_ADC; //get desired heading from SOFT
    //both headings are relative to true north

    //sign of error determines whether we turn starboard(right) or port(left)
    error_adjusted = error;

    //dead zone case (small angle error shouldn't lead to pwm change) --> avoids small constant corrections
    if (abs(error) < small_angle) {
        error = 0; //does that make sense?
        adjusted_error = 0;
    }

    //adjusting error values 
    if (error > 180) {adjusted_error = error - 360;} //so it now moves port not stbd, though error is +
    if (error < -180) {adjusted_error = error + 360;} //so it now moves stbd nor port, though error is -

    P_term = kp * |adjusted_error|; //calculate p term

    //make sure we dont have integral windup (for when we have sudden change in current/desired heading)
    //since we'll be using error polarity to determine which way to move, does it make sense to not change error var
    //directly? we can use a different param just for calculations? i changed eror to adjusted_error here

    if (adjusted_error >= error_max) {
        adjusted_error = error_max;
    }

    else if (adjusted_error <= error_min) {
        adjusted_error = error_min;
    }
    //do we need to prevent wind ups with P or D ?
    
    //does it make sense to move the following line after the if statements? It was above them before
    I_temp += |adjusted_error|; //accumulated error over time, used for Pi term calculation

    I_term = ki * I_temp; //calculate i term

    //calculate d term from the differential between previous error and current error
    D_term = kd * (D_temp - |adjusted_error|);
    D_temp = |adjusted_error|;

    //calculate PWM output value based on previous PWM value 
    PWM_value = PWM_temp - (P_term + I_term + D_term);

    //PWM overvoltage protection
    if (PWM_value > ADC_max) {
        PWM_value = ADC_max;
    }

    else if (PWM_value < ADC_min) {
        PWM_value = ADC_min;
    }

//des_heading > curr_heading → error > 0 → increase curr_heading to reduce error --> Move cw i.e stbd
    if (adjusted_error > 0) {
        move_stbd(PWM_value);
    }

//des_heading < curr_heading → error < 0 → decrease curr_heading to increase error → move ccw i.e. move port
    if (adjusted_error < 0) {
            move_port(PWM_value);
    }

    //set current value to temp
    PWM_temp = PWM_value;

    delay(500000); 
    //wait this much time before redoing the whole loop, we need to match it to sampling rate of e-compass
}
