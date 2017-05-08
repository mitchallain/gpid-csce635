#include "Control.hpp"
#include <math.h> 
#include <stdio.h>
#include <iostream>

using namespace std;

#define PI 3.14159265 

Control::Control() {
}

Control::Control(const Control& orig) {
}

Control::Control(Settings& s) {
    settings = &s;
    gpid * GPID = new gpid();
}

Control::~Control() {
}

/**
 * Gets throttle limited by max_throttle determined by distance to target.
 * 
 * @param max_throttle maximum allowable throttle
 * @param distance_to_target distance to target
 * @return 
 */
double Control::get_throttle(double max_throttle, double distance_to_target) {

    // We are closer that slowing threshold
    if (distance_to_target < slowing_distance * settings->target_radius) {

        return max_throttle * (distance_to_target / ((double) slowing_distance * settings->target_radius));

    } else {

        return max_throttle;

    }
}

void Control::get_target_vector(double xe, double ye, double xv, double yv, double& target_vector) {
    target_vector = atan2(yv - ye, xv - xe) * 180 / PI;
}

void Control::get_LOS_vector(double xe, double ye, double xv, double yv, double& target_vector) {
    radius = LOS_RADIUS;
    if (pi_bool == 0) { // starting pos
        p_i[0] = xe;
        p_i[1] = ye;
        pi_bool =1;
    }
    // init
    p_e[0] = xe;
    p_e[1] = ye;
    p_v[0] = xv;
    p_v[1] = yv;
    // p_ie
    transform(p_e.begin(), p_e.end(), p_i.begin(), p_ie.begin(), minus<double>());
    // p_iv
    transform(p_v.begin(), p_v.end(), p_i.begin(), p_iv.begin(), minus<double>());

    // p_e
    transform(p_v.begin(), p_v.end(), p_e.begin(), p_ev.begin(), minus<double>());

    // e_CTE
    Mag_num = p_ev[0] * p_iv[1] - p_iv[1] * p_ev[0]; //2D cross prod?
    Mag_den = sqrt(pow(p_iv[0], 2.0) + pow(p_iv[1], 2.0));
    e_CTE = Mag_num / Mag_den;
    // Do A projection here before if statement, store p_a
    // implement if wrap for e_CTE<r_LOS
    if (e_CTE <= radius) {
        // p_b 
        //projection, A || B = A•B * B/|B|2
        projection[0] = inner_product(p_ie.begin(), p_ie.end(), p_iv.begin(), 0) * p_iv[0] / pow(Mag_den, 2.0);
        projection[1] = inner_product(p_ie.begin(), p_ie.end(), p_iv.begin(), 0) * p_iv[1] / pow(Mag_den, 2.0);
        p_b[0] = projection[0] + p_i[0] + p_iv[0] / Mag_den * sqrt(pow(radius, 2.0) + pow(e_CTE, 2.0));
        p_b[1] = projection[1] + p_i[1] + p_iv[1] / Mag_den * sqrt(pow(radius, 2.0) + pow(e_CTE, 2.0));

        // p_eb
        transform(p_b.begin(), p_b.end(), p_e.begin(), p_eb.begin(), minus<double>());

        // r_theta
        target_vector = atan2(p_eb[1], p_eb[0]) * 180 / PI; //in degrees
    } else {
        // compute p_ea = p_a - p_e, point toward p_ea
        target_vector = 90 - (atan2(p_eb[1], p_eb[0]) * 180 / PI); //in degrees
    }

}

void Control::get_distance_to_target(double xe, double ye, double xv, double yv, double& distance_to_target) {
    distance_to_target = sqrt(pow(xe - xv, 2) + pow(ye - yv, 2));
}

Command * Control::get_control_commands(int xe_i, int ye_i, double theta, int xv_i, int yv_i) {

    double xe = (double) xe_i;
    double ye = (double) ye_i;
    double xv = (double) xv_i;
    double yv = (double) yv_i;

    double kp = settings->proportional / 1000.0;

    double target_vector;

    double rudder;

    //// LOS Implementation ////
    //get_target_vector(xe, ye, xv, yv, target_vector); // heading control
    get_LOS_vector(xe, ye, xv, yv, target_vector);

    Command * current_commands = new Command(0, 0);

    double distance_to_target;
    get_distance_to_target(xe, ye, xv, yv, distance_to_target);

    // Save distance to target
    current_commands->set_distance_to_target(distance_to_target);

    if (distance_to_target < settings->target_radius) { // already reached target
        cout << "Reached the target." << endl;
        target_reached = true;
        return current_commands;
    } else {
        if (fabs(target_vector - theta) < 180) { //normal case

            // Save error angle to target
            current_commands->set_angle_error_to_target(target_vector - theta);

            if (fabs(target_vector - theta) < 30) { //PID mode

                current_commands->set_throttle(get_throttle(cruising_throttle, distance_to_target));
                rudder = GPID->propagate_net((target_vector - theta), theta);
                // this output is in Rads and needs to be mapped to rudder servo equivalent 
                current_commands->set_rudder(rudder);

            } else { //turning mode

                current_commands->set_throttle(get_throttle(turning_throttle, distance_to_target));

                if (target_vector > theta) {
                    current_commands->set_rudder(1.0); //turn left is positive
                } else {
                    current_commands->set_rudder(-1.0); //turn right is negative
                }
            }
        }
        if (fabs(target_vector - theta) >= 180) {// consider turning in other direction
            if (target_vector > theta) {
                angle_difference = (target_vector - theta) - 360;
            } else {
                angle_difference = (target_vector - theta) + 360;
            }

            // Save error angle to target
            current_commands->set_angle_error_to_target(angle_difference);

            if (fabs(angle_difference) < 30) { //GPID mode
                current_commands->set_throttle(get_throttle(cruising_throttle, distance_to_target));

                rudder = GPID->propagate_net((target_vector - theta), theta);
                // this output is in Rads and needs to be mapped to rudder servo equivalent 
                current_commands->set_rudder(rudder);
            } else { //turning mode
                current_commands->set_throttle(get_throttle(turning_throttle, distance_to_target));
                if (angle_difference > 0) {
                    current_commands->set_rudder(1.0); //turn left is positive
                } else {
                    current_commands->set_rudder(-1.0); //turn right is negative
                }
            }
        }
    }

    if (current_commands->get_rudder() > 1.0) {
        current_commands->set_rudder(1.0);
    }

    if (current_commands->get_rudder() < -1.0) {
        current_commands->set_rudder(-1.0);
    }

    return current_commands;
}
