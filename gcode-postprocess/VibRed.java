/*
 *	G-Code vibration reduction postprocessor by Milan Cosnefroy
 *	Currently assumes infinitely high jerk and accel, constant print speed
 *	i.e. tuned for high accuracy printing at moderate speeds,
 *	rather than decent results at high speeds.
 */

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class VibRed {

	// Parameters - actually set from command-line
	static double v = 0;
	static double kx = 0;
	static double ky = 0;
	static double k1 = 0;
	static double k2 = 0;

	// Run-time variables
	static double[] p1 = {0,0,0,0};
	static double[] p2 = {0,0,0,0};
	static double[] p3 = {0,0,0,0};

	// Usage: VibRed print_speed x_rigidity y_rigidity corr_k1 corr_k2
	public static void main(String[] args) throws IOException
	{
		String inputFilename = args[0];
		double v = Double.parseDouble(args[2]);
		double kx = Double.parseDouble(args[3]);
		double ky = Double.parseDouble(args[4]);
		double k1 = Double.parseDouble(args[5]);
		double k2 = Double.parseDouble(args[6]);

		BufferedReader br = new BufferedReader(new FileReader(inputFilename));
		String line = "; Processed by Milan Cosnefroy's vibration reduction engine\n";
		while ( line != null )
		{
			parseLine(line);
			line = br.readLine();
		}
	}

	static void parseLine(String input)
	{
		String output = input.split(";")[0].replaceAll("[ 	]","").toUpperCase();
		if ( output.contains("G0") || output.contains("G1") ) {
			output = processCoordinates(output);
		} else { output = input; }
		System.out.println(output);
	}

	static double[] ptsToVec(double xa, double ya, double xb, double yb)
	{
		double[] output = { xb - xa, yb - ya };
		System.out.println(xb-xa);
		System.out.println(output[0] + " " + output[1]);
		return output;
	}

	static String processCoordinates(String input)
	{
		p1 = p2;
		p2 = p3;
		if ( input.contains("X") ) p3[0] = Double.parseDouble(input.substring(input.indexOf("X")+1).split("[GXYZE]")[0]);
//		else p3[0] = p2[0]; // already true
		if ( input.contains("Y") ) p3[1] = Double.parseDouble(input.substring(input.indexOf("Y")+1).split("[GXYZE]")[0]);
//		else p3[1] = p2[1];
		if ( input.contains("Z") ) p3[2] = Double.parseDouble(input.substring(input.indexOf("Z")+1).split("[GXYZE]")[0]);
//		else p3[2] = p2[2];
		if ( input.contains("E") ) p3[3] = Double.parseDouble(input.substring(input.indexOf("E")+1).split("[GXYZE]")[0]);
//		else p3[3] = p2[3];

		String output = "G0 X" + p3[0] + " Y" + p3[1] + " Z" + p3[2] + " E" + p3[3] + " "
+ ptsToVec(p2[0],p2[1],p3[0],p3[1])[0] + " "
+ ptsToVec(p2[0],p2[1],p3[0],p3[1])[1] ;
		return output;
	}

}
