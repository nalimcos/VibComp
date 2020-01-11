#include <stdio.h>
#include <math.h>
#include <array>

// Parameters
double dt = 0.001;
// Friction coefficient, per second
double kf = 0.98;
// Derive per-iteration speed multiplier
double kd = pow(1-kf,dt);
double V = 100;
double Kx = 400;
double Ky = 400;
// Compensation can only be perfect if Kx = Ky; use compromise
double K = (Kx+Ky)/2;
// Compensation time
double T = M_PI/sqrt(K);

// Postscript output stuff
FILE *outfile;
void file_init();
void stroke() { fprintf(outfile,"stroke\n"); }
void blue() { fprintf(outfile,"0 0 1 setrgbcolor\n"); }
void red() { fprintf(outfile,"1 0 0 setrgbcolor\n"); }
void green() { fprintf(outfile,"0 1 0 setrgbcolor\n"); }
void black() { fprintf(outfile,"0 0 0 setrgbcolor\n"); }
void gray() { fprintf(outfile,"0.5 0.5 0.5 setrgbcolor\n"); }
void color(double r, double g, double b) { fprintf(outfile, "%f %f %f setrgbcolor\n", r, g, b); }
void lineto(double x, double y) { fprintf(outfile,"%f mm %f mm lineto\n", x, y); }
void moveto(double x, double y) { fprintf(outfile,"%f mm %f mm moveto\n", x, y); }

void plot_ideal_path();
void plot_real_path();
void plot_paths(double points[][2], int n);
void plot_paths_new(double points[][2], double seg_v[], int n);
void plot_vectors();

void segment(double p1[2], double p2[2])
{
	moveto(p1[0],p1[1]);
	lineto(p2[0],p2[1]);
	stroke();
}

void plot_segments(double points[][2], int n) { for (int i = 0; i < n-1; i++) segment( points[i], points[i+1] ); }

double norm(std::array<double, 2> vec)
{
	return sqrt(pow(vec[0],2)+pow(vec[1],2));
}

double vec_cos(std::array<double, 2> vec_A, std::array<double, 2> vec_B)
{
	return (vec_A[0]*vec_B[0]+vec_A[1]*vec_B[1])/(norm(vec_A)*norm(vec_B));
}

std::array<double, 2> seg_vec(double p1[2], double p2[2])
{
	return { p2[0]-p1[0], p2[1]-p1[1] };
}

// Path
double points_orig[][2] = {
	{0,0},
	{0,100},
	{80,100},
	{160,160},
	{0,160}
};

double points_test[][2] = {
	{0,0},
	{0,91},
	{25,100},
	{90,100},
	{112,112},
	{114,114},
	{90,120},
	{0,120}
};

const int N = sizeof(points_orig)/sizeof(points_orig[0]);
double points_fixed_new [2*N-1] [2] = {};
double seg_v [2*N-2] = {};

void fill_points_fixed_new()
{
	std::array<double, 2> seg1;
	std::array<double, 2> seg2;
	std::array<double, 2> u;
	std::array<double, 2> w;
	std::array<double, 2> v;

	points_fixed_new[0][0] = points_orig[0][0];
	points_fixed_new[0][1] = points_orig[0][1];
	for (int i = 1; i < N; i++)
	{
		seg1 = seg_vec( points_orig[i-1], points_orig[i] );
		seg2 = seg_vec( points_orig[i], points_orig[i+1] );
		u[0] = seg1[0] * V / norm(seg1);
		u[1] = seg1[1] * V / norm(seg1);
		w[0] = seg2[0] * V / norm(seg2);
		w[1] = seg2[1] * V / norm(seg2);
		v[0] = ( u[0] + w[0] ) / 2;
		v[1] = ( u[1] + w[1] ) / 2;

		// Write segment speeds
		seg_v[2*i-2] = norm(u);
		seg_v[2*i-1] = norm(v);

		// Place points
		points_fixed_new[2*i-1][0] = points_orig[i][0] - u[0] / 2 * T;
		points_fixed_new[2*i-1][1] = points_orig[i][1] - u[1] / 2 * T;
		if ( i != N-1 )
		{
			points_fixed_new[2*i][0] = points_orig[i][0] + w[0] / 2 * T;
			points_fixed_new[2*i][1] = points_orig[i][1] + w[1] / 2 * T;
		} else {
			points_fixed_new[2*i][0] = points_orig[i][0];
			points_fixed_new[2*i][1] = points_orig[i][1];
		}
	}
}

