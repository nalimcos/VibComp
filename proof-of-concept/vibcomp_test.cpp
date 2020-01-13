#include <stdio.h>
#include <math.h>
#include <array>

// Parameters
// Feedrate mm/s
unsigned int v = 60;
// Start corners Tc seconds early - Tc = structure vibration period / 4
//double Tc = 0.0001; // basically no correction
//double Tc = 0.006;
//double Tc = 0.008;
//double Tc = 0.007; // Tc = 0.01 -> T = 0.04 -> f = 25 Hz
// Correction distance
//double corr_l = v*Tc;
// Layer height
double layer_h = 0.2;
// Line width
double line_w = 0.4;
// Line approximate cross-sectional area
double line_a = layer_h * line_w;
// Filament diameter
double fil_d = 1.75;
// Filament cross-sectional area
double fil_a = fil_d*fil_d*M_PI/4;
// Filament printing temperature
unsigned int fil_temp = 230;
// Extrude Ek mm of filament per line mm
double Ek = line_a / fil_a;
// Printed square dimensions
double w = 40;
double h = 5; // per zone, total is *zones
int zone_i = 0;
int zone_f = 1;
double layers = h/layer_h;
// Start pos
double x_0 = 125;
double y_0 = 75;

// Current pos
double cur_x = 0;
double cur_y = 0;
double cur_z = 0;
double cur_e = 0;

// Skirt
double skirt_lines = 5;

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
	// Prepare gcode output
	outfile = fopen("vibcomp_test.gcode", "w");
	fprintf(outfile, "; Vibration compensation proof of concept\n");
	fprintf(outfile, "; Milan Cosnefroy, 2020\n\n");
	// Set jerk to target speed, heat hotend while homing
	fprintf(outfile, "; Printer init\nM205 X%u Y%u\nG28\nG29\nT1\nM109 S%u\n", v*2, v*2, fil_temp);
	fprintf(outfile, "G92 E0\n");
	fprintf(outfile, "G0 X%.2f Y%.2f\n", x_0 - 10, y_0 - 10);
	fprintf(outfile, "G0 Z0 F300\n\n");
	// Prime nozzle with skirt
	for(int i = 0; i < skirt_lines; i++)
	{
		// 1
		g1(	x_0 - 9 + i,		y_0 - 10 + i);
		// 2
		g1(	x_0 + w + 9 - i,	y_0 - 10 + i);
		g1(	x_0 + w + 10 - i,	y_0 - 9 + i);
		// 3
		g1(	x_0 + w + 10 - i,	y_0 + w + 9 - i);
		g1(	x_0 + w + 9 - i,	y_0 + w + 10 - i);
		// 4
		g1(	x_0 - 9 + i,		y_0 + w + 10 - i);
		g1(	x_0 - 10 + i,		y_0 + w + 9 - i);
		// Back to 1
		g1(	x_0 - 10 + i,		y_0 - 9 + i);
	}
	// Start actual print
	g0(x_0, y_0);
	double last_corr_l = 0;
	for (int i = zone_i; i <= zone_f; i += 1)
	{
//		double Tc = i*0.001;
		double Tc = i*0.007;
		double corr_l = v*Tc;
		while (cur_z < h*(i-zone_i+1))
		{
			fprintf(outfile, "\n; Start layer Z=%.2f\n; Tc = %.3f\n", cur_z, Tc);
			g1(x_0 + corr_l, y_0);
			g1(x_0 + w - corr_l, y_0);
			g1(x_0 + w, y_0 + corr_l);
			g1(x_0 + w, y_0 + w - corr_l);
			g1(x_0 + w - corr_l, y_0 + w);
			g1(x_0 + corr_l, y_0 + w);
			g1(x_0, y_0 + w - corr_l);
	//		g1(x_0, y_0 + w/2);
			g1_layerup(x_0, y_0 + corr_l);
		}
		last_corr_l = corr_l;
	}
	// Wipe
	g0(x_0 + last_corr_l, y_0);
	g0(x_0 + w - last_corr_l, y_0);
	g0(x_0 + w, y_0 + last_corr_l);
	g0(x_0 + w, y_0 + w - last_corr_l);
	g0(x_0 + w - last_corr_l, y_0 + w);
	g0(x_0 + last_corr_l, y_0 + w);
//	g0(x_0, y_0 + w - last_corr_l);
	g0(x_0 + w/2, y_0 + w/2);
	fprintf(outfile, "G0 E%f Z20\nG92 E0\nM104 S0\n", cur_e - 5); // retract, turn hotend off
	g0(0,0); // move to origin
}
