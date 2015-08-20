// C++ Headers
#include <memory>

// EnergyPlus headers
#include <DataGlobals.hh>
#include <DataIPShortCuts.hh>
#include <GroundTemperatureModeling/GroundTemperatureModelManager.hh>
#include <GroundTemperatureModeling/XingGroundTemperatureModel.hh>
#include <InputProcessor.hh>

namespace EnergyPlus {

	//******************************************************************************

	// Xing model factory
	std::shared_ptr< XingGroundTempsModel > 
	XingGroundTempsModel::XingGTMFactory( 
		int objectType, 
		std::string objectName
	)
	{
		// SUBROUTINE INFORMATION:
		//       AUTHOR         Matt Mitchell
		//       DATE WRITTEN   Summer 2015
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// Reads input and creates instance of Xing ground temps model

		// USE STATEMENTS:
		using namespace DataIPShortCuts;
		using namespace GroundTemperatureManager;

		// Locals
		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		bool found = false;
		int NumNums;
		int NumAlphas;
		int IOStat;
		bool ErrorsFound = false;

		// New shared pointer for this model object
		std::shared_ptr< XingGroundTempsModel > thisModel( new XingGroundTempsModel() );

		std::string const cCurrentModuleObject = "Site:GroundTemperature:Undisturbed:Xing";
		int numCurrModels = InputProcessor::GetNumObjectsFound( cCurrentModuleObject );

		for ( int modelNum = 1; modelNum <= numCurrModels; ++modelNum ) {

			InputProcessor::GetObjectItem( cCurrentModuleObject, modelNum, cAlphaArgs, NumAlphas, rNumericArgs, NumNums, IOStat );

			if ( objectName == cAlphaArgs( 1 ) ) {
				// Read input into object here

				thisModel->objectName = cAlphaArgs( 1 );
				thisModel->objectType = objectType;
				thisModel->groundThermalDiffisivity = rNumericArgs( 1 ) / ( rNumericArgs( 2 ) * rNumericArgs( 3 ) );
				thisModel->aveGroundTemp = rNumericArgs( 4 );
				thisModel->surfTempAmplitude_1 = rNumericArgs( 5 );
				thisModel->phaseShift_1 = rNumericArgs( 6 );
				thisModel->surfTempAmplitude_2 = rNumericArgs( 7 );
				thisModel->phaseShift_2 = rNumericArgs( 8 );
				
				found = true;
				break;
			}
		}

		if ( found && !ErrorsFound ) {
			groundTempModels.push_back( thisModel );
			return thisModel;
		} else {
			ShowFatalError( "Site:GroundTemperature:Undisturbed:Xing--Errors getting input for ground temperature model");
			return nullptr;
		}
	}

	//******************************************************************************

	Real64 XingGroundTempsModel::getGroundTemp()
	{
		// SUBROUTINE INFORMATION:
		//       AUTHOR         Matt Mitchell
		//       DATE WRITTEN   Summer 2015
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// Returns the ground temperature for the Site:GroundTemperature:Undisturbed:Xing

		// USE STATEMENTS:
		using DataGlobals::Pi;

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		Real64 retVal;
		Real64 summation;

		// Inits
		summation = 0.0;

		for ( int n = 1; n <= 2; ++n ) {
			
			Real64 static tp( 365 ); // Period of soil temperature cycle
			Real64 Ts_n; // Amplitude of surface temperature
			Real64 PL_n; // Phase shift of surface temperature
			
			Real64 term1;
			Real64 term2;

			if ( n == 1 ) {
				Ts_n = surfTempAmplitude_1;
				PL_n = phaseShift_1;
			} else if ( n == 2 ) {
				Ts_n = surfTempAmplitude_2;
				PL_n = phaseShift_2;
			}

			term1 = -depth * std::sqrt( ( n * Pi ) / ( groundThermalDiffisivity * tp ) );
			term2 = ( 2 * Pi * n ) / tp * ( simTimeInDays - PL_n ) - depth * std::sqrt( ( n * Pi ) / ( groundThermalDiffisivity * tp ) );

			summation += std::exp( term1 ) * Ts_n * std::cos( term2 );
		}

		retVal = aveGroundTemp - summation;

		return retVal;
	}

	//******************************************************************************

	Real64 XingGroundTempsModel::getGroundTempAtTimeInMonths(
		Real64 _depth,
		int month
	)
	{
		// SUBROUTINE INFORMATION:
		//       AUTHOR         Matt Mitchell
		//       DATE WRITTEN   Summer 2015
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// Returns ground temperature when input time is in months

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		Real64 const aveDaysInMonth = 365 / 12;

		depth = _depth;

		// Convert months to seconds. Puts 'seconds' time in middle of specified month
		if ( month >= 1 && month <= 12 ) {
			simTimeInDays = aveDaysInMonth * ( ( month - 1 ) + 0.5 );
		} else {
			ShowFatalError("XingGroundTempsModel: Invalid month passed to ground temperature model");
		}
		
		// Get and return ground temperature
		return getGroundTemp();
	}

	//******************************************************************************

	Real64 XingGroundTempsModel::getGroundTempAtTimeInSeconds(
		Real64 _depth,
		Real64 seconds
	)
	{
		// SUBROUTINE INFORMATION:
		//       AUTHOR         Matt Mitchell
		//       DATE WRITTEN   Summer 2015
		//       MODIFIED       na
		//       RE-ENGINEERED  na

		// PURPOSE OF THIS SUBROUTINE:
		// Returns ground temperature when time is in seconds

		// SUBROUTINE LOCAL VARIABLE DECLARATIONS:
		Real64 const secPerDay = 24 * 3600;

		depth = _depth;

		simTimeInDays = seconds / secPerDay;

		return getGroundTemp();
	}

	//******************************************************************************

	//     NOTICE

	//     Copyright (c) 1996-2015 The Board of Trustees of the University of Illinois
	//     and The Regents of the University of California through Ernest Orlando Lawrence
	//     Berkeley National Laboratory.  All rights reserved.

	//     Portions of the EnergyPlus software package have been developed and copyrighted
	//     by other individuals, companies and institutions.  These portions have been
	//     incorporated into the EnergyPlus software package under license.   For a complete
	//     list of contributors, see "Notice" located in main.cc.

	//     NOTICE: The U.S. Government is granted for itself and others acting on its
	//     behalf a paid-up, nonexclusive, irrevocable, worldwide license in this data to
	//     reproduce, prepare derivative works, and perform publicly and display publicly.
	//     Beginning five (5) years after permission to assert copyright is granted,
	//     subject to two possible five year renewals, the U.S. Government is granted for
	//     itself and others acting on its behalf a paid-up, non-exclusive, irrevocable
	//     worldwide license in this data to reproduce, prepare derivative works,
	//     distribute copies to the public, perform publicly and display publicly, and to
	//     permit others to do so.

	//     TRADEMARKS: EnergyPlus is a trademark of the US Department of Energy.

}	// EnergyPlus namespace
