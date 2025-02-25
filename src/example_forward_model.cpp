/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

/* Example driver program for simple forward model*/

#include <vector>
#include <cstring>
#include <iostream>

#include "general_utils.h"
#include "file_utils.h"
#include "random_utils.h"
#include "blocklanguage.h"
#include "vector_utils.h"
#include "lem.h"
#include "tdemsystem.h"
class cLogger glog; //The global instance of the log file manager
class cStackTrace gtrace; //The global instance of the stacktrace

int skytem_example_ip()
{
	//Load the AEM system specification files for the Skytem moments //only do this once				
	//cTDEmSystem S("..\\..\\examples\\bhmar-skytem\\stmfiles\\Skytem-LM.stm");	
	cTDEmSystem S("..\\..\\examples\\bhmar-skytem\\stmfiles\\Skytem-HM.stm");
	

	//Load the system geometry 
	cTDEmGeometry G;
	G.tx_height = 40;
	G.tx_roll = 0;       G.tx_pitch = 0; G.tx_yaw  = 0;
	G.txrx_dx = -13.35;  G.txrx_dy  = 0; G.txrx_dz = +2.00;
	G.rx_roll = 0;       G.rx_pitch = 0; G.rx_yaw  = 0;

	//Create the  earth structure	
	cEarth1D E(3);
	E.conductivity[0] = 0.010;
	E.conductivity[1] = 0.100;
	E.conductivity[2] = 0.001;
	E.thickness[0] = 20;
	E.thickness[1] = 40;

	E.chargeability[0] = 0.0;
	E.chargeability[1] = 1.0;	
	E.chargeability[2] = 0.0;

	E.timeconstant[0] = 0.0;
	E.timeconstant[1] = 0.001;
	E.timeconstant[2] = 0.0;

	E.frequencydependence[0] = 0.0;
	E.frequencydependence[1] = 1.0;
	E.frequencydependence[2] = 0.0;
	
	//Create a response object
	cTDEmResponse R;
	//Run the forward model
	S.forwardmodel(G, E, R);	
	for (size_t i = 0; i < R.SZ.size(); i++){
		double wct = 0.5*(S.WinSpec[i].TimeHigh + S.WinSpec[i].TimeLow);
		printf("%zu %10e %10e\n", i, wct, R.SZ[i]);		
	}	
	return 0;
}

int skytem_example()
{
	//Load the AEM system specification files for the Skytem moments
	//only do this once
	//LM = Low moment pulse
	cTDEmSystem LM("..\\..\\examples\\bhmar-skytem\\stmfiles\\Skytem-LM.stm");
	//HM = high moment pulse	
	cTDEmSystem HM("..\\..\\examples\\bhmar-skytem\\stmfiles\\Skytem-HM.stm");

	//Load the system geometry (same for both moments)
	//This changes every fiducial/station
	cTDEmGeometry G;
	G.tx_height = 30;
	G.tx_roll = 0;       G.tx_pitch = 0; G.tx_yaw = 0;
	G.txrx_dx = -12.62;  G.txrx_dy = 0; G.txrx_dz = +2.16;
	G.rx_roll = 0;       G.rx_pitch = 0; G.rx_yaw = 0;

	//Create the earth structure
	//This changes every fiducial/station
	cEarth1D E(3);
	E.conductivity[0] = 0.010;
	E.conductivity[1] = 0.100;
	E.conductivity[2] = 0.001;
	E.thickness[0] = 20;
	E.thickness[1] = 40;

	//bottom layer is infinite thickness and not set

	//Create a response object for each moment (they have different numbers of windwos)
	cTDEmResponse LMR;
	cTDEmResponse HMR;	
				
	//Run the forward model for each moment			
	LM.forwardmodel(G, E, LMR);
	HM.forwardmodel(G, E, HMR);			
	
	//Merge the secondary field vertical (Z) components data into one data vector
	//std::vector<double> data = concaternate(LMR.SZ, HMR.SZ);
	std::vector<double> data = -1.0*HMR.SZ;

	for (size_t i = 0; i < LMR.SZ.size(); i++){
		printf("%zu %g\n", i, LMR.SZ[i]);
	}

	for (size_t i = 0; i < HMR.SZ.size(); i++){
		printf("%zu %g\n", i, HMR.SZ[i]);
	}
	
	return 0;
}

int skytem_computation_time()
{
	//Load the AEM system specification files for the Skytem moments
	//only do this once
	//LM = Low moment pulse
	cTDEmSystem LM("..\\..\\examples\\bhmar-skytem\\stmfiles\\Skytem-LM.stm");
	//HM = high moment pulse	
	cTDEmSystem HM("..\\..\\examples\\bhmar-skytem\\stmfiles\\Skytem-HM.stm");

	//Load the system geometry (same for both moments)
	//This changes every fiducial/station
	cTDEmGeometry G;
	G.tx_height = 30;
	G.tx_roll = 0;       G.tx_pitch = 0; G.tx_yaw = 0;
	G.txrx_dx = -12.62;  G.txrx_dy = 0; G.txrx_dz = +2.16;
	G.rx_roll = 0;       G.rx_pitch = 0; G.rx_yaw = 0;

	//Create the earth structure
	//This changes every fiducial/station
	cEarth1D E(1);
	
	//Create a response object for each moment (they have different numbers of windwos)	
	cTDEmResponse LMR;
	cTDEmResponse HMR;
	double sum = 0.0;
	for (size_t j = 1; j <= 50; j++) {
		size_t nlayers = j;
		E = cEarth1D(nlayers);
		double t1 = gettime();
		size_t nloops = 1000;
		for (size_t i = 0; i < nloops; i++) {
			//Run the forward model for each moment with random models
			for (size_t k = 0; k < nlayers; k++) E.conductivity[k] = urand(0.001, 2.0);
			for (size_t k = 0; k < nlayers - 1; k++) E.thickness[k] = urand(1.0, 10.0);
			LM.forwardmodel(G, E, LMR);
			HM.forwardmodel(G, E, HMR);
			sum += LMR.PX;//Just to make sure compiler does not optimize out the computation
			sum += HMR.PX;//Just to make sure compiler does not optimize out the computation
		}
		double t2 = gettime();
		std::cout << "Time: " << j << " " << t2 - t1 << std::endl;
	}	
	return 0;
}

int main(int argc, char* argv[])
{		
	//skytem_example();		
	//skytem_example_ip();
	skytem_computation_time();
	prompttocontinue();
}