int main()
{
	file_init();
	fill_points_fixed_new();

	// Target path
	color(0.5,0.5,0.5);
	plot_segments( points_orig, sizeof(points_orig)/sizeof(points_orig[0]) );
	// Without correction
	red();
	plot_paths(points_orig, sizeof(points_orig)/sizeof(points_orig[0]) );
	// Corrected segments
	color(0.8,0.8,0.8);
	plot_segments(points_fixed_new, sizeof(points_fixed_new)/sizeof(points_fixed_new[0]) );
	// Result
	color(0.5,0,0.9);
	plot_paths_new(points_fixed_new, seg_v, sizeof(points_fixed_new)/sizeof(points_fixed_new[0]) );

	return 0;
}

void file_init()
{
	// Prepare postscript output
	outfile = fopen("output.ps", "w");
	fprintf(outfile, "\%!\n");
	fprintf(outfile, "/mm {360 mul 127 div} def\n");
	fprintf(outfile, "10 mm 10 mm translate\n");
}

void plot_paths(double points[][2], int n)
{
	double nozzle_p[] = { points[0][0], points[0][1] };
	double nozzle_v[] = { 0, 0 };
	double nozzle_a[] = { 0, 0 };
	std::array<double, 2> seg_v = { 0, 0 };
	std::array<double, 2> seg = { 0, 0 };

	for (int i = 0; i < n-1; i++)
	{
		// Update target position difference and speed vectors
		seg = seg_vec( points[i], points[i+1] );
		seg_v = { seg[0] * V / norm(seg), seg[1] * V / norm(seg) };
		if (i == 0)
		{
			nozzle_v[0] = seg_v[0];
			nozzle_v[1] = seg_v[1];
		}

		// Compute and trace effective path
		for (double t = 0; t < norm(seg)/V; t += dt)
		{
			moveto( nozzle_p[0], nozzle_p[1] );
			nozzle_a[0] = Kx * ( points[i][0] + seg_v[0] * t - nozzle_p[0] );
			nozzle_a[1] = Ky * ( points[i][1] + seg_v[1] * t - nozzle_p[1] );
			nozzle_v[0] = nozzle_v[0] * kd + nozzle_a[0] * dt;
			nozzle_v[1] = nozzle_v[1] * kd + nozzle_a[1] * dt;
			nozzle_p[0] += nozzle_v[0] * dt;
			nozzle_p[1] += nozzle_v[1] * dt;
			lineto( nozzle_p[0], nozzle_p[1] );
			stroke();
		}
	}
}

void plot_paths_new(double points[][2], double seg_v[], int n)
{
	std::array<double, 2> seg = { 0, 0 };
	double seg_vx;
	double seg_vy;
	double nozzle_p[] = { points[0][0], points[0][1] };
	double nozzle_v[] = { 0, 0 };
	double nozzle_a[] = { 0, 0 };

	for (int i = 0; i < n-1; i++)
	{
		// Update target position difference and speed vectors
		seg = seg_vec( points[i], points[i+1] );
		seg_vx = seg[0] * seg_v[i] / norm(seg);
		seg_vy = seg[1] * seg_v[i] / norm(seg);
		if (i == 0)
		{
			nozzle_v[0] = seg_vx;
			nozzle_v[1] = seg_vy;
		}
		// Compute and trace effective path
		for (double t = 0; t < norm(seg) / seg_v[i]; t += dt)
		{
			moveto( nozzle_p[0], nozzle_p[1] );
			nozzle_a[0] = Kx * ( points[i][0] + seg_vx * t - nozzle_p[0] );
			nozzle_a[1] = Ky * ( points[i][1] + seg_vy * t - nozzle_p[1] );
			nozzle_v[0] = nozzle_v[0] * kd + nozzle_a[0] * dt;
			nozzle_v[1] = nozzle_v[1] * kd + nozzle_a[1] * dt;
			nozzle_p[0] += nozzle_v[0] * dt;
			nozzle_p[1] += nozzle_v[1] * dt;
			lineto( nozzle_p[0], nozzle_p[1] );
			stroke();
		}
	}
}
