#include <stdio.h>
#include <math.h>
#include <array>

// Parameters
// Feedrate mm/s
unsigned int v = 50;
// Start corners Tc seconds early - Tc = structure vibration period / 4
double Tc = 0.01; // Tc = 0.01 -> T = 0.04 -> f = 25 Hz
// Correction distance
double corr_l = v*Tc;
// Layer height
double layer_h = 0.15;
// Line width
double line_w = 0.4;
// Line approximate cross-sectional area
double line_a = layer_h * line_w;
// Filament diameter
double fil_d = 1.75;
// Filament cross-sectional area
double fil_a = fil_d*fil_d*M_PI/4;
// Filament printing temperature
unsigned int fil_temp = 200;
// Extrude Ek mm of filament per line mm
double Ek = line_a / fil_a;
// Printed square dimensions
double w = 50;
double h = 10;
double layers = h/layer_h;
// Start pos
double x_0 = 50;
double y_0 = 50;

// Current pos
double cur_x = 0;
double cur_y = 0;
double cur_z = 0;
double cur_e = 0;

// Gcode output stuff
FILE *outfile;
void init(); // defined further down

// Simple move, no extrusion
void g0(double x, double y)
{
	fprintf(outfile,"G0 X%.2f Y%.2f F%u\n", x, y, v*60);
	cur_x = x;
	cur_y = y;
}

// Print move; automatically extrude correct amount
void g1(double x, double y)
{
	cur_e += pow(pow(x-cur_x,2)+pow(y-cur_y,2),0.5) * Ek; // segment length * Ek
	cur_x = x;
	cur_y = y;
	fprintf(outfile,"G1 X%.2f Y%.2f E%.3f F%u\n", cur_x, cur_y, cur_e, v*60);
}

// Move to new layer during print move - no time to loose :p
void g1_layerup(double x, double y)
{
	cur_e += pow(pow(x-cur_x,2)+pow(y-cur_y,2),0.5) * Ek; // segment length * Ek
	cur_x = x;
	cur_y = y;
	cur_z += layer_h;
	fprintf(outfile,"G1 X%.2f Y%.2f Z%.3f E%.3f F%u\n", cur_x, cur_y, cur_z, cur_e, v*60);
}


int main()
{
	init();
	g0(x_0, y_0);
	while (cur_z < h)
	{
		fprintf(outfile, "\n; Start layer Z=%.2f\n", cur_z);
		g1(x_0 + corr_l, y_0);
		g1(x_0 + w - corr_l, y_0);
		g1(x_0, y_0 + corr_l);
		g1(x_0, y_0 + w - corr_l);
		g1(x_0 + w - corr_l, y_0 + w);
		g1(x_0 + corr_l, y_0 + w);
		g1(x_0, y_0 + w - corr_l);
		g1_layerup(x_0, y_0 + corr_l);
	}
	fprintf(outfile, "G0 E%f F2400;\n", cur_e - 5); // retract 5mm
	g0(0,0); // move to origin
}
void init()
{
	// Prepare gcode output
	outfile = fopen("vibcomp_test.gcode", "w");
	fprintf(outfile, "; Vibration compensation proof of concept\n");
	fprintf(outfile, "; Milan Cosnefroy, 2020\n\n");
	// Set jerk to target speed, heat hotend while homing
	fprintf(outfile, "; Printer init\nM205 X%u Y%u\nM104 S%u\nG28\nM109 S%u\n", v, v, fil_temp, fil_temp);
	fprintf(outfile, "; Prime nozzle\nG0 E6 F30\nG92 E0\n");
	fprintf(outfile, "; Move down so first move clears nozzle\nG0 Z0\n");
}
