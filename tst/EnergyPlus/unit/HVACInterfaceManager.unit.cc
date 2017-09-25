// EnergyPlus, Copyright (c) 1996-2017, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// EnergyPlus::HVACInterfaceManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/HVACInterfaceManager.hh>
#include <EnergyPlus/DataPlant.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include "Fixtures/EnergyPlusFixture.hh"

namespace EnergyPlus {
	TEST_F(EnergyPlusFixture, ExcessiveHeatStorage_Test) {
		using namespace DataPlant;
		using namespace HVACInterfaceManager;
		using namespace DataHVACGlobals;
		Real64 TankOutletTemp;
		TimeStepSys = 1;
		TotNumLoops = 1;
		PlantLoop.allocate(TotNumLoops);
		for (int i = 1; i <= TotNumLoops; ++i) {
			auto & loop(PlantLoop(i));
			loop.LoopSide.allocate(2);
		}
		//Set Up PlantLoop Variables
		PlantLoop(1).Mass = 50;
		PlantLoop(1).FluidName = "Water";
		PlantLoop(1).FluidIndex = 1;
		PlantLoop(1).LoopSide(1).NodeNumOut = 1;
		PlantLoop(1).LoopSide(1).NodeNumIn = 1;
		PlantLoop(1).LoopSide(2).LastTempInterfaceTankOutlet = 80;
		PlantLoop(1).LoopSide(2).TotalPumpHeat = 500;
		DataLoopNode::Node.allocate(TotNumLoops);
		DataLoopNode::Node(1).Temp = 100;
		DataLoopNode::Node(1).MassFlowRate = 10;

		//LoopSideInlet_MdotCpDeltaT should be < LoopSideInlet_McpDTdt
		//Therefore CapExcessStorageTime AND TotalTime will increase by 1 timestep
		UpdateHalfLoopInletTemp(1, 1, TankOutletTemp);
		EXPECT_NEAR(-500, PlantLoop(1).LoopSide(2).LoopSideInlet_MdotCpDeltaT, 0.001);
		EXPECT_NEAR(2928.82, PlantLoop(1).LoopSide(2).LoopSideInlet_McpDTdt, 0.001);
		EXPECT_EQ(1, PlantLoop(1).LoopSide(2).LoopSideInlet_CapExcessStorageTime);
		EXPECT_EQ(1, PlantLoop(1).LoopSide(2).LoopSideInlet_TotalTime);

		PlantLoop(1).LoopSide(2).LastTempInterfaceTankOutlet = 120;		//random

																		//LoopSideInlet_MdotCpDeltaT should be > LoopSideInlet_McpDTdt
																		//Therefore TotalTime will increase by 1 more timestep, but CapExcessStorageTime will NOT increase
		UpdateHalfLoopInletTemp(1, 1, TankOutletTemp);
		EXPECT_NEAR(-500, PlantLoop(1).LoopSide(2).LoopSideInlet_MdotCpDeltaT, .001);
		EXPECT_NEAR(-588.264, PlantLoop(1).LoopSide(2).LoopSideInlet_McpDTdt, .001);
		EXPECT_EQ(1, PlantLoop(1).LoopSide(2).LoopSideInlet_CapExcessStorageTime);
		EXPECT_EQ(2, PlantLoop(1).LoopSide(2).LoopSideInlet_TotalTime);

	}
}
