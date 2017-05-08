/* 
 * File:   Command.hpp
 * Author: Jan Dufek
 * 
 * Contributors: Austin Burch: ajburch92@gmail.com
 * 
 * This class has been adapted to implement LOS control.
 */

#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "Command.hpp"
#include "Settings.hpp"
#include "gpid.hpp"


#define LOS_RADIUS 20.00;

using namespace std;

// Target was reached
extern bool target_reached;


class Control {
public:
    Control();
    Control(Settings&);
    Control(const Control& orig);
    virtual ~Control();

    Command * get_control_commands(int, int, double, int, int);
    

private:
    // geometric LOS vars //
    double radius;
    vector <double> p_i;
    vector <double> p_e;
    vector <double> p_v;
    vector <double> p_ie;
    vector <double> p_iv;
    vector <double> p_ev;
    double e_CTE;
    double Mag_num;
    double Mag_den;
    vector <double> p_ia;
    vector <double> p_b;
    vector <double> p_eb;
    vector <double> p_ae;
    ////////////////////////
    
    bool pi_bool = 0;

    double xi; 
    double yi;
    
    // Returns throttle based on distance to target
    double get_throttle(double, double);
    
    void get_distance_to_target(double xe, double ye, double xv, double yv, double& distance_to_target);
    
    void get_target_vector(double xe, double ye, double xv, double yv, double& target_vector);
    void get_LOS_vector(double xe, double ye, double xv, double yv, double& target_vector);

    // Turning throttle
    const double turning_throttle = 0.3;

    // Cruising throttle
    const double cruising_throttle = 0.6;
    
    // Slowing distance as multiple of target radius. When this threshold is reached, EMILY will start linearly slowing down.
    int slowing_distance = 3;

    // Angle difference
    double angle_difference;
    
    // Program settings
    Settings * settings;
    
    gpid * GPID;
   
};

#endif /* CONTROL_HPP */