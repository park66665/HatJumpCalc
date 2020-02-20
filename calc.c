/* DISCLAIMER: THIS SIMULATION WAS WRITTEN WITH THE CONCEPTION THAT POSITS THE HAT_PLAYER'S POSITION AS LOWER
BACK OF THE COLLISION BOX INSTEAD OF THE POSITION WHERE THE GAME(A HAT IN TIME) USES: X = center, Y = center,
Z = about 36 over the feet; PLEASE KEEP THAT IN MIND WHILE YOU ARE DEALING WITH THE SIMULATION'S CODE. THIS
SIMULATION WILL COMPENSATE THAT DIFFERENCE AT OUTPUT, BUT PLEASE NOTE THAT THE COMPENSATION IS ONLY VERISIMILAR,
AND IT WAS BASED ON LOGS PRODUCED USING KISMET ON A SPECIALLY PREPARED MAP FOR THIS PURPOSE. WHILE THE SIMULATION'S
ERRORS IN VELX, VELZ AND POSX IS UNLIKELY TO EXCEED 0.1 AT THE LAST AND PENULTIMATE FRAMES OF THE SIMULATION
UNLESS YOU RUN THE SIMULATION FOR AN EXTENDED TIME (>500 FRAMES), PLEASE NOTE THAT THERE IS AFOREMENTIONED
"INEXPLICABLY LARGE" POSZ CALCULATION ERROR THAT MIGHT EXCEED 0.1 IF THE SIMULATION HAPPENS TO BE RUN FOR 300
FRAMES OR LONGER, BUT ALL ESTIMATES GIVEN HERE ARE VALID ONLY WHEN THE GAME CONSISTENTLY RUNS AT EXACTLY 60.00 FPS
FOR THE LENGTH OF THE SIMULATION; SUCH FRAMES WHEN THE GAME FAILS TO RUN AT 1/60 SECONDS PER FRAME WILL CAUSE
MUCH LARGER ERRORS. THIS SIMULATION IS NOT A PERFECT RECREATION OF THE GAME'S BEHAVIOR, THEREFORE PLEASE DO NOT
PRESUME ITS INVULNERABILITY, FRICTIONLESS INTERCHANGEABILITY WITH THE GAME (E.G. CITING ONLY THIS SIMULATION'S
OUTPUT WHEN DESCRIBING THE GAME'S ACTUAL BEHAVIOR), INFALLIBILITY, SUPREMACY AGAINST THE ACTUAL GAME, AND OTHER
QUALITIES THAT MIGHT ARISE FROM THE ASSUMPTION OF COMPLETENESS AND EXACTITUDE OF THIS SIMULATION. */

// Above disclaimer is just here to keep me out of trouble. Go nuts, but don't get me in trouble.

// This simulation currently only *simulates* airtime; from the onset of the grace period (0.2 sec from stepoff)
// to right before the onset of WallSlide.

// No Hat Kid was mortally wounded during the making of this simulator. I promise!

// Not everyone uses GCC, right? I have a feeling that someone will try to compile this on an ancient cc.
//#include <quadmath.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// You might want to edit these to make this run in actual linux. Getting rid of this entirely might be the way.
#if __MINGW32__ || __MINGW64__ || MINGW
    #define printf __mingw_printf
    #define fprintf __mingw_fprintf
#endif

int main(void) {

    // THE ROOT OF ALL EVILS: random 0.01ms-level delays in a few frames, i.e. stutters.

    // Using double would speed things up, BUT I AM NOT GIVING QUADRUPLE PRECISION UP!

    // Nevermind, quadruple precision was MUCH slower than I have thought. I will use extended precision instead.

    // Principal Defines

    // This define controls whether the program will calculate about common jumps, i.e. the canonical long jump.
    #define CJ 0
    // This define controls whether the program will calculate about scooter jumps.
    #define SJ 1

    // ApplyJumpHold-related variables
    long double JumpRemain, a;
    // Z velocity of the Hat_Player
    long double VelZ;
    // Z position of the Hat_Player
    long double PosZ;
    // X velocity of the Hat_Player
    long double VelX;
    // X position of the Hat_Player
    long double PosX;
    // JumpZ of the Hat_Player. Hat Kid can jump real high!
    long double JumpZ = 540.0L;
    #if CJ
        // Special gravity multiplier for the Hat_Player. It is 0.85 for JumpDive, 1.0 for everything else.
        long double GravityMulti = 1.0L;
    #endif
    // For Squeezing out PosX. It should be the maximum land speed when the Hat_Player is on the starting platform.
    long double BaseVelX;
    // The Highscore! Will be replaced if a stronger challenger approches. Only valid for each HeightDiff.
    long double High_PosX;
    // This highscore means mostly nothing, but it remains here for verification.
    long double High_PosZ;
    // Frame- or Tick-counter variable. Ticks should always happen exactly 60 times per second, but it never really does,
    // thus THAT'S THE ROOT OF ALL EVILS!
    int Frame;
    // When you walk over the ledge and start falling, there is a 0.2-second-long grace period for non-DoubleJump opportunity.
    // You will be forced to DoubleJump if you run out of that grace period. This is the gap between the first falling
    // frame and the first jump.
    int Hat_Gracew;
    // Gap between the first jump and the DoubleJump.
    int Hat_Fw;
    #if CJ
        // Gap between the DoubleJump and the JumpDive.
        int Hat_Sw;
        // Gap between the JumpDive and the ExitJumpDive. It is very likely to be the longest of the four.
        int Hat_Dw;
    #endif
    // The HeightDiff highscore for Gracew. It is very liekly to be 11, but it can be lower on high HeightDiffs:
    // >200 for common jumps, >350 for scooter jumps.
    int High_Hat_Gracew;
    // The HeightDiff highscore for Hat_Fw. It will grow as HeightDiff gets lower.
    int High_Hat_Fw;
    // The HeightDiff highscore for Frame. It will grow as HeightDiff gets lower though it is mostly useless.
    int High_Frame;
    #if CJ
        // (CommonJump Only) The HeightDiff highscore for Hat_Sw. It will be somewhere neer High_Hat_Sw.
        int High_Hat_Sw;
        // (CommonJump Only) The HeightDiff highscore for Hat_Dw. It will be much larger than High_Hat_Fw and High_Hat_Sw.
        int High_Hat_Dw;
    #endif
    // FRAMERATE, THE AVATAR OF THE ROOT OF ALL EVILS
	// note: If you want to change this, please check EVERY initial variables since some of them are there with an assumption
	// of 60 ticks per second, e.g. TestRangeHi_Gracew.
    long double const TicksperS = 60.0L;
    // Hat_Gracew higher than 11 is absolutely impossible in game. You have been warned.
    int const TestRangeHi_Gracew = 11;
    // WallSlide increases the Z about 148.624; the rest is what I call the "collision gap" which is ledge jump's responsibility.
    // The "collision gap" is much harder to measure, yet it is known to be between 86.15 and 86.21.
    // +0.1 is added to compensate the inexplicably large VelZ (and PosZ) calculation errors, which is likely to be
    // from the accumulation of JumpHold-related calculation errors.
    // Use -65.9L for ledge jump/hang only platforms.
    long double const WallSlideThreshold = -234.674L;
    //long double const WallSlideThreshold = -65.9L;
    // HeightDiff means the difference between the PosZ of the starting platform and the destination platform.
    long double HeightDiff = 0.0L;
    // The amount which the Hat_Player will run on the starting platform.
    long double RunPosX;

	// BELOW THREE ARE ONLY UESFUL IF YOU WANT TO COMPARE THE ACTUAL OUTPUT OF THE GAME AND THIS. YOU CAN SAFELY LEAVE
	// THIS VALUE TO GO NUTS IF YOU ARE NOT GOING TO DO SO. THESE NUMBERS ARE CONSERVED ONLY FOR THE PURPOSE OF
    // MAKING YOU UNDERSTAND THESE BETTER.

    // In-Game Test Only Variable
    // This is literally the same thing from the recommended start position.
    long double StartPosX;
    // In-Game Test Only Variable
    // The in-game PosX of the Hat_Player where the Hat_Player starts falling. This number is empirical, i.e.
    // I made the Hat Kid fall off the ledge a LOT, and situational, i.e. SHOULD be observed again when you
    // change the PosX or collision box of the starting platform.
    long double const LedgePosX = -533.958L;
    // In-Game Test Only Variable
    // The in-game PosZ of the Hat_Player on the starting Platform. The actual number WILL fluctuate a little,
    // so the standard practice here is the PosZ when you start initial acceleration on the starting platform.
    long double const BasePosZ = 246.150L;

    // The game's PosX calculation errors DO happen in that short period of the initial acceleration. This will
    // mitigate the error by adding a buffer. DECREASE THIS AT YOUR OWN RISK.
    long double const LedgePosXErrBuf = 0.025L;
    // This is the in-game GravityZ. THIS NUMBER IS A LIE, AND -520.0 IS ALSO A LIE.
    long double const GravityZ = -750.0L;
    // This magic number is related to the initiation of the WallSlide, a.k.a sticking-to-the-wall. I don't
    // understand this fully; this number is empirical. (I thought it was 28.0, btw.)
    long double const WallSlideInitiationMagicNumber = 53.85L;

    // The Graveyard of Unused Variables
    //int const TickScale = 1;
    //int const TestRangeMid = 45;

    // This will increase the simulation speed a LOT, disable this if you want the whole data.
    bool const Trim_Hat_Gracew = false;
    // Heuristic Testrange Trimming is quite simple; when the heightdiff doesn't change much, the optimal numbers
    // don't change much too, so this will reduce the runtime DRASTICALLY by narrowing testranges of the
    // following simulation down at the calculated optimal numbers, i.e. Highs. This technically can compromise
    // the accuracy of the calculation, but the results say that HeuristicTestrangeTrimmingWidth >= 10 will be
    // more than enough on most occasions. Please note that HTT is currently IMCOMPATIBLE with WD.
    #define HTT 0

    // Infomation Output Settings

    // This define controls whether the program will print preliminary highs. Turn this on if you are really bored, prone to the
    // numbers wrong, or have a really slow CPU; like Intel 80486 or something.
    #define SPH 0
    // This define controls whether the program will print some info about StartPosX. This is only useful at in-game testing.
    #define SSPXR 0
    // This define controls whether the program will print highs per GraceW to stdout.
    #define PGWHS 0
    // This define controls whether the program will print highs per GraceW to a file.        
    #define PGWHF 1
    // This define controls whether the program will print highs to a file.
    #define PHF 1
    // This define controls whether the program will print highs to stdout.
    #define PHS 0

    // This define coltrols whether the program will print every simulation's result.
    #if !HTT
        #define WD 1
    #endif

    // This define coltrols whether the program will print processed ScooterJump GrandMasterFile.
    #if SJ
        #define PMF 1
    #endif

    // These defines controls whether the program will try to trim "empty" highs.
    #if PHF || PHS
        #define TEHHD 1
    #endif
    #if PGWHS || PGWHF
        #define TEHGW 1
    #endif

    // This controls how many HeightDiff will be calculated by changing the changes made to the HeightDiff of each pass;
    // in other words, the higher it is, the "sparser" the data will be, and vice versa. Please note that changing this
    // number can require the tuning of HeuristicTestrangeTrimmingWidth; >-100.0 can push (HeuristicTestrangeTrimmingWidth
    // = 10) to its limit. The HeightDiff is controlled by a for statement, so take a look at that.
    long double const HeightDiffChange = -10.0L;

    // These numbers controls the HeightDiffs the program will simulate and calculate on. The HeightDiff is controlled
    // by a for statement, so take a look at that.

    #if CJ
        long double const TestRangeHeightDiffCommonJumpStart = 300.0L;
        long double const TestRangeHeightDiffCommonJumpEnd = -1000000.0L;
    #endif
    #if SJ
        long double const TestRangeHeightDiffScooterJumpStart = 500.0L;
        long double const TestRangeHeightDiffScooterJumpEnd = -10000.0L;
    #endif

    // Public String Buffer, Yaaayayyyayyyayayaayayay
    #if WD || PMF
        char buf[128];
    #endif

    #if PHF
        // The RawOutput file
        FILE *output = fopen("HatPhysicsCalcOutputRaw.csv", "a");
    #endif

    #if PGWHF
        // The RawOutput file, except it is GraceW highs.
        FILE *PGWHF_output = fopen("HatPhysicsCalcOutputRaw_PGWHF.csv", "a");
    #endif

    #if HTT

        // When a calculation at a HeightDiff is finished, the Highs will be referenced to determine the next calcultion's
        // testranges. The testranges will be (High ± HeuristicTestrangeTrimmingWidth), so this number is at the core of the
        // compromise of this heuristic. 10 is the rule-of-thumb, but you can reduce this to further the compromise, or
        // increase this to deal with potential outliers that lurk out there when playing with HeightDiffChange or
        // TestRangeHeightDiff.
        int const HeuristicTestrangeTrimmingWidth = 15;

        // These numbers control the initial testrange before the hueristic. THIS INITIAL WIDE-RANGE SEARCH IS NECESSARY
        // SINCE THE PROGRAM CANNOT HEURISTICALLY DETERMINE THE NARROW AND EFFICIENT TESTRANGES WHEN THERE ARE NO PREVIOUS
        // HIGHS. However, if you have calculated highs, you can narrow this down, but please be catious.

        #if CJ

            int const TestRangeLo_Initial_CommonJump_Fw = 20;
            int const TestRangeHi_Initial_CommonJump_Fw = 100;
            int const TestRangeLo_Initial_CommonJump_Sw = 20;
            int const TestRangeHi_Initial_CommonJump_Sw = 100;
            int const TestRangeLo_Initial_CommonJump_Dw = 20;
            int const TestRangeHi_Initial_CommonJump_Dw = 100;

        #endif

        // The TestRange of ScooterJump_Fw can be found at #if PMF section below.

        #if CJ

            // Setting these to be lower than 9 would be futile, since there is an explicit limit(SetNoJumpPeriod(0.15)).
            int TestRangeLo_CJ_Fw, TestRangeLo_Sw, TestRangeLo_Dw

            // You might want to increase (or decrease to save a little bit of time) these for extremely low HeightDiff (<-5000).
            int TestRangeHi_CJ_Fw, TestRangeHi_Sw, TestRangeHi_Dw

            TestRangeLo_CJ_Fw = TestRangeLo_Initial_CommonJump_Fw;
            TestRangeLo_Sw = TestRangeLo_Initial_CommonJump_Sw;
            TestRangeLo_Dw = TestRangeLo_Initial_CommonJump_Dw;

            TestRangeHi_CJ_Fw = TestRangeHi_Initial_CommonJump_Fw;
            TestRangeHi_Sw = TestRangeHi_Initial_CommonJump_Sw;
            TestRangeHi_Dw = TestRangeHi_Initial_CommonJump_Dw;

        #endif

        #if SJ

        int TestRangeLo_SJ_Fw, TestRangeHi_SJ_Fw;

        #endif

    #else

        #if CJ
            int const TestRangeLo_CJ_Fw = 20;
            int const TestRangeHi_CJ_Fw = 100;
            int const TestRangeLo_Sw = 20;
            int const TestRangeHi_Sw = 100;
            int const TestRangeLo_Dw = 20;
            int const TestRangeHi_Dw = 100;

        // The TestRange of ScooterJump_Fw can be found at #if PMF section below.

        // PrintEverySimulationResult-related declarations

            #if WD

                int const TestRange_I_CJ_F_Amount = TestRangeHi_CJ_Fw - TestRangeLo_CJ_Fw + 1;
                int const TestRange_I_CJ_S_Amount = TestRangeHi_Sw - TestRangeLo_Sw + 1;
                int const TestRange_I_CJ_D_Amount = TestRangeHi_Dw - TestRangeLo_Dw + 1;

            #endif
        #endif
    #endif

    // Data needs to be spliced to be seen in 2D *conviniently*.

    #if WD        
        
        #if CJ && SJ

            const int WDOutputArraySize = 1 + TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + TestRange_I_CJ_D_Amount + 2 + ( TestRangeHi_Gracew + 1 );
            
            // The order of The Cursed Array of Needy File Streams will be:
            // CJ HeightDiff MasterFiles        0
            // CJ HeightDiff Fw Files           1 ~ TestRange_I_CJ_F_Amount
            // CJ HeightDiff Fw Files           TestRange_I_CJ_F_Amount + 1 ~ TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount
            // CJ HeightDiff Fw Files           TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + 1 ~ TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + TestRange_I_CJ_D_Amount
            // SJ GrandMasterFile               TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + TestRange_I_CJ_D_Amount + 1 = WDScooterJumpPosBase
            // SJ HeightDiff MasterFiles        WDScooterJumpPosBase + 1
            // SJ HeightDiff Gracew Files       WDScooterJumpPosBase + 1 + 1 ~ WDScooterJumpPosBase + 1 + 1 + TestRangeHi_Gracew + 1

            // Thus WDScooterJumpPosBase is...
            int const WDScooterJumpPosBase = TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + TestRange_I_CJ_D_Amount + 1;

        #endif

        #if !CJ && SJ

            const int WDOutputArraySize = 2 + TestRangeHi_Gracew + 1;

            // The order of The Cursed Array of Needy File Streams will be:
            // SJ GrandMasterFile               0 = WDScooterJumpPosBase
            // SJ HeightDiff MasterFiles        WDScooterJumpPosBase + 1
            // SJ HeightDiff Gracew Files       WDScooterJumpPosBase + 1 + 1 ~ WDScooterJumpPosBase + 1 + 1 + TestRangeHi_Gracew + 1

            // Thus WDScooterJumpPosBase is...
            int const WDScooterJumpPosBase = 0;

        #endif

        #if CJ && !SJ

            const int WDOutputArraySize = 1 + TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + TestRange_I_CJ_D_Amount;
            
            // The order of The Cursed Array of Needy File Streams will be:
            // CJ HeightDiff MasterFiles        0
            // CJ HeightDiff Fw Files           1 ~ TestRange_I_CJ_F_Amount
            // CJ HeightDiff Fw Files           TestRange_I_CJ_F_Amount + 1 ~ TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount
            // CJ HeightDiff Fw Files           TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + 1 ~ TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount + TestRange_I_CJ_D_Amount

            // Thus WDScooterJumpPosBase needs not exist.

        #endif

        #if !CJ && !SJ

            puts("WTF ARE YOU DOING HERE?! LETTING ME SIMULATE NOTHING AND ASKING ME TO GIVE THE WHOLE DATA?\n");
            puts("GROW UP, WEIRDO. I QUIT!!");
            exit(1);

        #endif

        #if CJ || SJ

            // The Cursed Array of Needy File Streams
            FILE *wholedataoutput[WDOutputArraySize];

        #endif

    #endif

    // GenerateScooterJumpProcessedMasterFiles-related declarations
    #if PMF

        FILE *ScooterJumpProcessedGrandMasterFile; // The ScooterJumpProcessedGrandMasterFile, one file to rule them all
        FILE *ScooterJumpProcessedHeightDiffMasterFile; // The ScooterJumpProcessedHeightDiffMasterFile, n files for each HeighDiff

        #if HTT

            // You can set these to whatever you want.

            int const TestRangeLo_Initial_ScooterJump_Fw = 10;
            int const TestRangeHi_Initial_ScooterJump_Fw = 300;

            int const TestRange_I_SJ_F_Amount = TestRangeHi_Initial_ScooterJump_Fw - TestRangeLo_Initial_ScooterJump_Fw + 1;
            int TestRange_SJ_F_Amount;

            TestRangeLo_SJ_Fw = TestRangeLo_Initial_ScooterJump_Fw;
            TestRangeHi_SJ_Fw = TestRangeHi_Initial_ScooterJump_Fw;

            unsigned short int SJPGMFi; // master index

        #else

            int const TestRangeLo_SJ_Fw = 10;
            int const TestRangeHi_SJ_Fw = TestRangeLo_SJ_Fw + (1 << 8) - 1;
            int const TestRange_I_SJ_F_Amount = 1 << 8;

            union doubleindex // I know this is horrible... but I think this is cool too.
            {
                unsigned short int a; // master index
                unsigned char b[2]; // b[0]=Hat_Fw, b[1]=Hat_Gracew
            };
            union doubleindex SJPGMFi;

        #endif

        // POINTER SORCERY, BOO!
        // You can now use *pSJPGMFi for both HTT and non-HTT cases!
        unsigned short int *pSJPGMFi = (unsigned short int*)&SJPGMFi;

        struct result // A simple struct for simplicity
        {
            int Frame; // 0 is used for failed simulations.
            long double PosZ;
            long double PosX;
        };
        // The Grand Array of Simulation Results
        struct result SJPGMFData[ ( TestRangeHi_Gracew + 1 ) * TestRange_I_SJ_F_Amount ];

    #elif SJ

        // You can set these to whatever you want.

        #if HTT

            int const TestRangeLo_Initial_ScooterJump_Fw = 10;
            int const TestRangeHi_Initial_ScooterJump_Fw = 300;

            TestRangeLo_SJ_Fw = TestRangeLo_Initial_ScooterJump_Fw;
            TestRangeHi_SJ_Fw = TestRangeHi_Initial_ScooterJump_Fw;

        #else

            int const TestRangeLo_SJ_Fw = 10;
            int const TestRangeHi_SJ_Fw = 300;

        #endif

    #endif

    #if ( PGWHF || PGWHS ) || PMF

        // integer GraceW highscores
        int GraceW_High_Hat_Fw[TestRangeHi_Gracew+1], GraceW_High_Frame[TestRangeHi_Gracew+1];

        #if CJ

            // CommonJump-only GraceW highscores
            int GraceW_High_Hat_Sw[TestRangeHi_Gracew+1], GraceW_High_Hat_Dw[TestRangeHi_Gracew+1];

        #endif

        // long double GraceW highscores
        long double GraceW_High_PosX[TestRangeHi_Gracew+1], GraceW_High_PosZ[TestRangeHi_Gracew+1];

    #endif

    // If JumpRemain gets too small, this boolean will stop ApplyJumpHold.
    // bool const CullJumpRemain = false;

    // TODO: Heuristic error reduction
    // TODO: Investigate the After-Jump Low-Value-Minus-DeltaVelZ related error. ("When You Start To Fall Error")
    // TODO: Early bailout
    // TODO: Reduce Redundant Simulations, i.e. the 11, 50, 50, 50 simulation would be done a LOT of times.
    // TODO: I don't need to search for THAT MANY cases, right? Can we use more... assertive heuristics, right?

    /*
    struct Tick {
        long double VelX;
        long double PosX;
        long double VelZ;
        long double PosZ;
    };
    */

    High_PosX = 0.0L;

    // Testing the Jump + DoubleJump + JumpDive + ExitJumpDive combination - the canonical long jump.

    #if CJ

        BaseVelX = 400.00L;
        PosX = 0.0L;

        // Squeezing Out...

        for (VelX = 0.0L; VelX <= BaseVelX; VelX += 25.0L) {
            PosX += VelX / TicksperS;
            #if SSPXR
                printf("%.17Lg, ",VelX);
                printf("%.17Lg\n",PosX); 
            #endif
        }

        VelX = BaseVelX;
        PosX += VelX / TicksperS;
        #if SSPXR
            printf("%.17Lg, ",VelX);
            printf("%.17Lg\n",PosX);
        #endif

        RunPosX = PosX + LedgePosXErrBuf - VelX / TicksperS;
        StartPosX = LedgePosX - RunPosX;
        #if SSPXR
            printf("Recommended Start Pos X: %.17Lg\n", StartPosX );
        #endif
        
        for (HeightDiff = TestRangeHeightDiffCommonJumpStart; HeightDiff >= TestRangeHeightDiffCommonJumpEnd; HeightDiff += HeightDiffChange ) {
            // High records are only for each HeightDiff.
            High_PosX = 0.0L;
            High_Hat_Fw = 0;
            High_Hat_Sw = 0;
            High_Hat_Dw = 0;
            High_Hat_Gracew = 0;
            High_Frame = 0;
            High_PosZ = 0.0L;

            #if WD
                int i = 0;
                __mingw_snprintf(buf, sizeof buf, "WDoutput/HatPhysicsCalcOutputWholeDataCommonJump HeightDiff %.17Lg MasterFile.csv", HeightDiff);
                wholedataoutput[i] = fopen(buf, "a");
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_HeightDiff, ", HeightDiff);
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_Hat_Gracew, ", HeightDiff);
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_Hat_Fw, ", HeightDiff);
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_Hat_Sw, ", HeightDiff);
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_Hat_Dw, ", HeightDiff);
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_Frame, ", HeightDiff);
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_PosZ, ", HeightDiff);
                fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Masterfile_PosX\n", HeightDiff);
                i++;
                for (Hat_Fw = TestRangeLo_CJ_Fw; Hat_Fw <= TestRangeHi_CJ_Fw; Hat_Fw++ ) {
                    __mingw_snprintf(buf, sizeof buf, "WDoutput/HatPhysicsCalcOutputWholeDataCommonJump HeightDiff %.17Lg Fw %i.csv", HeightDiff, Hat_Fw);
                    wholedataoutput[i] = fopen(buf, "a");
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_HeightDiff, ", HeightDiff, Hat_Fw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_Hat_Gracew, ", HeightDiff, Hat_Fw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_Hat_Fw, ", HeightDiff, Hat_Fw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_Hat_Sw, ", HeightDiff, Hat_Fw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_Hat_Dw, ", HeightDiff, Hat_Fw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_Frame, ", HeightDiff, Hat_Fw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_PosZ, ", HeightDiff, Hat_Fw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Fw_%i_PosX\n", HeightDiff, Hat_Fw);
                    i++;
                }
                for (Hat_Sw = TestRangeLo_Sw; Hat_Sw <= TestRangeHi_Sw; Hat_Sw++ ) {
                    __mingw_snprintf(buf, sizeof buf, "WDoutput/HatPhysicsCalcOutputWholeDataCommonJump HeightDiff %.17Lg Sw %i.csv", HeightDiff, Hat_Sw);
                    wholedataoutput[i] = fopen(buf, "a");
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_HeightDiff, ", HeightDiff, Hat_Sw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_Hat_Gracew, ", HeightDiff, Hat_Sw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_Hat_Fw, ", HeightDiff, Hat_Sw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_Hat_Sw, ", HeightDiff, Hat_Sw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_Hat_Dw, ", HeightDiff, Hat_Sw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_Frame, ", HeightDiff, Hat_Sw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_PosZ, ", HeightDiff, Hat_Sw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Sw_%i_PosX\n", HeightDiff, Hat_Sw);
                    i++;
                }
                for (Hat_Dw = TestRangeLo_Dw; Hat_Dw <= TestRangeHi_Dw; Hat_Dw++ ) {
                    __mingw_snprintf(buf, sizeof buf, "WDoutput/HatPhysicsCalcOutputWholeDataCommonJump HeightDiff %.17Lg Dw %i.csv", HeightDiff, Hat_Dw);
                    wholedataoutput[i] = fopen(buf, "a");
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_HeightDiff, ", HeightDiff, Hat_Dw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_Hat_Gracew, ", HeightDiff, Hat_Dw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_Hat_Fw, ", HeightDiff, Hat_Dw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_Hat_Sw, ", HeightDiff, Hat_Dw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_Hat_Dw, ", HeightDiff, Hat_Dw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_Frame, ", HeightDiff, Hat_Dw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_PosZ, ", HeightDiff, Hat_Dw);
                    fprintf(wholedataoutput[i],"CJ_HeightDiff_%.17Lg_Dw_%i_PosX\n", HeightDiff, Hat_Dw);                    
                    i++;
                }
            #endif

            for (Hat_Gracew = 0; Hat_Gracew <= TestRangeHi_Gracew; Hat_Gracew++ ) {
                if (HeightDiff <= 0.0L  && Trim_Hat_Gracew) {
                    Hat_Gracew = TestRangeHi_Gracew;
                }
                #if SPH || PGWHS
                    printf("Now calculating: Gracew = %i\n",Hat_Gracew);
                #endif
                #if ( PGWHF || PGWHS ) || PMF
                    GraceW_High_PosX[Hat_Gracew] = 0.0L;
                    GraceW_High_Hat_Fw[Hat_Gracew] = 0;
                    GraceW_High_Hat_Sw[Hat_Gracew] = 0;
                    GraceW_High_Hat_Dw[Hat_Gracew] = 0;
                    GraceW_High_Frame[Hat_Gracew] = 0;
                    GraceW_High_PosZ[Hat_Gracew] = 0.0L;
                #endif
                for (Hat_Fw = TestRangeLo_CJ_Fw; Hat_Fw <= TestRangeHi_CJ_Fw; Hat_Fw++ ) {
                    for (Hat_Sw = TestRangeLo_Sw; Hat_Sw <= TestRangeHi_Sw; Hat_Sw++ ) {
                        for (Hat_Dw = TestRangeLo_Dw; Hat_Dw <= TestRangeHi_Dw; Hat_Dw++ ) {

                            Frame = 0;
                            JumpRemain = 0.0L;
                            VelZ = 0.0L;
                            PosZ = 0.0L;
                            VelX = 400.00L;
                            PosX = RunPosX - LedgePosXErrBuf + VelX / TicksperS;
                            //GravityMulti = 1.0L;
                            do {
                                if ( Frame == Hat_Gracew ) {
                                    // grace period ends, first jump commences
                                    JumpRemain=JumpZ*9.0L  /10.0L;
                                    VelZ=JumpZ/10.0L;
                                }
                                if ( Frame == Hat_Gracew + Hat_Fw ) {
                                    // DoubleJump commences
                                    JumpRemain=JumpZ*9.0L  /10.0L;
                                    VelZ=JumpZ/10.0L;
                                }
                                if ( Frame == Hat_Gracew + Hat_Fw + Hat_Sw + 1) {
                                    // Nominal JumpDive VelX is 663.54.
                                    // Yet Actual JumpDive VelX is 656.78.
                                    VelX = 656.78L;
                                    //VelZ = 250.0L;
                                }
                                if ( Frame == Hat_Gracew + Hat_Fw + Hat_Sw + Hat_Dw + 1) {
                                    //VelZ = 200.0L;
                                }

                                // ApplyJumpHold, adapted right from the game's src!
                                a = JumpRemain*13.0L  /TicksperS;
                                VelZ += a*1.22L; // * (((1/TicksperS)-0.01666)*-2.5 + 1);
                                JumpRemain -= a;

                                // Gravity. It SHOULD be ( GravityZ / 60.0 ) per tick, but whatever. I didn't make A Hat in Time.
                                VelZ += GravityZ / 60.0L  * 2.0L  * GravityMulti; //1000.0/TicksperS;                            

                                // Position calculation happens quite late.
                                PosX += VelX / TicksperS;
                                // This (GravityZ*-1/60) is REAL
                                PosZ += ( VelZ + ( GravityZ * -1.0L  / 60.0L  * GravityMulti ) ) / TicksperS;

                                // Setting nominal values will be ignored in position calculation
                                // Due to whatever reason, JumpDive-related changes happen... differently.

                                if ( Frame == Hat_Gracew + Hat_Fw + Hat_Sw ) {
                                    VelX = 650.0L;
                                    VelZ = 250.0L;

                                    JumpRemain = 0.0L;
                                    GravityMulti = 0.85L;
                                }
                                /*
                                if ( Frame == Hat_Gracew + Hat_Fw + Hat_Sw + 1)
                                {
                                    // This Nominal value is only for documentation.
                                    //VelX = 663.54;
                                }
                                */
                                if ( Frame == Hat_Gracew + Hat_Fw + Hat_Sw + Hat_Dw ) {
                                    VelZ = 200.0L;

                                    VelX = 464.48L;
                                    JumpRemain = 0.0L;
                                    GravityMulti = 1.0L;
                                }

                                //Simulation Data Output 1
                                
                                /*

                                if ( HeightDiff == -4600.0 && Hat_Gracew == 10 && Hat_Fw == 85 && Hat_Sw == 88 && Hat_Dw == 130 )
                                {
                                    printf("%.17Lg, ",VelX);
                                    printf("%.17Lg, ",PosX + StartPosX);
                                    printf("(%.17Lg), ",PosX);
                                    printf("%.17Lg, ",VelZ);
                                    printf("%.17Lg, ",PosZ + BasePosZ);
                                    printf("(%.17Lg)\n",PosZ);
                                }

                                */

                                if ( Frame >= Hat_Gracew + Hat_Fw + Hat_Sw + Hat_Dw && PosZ <= WallSlideThreshold + HeightDiff && VelZ < -12.5L  ) {
                                    // We shouldn't go too low; it will be a failed jump.
                                    break;
                                }

                                Frame++;

                            } while ( true );

                            // undo the last tick since the last tick is where Hat_Player should NOT be at.
                            PosX -= VelX/TicksperS;
                            PosZ -= ( VelZ + 12.5L  ) / TicksperS;

                            PosX += WallSlideInitiationMagicNumber;

                            // Compensating RunPosX
                            PosX += - RunPosX;

                            if (PosZ >= WallSlideThreshold + HeightDiff) {

                                #if ( PGWHF || PGWHS ) || PMF

                                if (GraceW_High_PosX[Hat_Gracew] < PosX) {
                                    GraceW_High_PosX[Hat_Gracew] = PosX;
                                    GraceW_High_Hat_Fw[Hat_Gracew] = Hat_Fw;
                                    GraceW_High_Hat_Sw[Hat_Gracew] = Hat_Sw;
                                    GraceW_High_Hat_Dw[Hat_Gracew] = Hat_Dw;
                                    GraceW_High_Frame[Hat_Gracew] = Frame;
                                    GraceW_High_PosZ[Hat_Gracew] = PosZ;

                                #endif

                                    if (High_PosX < PosX) {
                                        // We have a new highscore! Bring a fresh bottle of champagne... wait peck no.
                                        High_PosX = PosX;
                                        High_Hat_Fw = Hat_Fw;
                                        High_Hat_Sw = Hat_Sw;
                                        High_Hat_Dw = Hat_Dw;
                                        High_Hat_Gracew = Hat_Gracew;
                                        High_Frame = Frame;
                                        High_PosZ = PosZ;
                                        
                                        #if SPH
                                            printf("A Preliminary High ( HeightDiff: %.17Lg ), ",HeightDiff);
                                            printf("%i, ",Hat_Gracew);
                                            printf("%i, ",Hat_Fw);
                                            printf("%i, ",Hat_Sw);
                                            printf("%i, ",Hat_Dw);
                                            printf("%i, ",Frame);
                                            printf("%.17Lg, ",PosZ);
                                            printf("%.17Lg\n",PosX);
                                        #endif
                                    }
                                    
                                #if ( PGWHF || PGWHS ) || PMF

                                }

                                #endif

                                #if WD
                                    fprintf(wholedataoutput[0], "%.17Lg, ",HeightDiff);
                                    fprintf(wholedataoutput[0], "%i, ",Hat_Gracew);
                                    fprintf(wholedataoutput[0], "%i, ",Hat_Fw);
                                    fprintf(wholedataoutput[0], "%i, ",Hat_Sw);
                                    fprintf(wholedataoutput[0], "%i, ",Hat_Dw);
                                    fprintf(wholedataoutput[0], "%i, ",Frame);
                                    fprintf(wholedataoutput[0], "%.17Lg, ",PosZ);
                                    fprintf(wholedataoutput[0], "%.17Lg\n",PosX);

                                    int WDFwPos = Hat_Fw - TestRangeLo_CJ_Fw + 1;
                                    fprintf(wholedataoutput[WDFwPos], "%.17Lg, ",HeightDiff);
                                    fprintf(wholedataoutput[WDFwPos], "%i, ",Hat_Gracew);
                                    fprintf(wholedataoutput[WDFwPos], "%i, ",Hat_Fw);
                                    fprintf(wholedataoutput[WDFwPos], "%i, ",Hat_Sw);
                                    fprintf(wholedataoutput[WDFwPos], "%i, ",Hat_Dw);
                                    fprintf(wholedataoutput[WDFwPos], "%i, ",Frame);
                                    fprintf(wholedataoutput[WDFwPos], "%.17Lg, ",PosZ);
                                    fprintf(wholedataoutput[WDFwPos], "%.17Lg\n",PosX);

                                    int WDSwPos = Hat_Sw - TestRangeLo_Sw + 1 + TestRange_I_CJ_F_Amount;
                                    fprintf(wholedataoutput[WDSwPos], "%.17Lg, ",HeightDiff);
                                    fprintf(wholedataoutput[WDSwPos], "%i, ",Hat_Gracew);
                                    fprintf(wholedataoutput[WDSwPos], "%i, ",Hat_Fw);
                                    fprintf(wholedataoutput[WDSwPos], "%i, ",Hat_Sw);
                                    fprintf(wholedataoutput[WDSwPos], "%i, ",Hat_Dw);
                                    fprintf(wholedataoutput[WDSwPos], "%i, ",Frame);
                                    fprintf(wholedataoutput[WDSwPos], "%.17Lg, ",PosZ);
                                    fprintf(wholedataoutput[WDSwPos], "%.17Lg\n",PosX);

                                    int WDDwPos = Hat_Dw - TestRangeLo_Dw + 1 + TestRange_I_CJ_F_Amount + TestRange_I_CJ_S_Amount;
                                    fprintf(wholedataoutput[WDDwPos], "%.17Lg, ",HeightDiff);
                                    fprintf(wholedataoutput[WDDwPos], "%i, ",Hat_Gracew);
                                    fprintf(wholedataoutput[WDDwPos], "%i, ",Hat_Fw);
                                    fprintf(wholedataoutput[WDDwPos], "%i, ",Hat_Sw);
                                    fprintf(wholedataoutput[WDDwPos], "%i, ",Hat_Dw);
                                    fprintf(wholedataoutput[WDDwPos], "%i, ",Frame);
                                    fprintf(wholedataoutput[WDDwPos], "%.17Lg, ",PosZ);
                                    fprintf(wholedataoutput[WDDwPos], "%.17Lg\n",PosX);
                                #endif
                            }
                        }
                    } 
                }
                #if TEHGW
                if ( GraceW_High_Frame[Hat_Gracew] != 0) {
                #endif
                    #if PGWHS                        
                        printf("Gracew %i High ( HeightDiff: %.17Lg ), ", Hat_Gracew, HeightDiff);
                        printf("%i, ",GraceW_High_Hat_Fw[Hat_Gracew]);
                        printf("%i, ",GraceW_High_Hat_Sw[Hat_Gracew]);
                        printf("%i, ",GraceW_High_Hat_Dw[Hat_Gracew]);
                        printf("%i, ",GraceW_High_Frame[Hat_Gracew]);
                        printf("%.17Lg, ",GraceW_High_PosZ[Hat_Gracew]);
                        printf("%.17Lg\n",GraceW_High_PosX[Hat_Gracew]);
                    #endif
                    #if PGWHF
                        fprintf(PGWHF_output,"%.17Lg, ",HeightDiff);
                        fprintf(PGWHF_output,"%i, ",Hat_Gracew);
                        fprintf(PGWHF_output,"%i, ",GraceW_High_Hat_Fw[Hat_Gracew]);
                        fprintf(PGWHF_output,"%i, ",GraceW_High_Hat_Sw[Hat_Gracew]);
                        fprintf(PGWHF_output,"%i, ",GraceW_High_Hat_Dw[Hat_Gracew]);
                        fprintf(PGWHF_output,"%i, ",GraceW_High_Frame[Hat_Gracew]);
                        fprintf(PGWHF_output,"%.17Lg, ",GraceW_High_PosZ[Hat_Gracew]);
                        fprintf(PGWHF_output,"%.17Lg\n",GraceW_High_PosX[Hat_Gracew]);
                    #endif
                #if TEHGW
                }
                #endif
            }
            #if TEHHD
            if ( High_Frame != 0) {
            #endif
                #if PHF
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", HeightDiff);
                    //fprintf(output,"%s, ",buf);
                    fprintf(output,"%.17Lg, ",HeightDiff);
                    fprintf(output,"%i, ",High_Hat_Gracew);
                    fprintf(output,"%i, ",High_Hat_Fw);
                    fprintf(output,"%i, ",High_Hat_Sw);
                    fprintf(output,"%i, ",High_Hat_Dw);
                    fprintf(output,"%i, ",High_Frame);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosZ);
                    //fprintf(output,"%s, ",buf);
                    fprintf(output,"%.17Lg, ",High_PosZ);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosX);
                    //fprintf(output,"%s\n",buf);
                    fprintf(output,"%.17Lg\n",High_PosX);
                #endif
                #if PHS
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", HeightDiff);
                    //printf("%s, ",buf);
                    printf("%.17Lg, ",HeightDiff);
                    printf("%i, ",High_Hat_Gracew);
                    printf("%i, ",High_Hat_Fw);
                    printf("%i, ",High_Hat_Sw);
                    printf("%i, ",High_Hat_Dw);
                    printf("%i, ",High_Frame);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosZ);
                    //printf("%s, ",buf);
                    printf("%.17Lg, ",High_PosZ);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosX);
                    //printf("%s\n",buf);
                    printf("%.17Lg\n",High_PosX);
                #endif
            #if TEHHD
            }
            #endif
            // This mechanism is quite simple and effective, but it is, well, obviously, heuristic.
            #if HTT
                TestRangeLo_CJ_Fw = High_Hat_Fw - HeuristicTestrangeTrimmingWidth;
                TestRangeHi_CJ_Fw = High_Hat_Fw + HeuristicTestrangeTrimmingWidth;
                TestRangeLo_Sw = High_Hat_Sw - HeuristicTestrangeTrimmingWidth;
                TestRangeHi_Sw = High_Hat_Sw + HeuristicTestrangeTrimmingWidth;
                TestRangeLo_Dw = High_Hat_Dw - HeuristicTestrangeTrimmingWidth;
                TestRangeHi_Dw = High_Hat_Dw + HeuristicTestrangeTrimmingWidth;
                if (TestRangeLo_CJ_Fw < 9) {
                    TestRangeLo_CJ_Fw = 9;
                }
                if (TestRangeLo_Sw < 9) {
                    TestRangeLo_Sw = 9;
                }
                if (TestRangeLo_Dw < 1) {
                    TestRangeLo_Dw = 1;
                }
                // Absoulte Jump Failure
                if (High_Hat_Fw == 0) {
                    TestRangeLo_CJ_Fw = 9;
                    TestRangeHi_CJ_Fw = 50;
                    TestRangeLo_Sw = 9;
                    TestRangeHi_Sw = 50;
                    TestRangeLo_Dw = 1;
                    TestRangeHi_Dw = 50;
                }
            #endif
            #if WD
                for (int i = 0; i <= TestRange_I_CJ_F_Amount+TestRange_I_CJ_S_Amount+TestRange_I_CJ_D_Amount; i++) {
                    fclose(wholedataoutput[i]);
                }
            #endif
        }
    #endif

    // Test Scooter Jump + DoubleJump

    #if SJ

        BaseVelX = 800.00L;
        PosX = 0.0L;
        // GravityMulti = 1.0L; // It won't change

        // Squeezing Out...
        
        for (VelX = 0.0L; VelX <= BaseVelX; VelX += 25.0L  ) {
            PosX += VelX / TicksperS;
            #if SSPXR
                printf("%.17Lg, ",VelX);
                printf("%.17Lg\n",PosX); 
            #endif
        }

        VelX = BaseVelX;
        PosX += VelX / TicksperS;
        #if SSPXR
            printf("%.17Lg, ",VelX);
            printf("%.17Lg\n",PosX);
        #endif

        RunPosX = PosX + LedgePosXErrBuf - VelX / TicksperS;
        StartPosX = LedgePosX - RunPosX;
        #if SSPXR
            printf("Recommended Start Pos X: %.17Lg\n", StartPosX );
        #endif

        #if WD
            wholedataoutput[WDScooterJumpPosBase] = fopen("WDoutput/HatPhysicsCalcOutputWholeDataScooterJump GrandMasterFile.csv", "a");
            fputs("SJ_GrandMasterfile_HeightDiff, ", wholedataoutput[WDScooterJumpPosBase]);
            fputs("SJ_GrandMasterfile_Hat_Gracew, ", wholedataoutput[WDScooterJumpPosBase]);
            fputs("SJ_GrandMasterfile_Hat_Fw, ", wholedataoutput[WDScooterJumpPosBase]);
            fputs("SJ_GrandMasterfile_Frame, ", wholedataoutput[WDScooterJumpPosBase]);
            fputs("SJ_GrandMasterfile_PosZ, ", wholedataoutput[WDScooterJumpPosBase]);
            fputs("SJ_GrandMasterfile_PosX\n", wholedataoutput[WDScooterJumpPosBase]);
        #endif
        #if PMF
            ScooterJumpProcessedGrandMasterFile = fopen("HatPhysicsCalcOutputWholeDataScooterJump ProcessedGrandMasterFile.csv", "a");
            fputs("SJ_ProcessedGrandMasterFile_HeightDiff, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Hat_Gracew, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Hat_Gracew_ADtO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Hat_Gracew_StO, ", ScooterJumpProcessedGrandMasterFile);            
            fputs("SJ_ProcessedGrandMasterFile_Hat_Fw, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Hat_Fw_ADtO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Hat_Fw_StO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Hat_Fw_ADtGWO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Hat_Fw_StGWO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Frame, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Frame_ADtO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Frame_StO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Frame_ADtGWO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_Frame_StGWO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_PosZ, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_PosX, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_PosX_ADtO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_PosX_StO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_PosX_ADtGWO, ", ScooterJumpProcessedGrandMasterFile);
            fputs("SJ_ProcessedGrandMasterFile_PosX_StGWO\n", ScooterJumpProcessedGrandMasterFile);
        #endif

        for (HeightDiff = TestRangeHeightDiffScooterJumpStart; HeightDiff >= TestRangeHeightDiffScooterJumpEnd; HeightDiff += HeightDiffChange ) {
            // High records are only for each HeightDiff.
            High_PosX = 0.0L;
            High_Hat_Fw = 0;
            High_Hat_Gracew = 0;
            High_Frame = 0;
            High_PosZ = 0.0L;

            #if PMF
                *pSJPGMFi = 0;                
                __mingw_snprintf(buf, sizeof buf, "HatPhysicsCalcOutputWholeDataScooterJump HeightDiff %.17Lg ProcessedMasterFile.csv", HeightDiff);
                ScooterJumpProcessedHeightDiffMasterFile = fopen(buf, "a");
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_HeightDiff, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Gracew, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Gracew_ADtO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Gracew_StO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Fw, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Fw_ADtO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Fw_StO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Fw_ADtGWO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Hat_Fw_StGWO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Frame, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Frame_ADtO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Frame_StO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Frame_ADtGWO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_Frame_StGWO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_PosZ, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_PosX, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_PosX_ADtO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_PosX_StO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_PosX_ADtGWO, ", HeightDiff);
                fprintf(ScooterJumpProcessedHeightDiffMasterFile,"SJ_HeightDiff_%.17Lg_ProcessedMasterFile_PosX_StGWO\n", HeightDiff);                
            #endif

            #if WD
                // ScooterJump is Masterfile and Gracew only, thus HTT can be turned on safely.
                __mingw_snprintf(buf, sizeof buf, "WDoutput/HatPhysicsCalcOutputWholeDataScooterJump HeightDiff %.17Lg MasterFile.csv", HeightDiff);
                wholedataoutput[WDScooterJumpPosBase+1] = fopen(buf, "a");
                fprintf(wholedataoutput[WDScooterJumpPosBase+1],"SJ_HeightDiff_%.17Lg_Masterfile_HeightDiff, ", HeightDiff);
                fprintf(wholedataoutput[WDScooterJumpPosBase+1],"SJ_HeightDiff_%.17Lg_Masterfile_Hat_Gracew, ", HeightDiff);
                fprintf(wholedataoutput[WDScooterJumpPosBase+1],"SJ_HeightDiff_%.17Lg_Masterfile_Hat_Fw, ", HeightDiff);
                fprintf(wholedataoutput[WDScooterJumpPosBase+1],"SJ_HeightDiff_%.17Lg_Masterfile_Frame, ", HeightDiff);
                fprintf(wholedataoutput[WDScooterJumpPosBase+1],"SJ_HeightDiff_%.17Lg_Masterfile_PosZ, ", HeightDiff);
                fprintf(wholedataoutput[WDScooterJumpPosBase+1],"SJ_HeightDiff_%.17Lg_Masterfile_PosX\n", HeightDiff);
                int i = WDScooterJumpPosBase + 1 + 1;
                for (Hat_Gracew = 0; Hat_Gracew <= TestRangeHi_Gracew; Hat_Gracew++ ) {
                    __mingw_snprintf(buf, sizeof buf, "WDoutput/HatPhysicsCalcOutputWholeDataScooterJump HeightDiff %.17Lg Gracew %i.csv", HeightDiff, Hat_Gracew);
                    wholedataoutput[i] = fopen(buf, "a");
                    fprintf(wholedataoutput[i],"SJ_HeightDiff_%.17Lg_Gracew_%i_HeightDiff, ", HeightDiff, Hat_Gracew);
                    fprintf(wholedataoutput[i],"SJ_HeightDiff_%.17Lg_Gracew_%i_Hat_Gracew, ", HeightDiff, Hat_Gracew);
                    fprintf(wholedataoutput[i],"SJ_HeightDiff_%.17Lg_Gracew_%i_Hat_Fw, ", HeightDiff, Hat_Gracew);
                    fprintf(wholedataoutput[i],"SJ_HeightDiff_%.17Lg_Gracew_%i_Frame, ", HeightDiff, Hat_Gracew);
                    fprintf(wholedataoutput[i],"SJ_HeightDiff_%.17Lg_Gracew_%i_PosZ, ", HeightDiff, Hat_Gracew);
                    fprintf(wholedataoutput[i],"SJ_HeightDiff_%.17Lg_Gracew_%i_PosX\n", HeightDiff, Hat_Gracew);
                    i++;
                }
            #endif
            for (Hat_Gracew = 0; Hat_Gracew <= TestRangeHi_Gracew; Hat_Gracew++ ) {
                if (HeightDiff <= 0.0L  && Trim_Hat_Gracew) {
                    Hat_Gracew = TestRangeHi_Gracew;
                }
                #if SPH || PGWHS
                    printf("Now calculating: Gracew = %i\n",Hat_Gracew);
                #endif
                #if ( PGWHF || PGWHS ) || PMF
                    GraceW_High_PosX[Hat_Gracew] = 0.0L;
                    GraceW_High_Hat_Fw[Hat_Gracew] = 0;
                    GraceW_High_Frame[Hat_Gracew] = 0;
                    GraceW_High_PosZ[Hat_Gracew] = 0.0L;
                #endif
                for (Hat_Fw = TestRangeLo_SJ_Fw; Hat_Fw <= TestRangeHi_SJ_Fw; Hat_Fw++ ) {
                    // Initializing Simulation
                    Frame = 0;
                    JumpRemain = 0.0L;
                    VelZ = 0.0L;
                    PosZ = 0.0L;
                    PosX = 0.0L;
                    // Scooter's Speed
                    VelX = 800.00L;
                    PosX = RunPosX - LedgePosXErrBuf + VelX / TicksperS;
                    do {
                        if ( Frame == Hat_Gracew ) {
                            // grace period ends, first jump commences.
                            JumpRemain=JumpZ*9.0L  /10.0L;
                            VelZ=JumpZ/10.0L;
                        }
                        if ( Frame == Hat_Gracew + Hat_Fw ) {
                            // DoubleJump commences.
                            // DoubleJump after scooter jump's (nominal) VelX really is AROUND 617.495.
                            //VelX = 617.495L;
                            // Yet the REAL VelX for the first tick really is AROUND 608.8.
                            VelX = 608.8L;
                            JumpRemain=JumpZ*9.0L  /10.0L;
                            VelZ=JumpZ/10.0L;
                        }

                        // ApplyJumpHold, adapted right from the game's src!
                        a = JumpRemain*13.0L  /TicksperS;
                        VelZ += a*1.22L; // * (((1/TicksperS)-0.01666)*-2.5 + 1);
                        JumpRemain -= a;

                        // Gravity. I guess it should be ( GravityZ / 60.0 ) per tick, but whatever. I didn't make A Hat in Time.
                        VelZ += GravityZ / 60.0L  * 2.0L;//  * GravityMulti; //1000.0/TicksperS;                            

                        // Position calculation happens quite late.
                        PosX += VelX / TicksperS;

                        // This (GravityZ*-1/60) is REAL
                        // PosZ += ( VelZ + ( GravityZ * -1.0L  / 60.0L  * GravityMulti ) ) / TicksperS;
                        PosZ += ( VelZ + ( GravityZ * -1.0L  / 60.0L ) ) / TicksperS;

                        if ( Frame == Hat_Gracew + Hat_Fw ) {
                            // (kinda) Real value goes here.
                            VelX = 617.495L;
                        }                        

                        // Simulation Data Output 1

                        /*
                        
                        if ( HeightDiff == -3000.0L && Hat_Gracew == 9 && Hat_Fw == 146 )
                        {
                            printf("%.17Lg, ",VelX);
                            printf("%.17Lg, ",PosX + StartPosX);
                            printf("(%.17Lg), ",PosX);
                            printf("%.17Lg, ",VelZ);
                            printf("%.17Lg, ",PosZ + BasePosZ);
                            printf("(%.17Lg)\n",PosZ);
                        }

                        */
                        
                        if ( Frame >= Hat_Gracew + Hat_Fw && PosZ <= WallSlideThreshold + HeightDiff && VelZ < -12.5L  ) {
                            // We shouldn't go too low; it will be a failed jump.
                            break;
                        }

                        Frame++; 

                    } while ( true );  

                    // undo the last tick since the last tick is where Hat_Player should NOT be at.
                    PosX -= VelX/TicksperS;
                    PosZ -= (VelZ+12.5L  )/TicksperS;

                    PosX += WallSlideInitiationMagicNumber;

                    // Compensating RunPosX
                    PosX += - RunPosX;

                    if (PosZ >= WallSlideThreshold + HeightDiff) {

                        #if ( PGWHF || PGWHS ) || PMF

                        if (GraceW_High_PosX[Hat_Gracew] < PosX) {
                            GraceW_High_PosX[Hat_Gracew] = PosX;
                            GraceW_High_Hat_Fw[Hat_Gracew] = Hat_Fw;
                            GraceW_High_Frame[Hat_Gracew] = Frame;
                            GraceW_High_PosZ[Hat_Gracew] = PosZ;

                        #endif

                            if (High_PosX < PosX) {
                                // We have a new highscore! Bring a fresh bottle of champagne... wait peck no.
                                High_PosX = PosX;
                                High_Hat_Fw = Hat_Fw;
                                High_Hat_Gracew = Hat_Gracew;
                                High_Frame = Frame;
                                High_PosZ = PosZ;
                                #if SPH
                                    printf("A Preliminary High ( HeightDiff: %.17Lg ), ",HeightDiff);
                                    printf("%i, ",Hat_Gracew);
                                    printf("%i, ",Hat_Fw);
                                    printf("%i, ",Frame);
                                    printf("%.17Lg, ",PosZ);
                                    printf("%.17Lg\n",PosX);
                                #endif
                            }

                        #if ( PGWHF || PGWHS ) || PMF

                        }

                        #endif

                        #if WD
                            fprintf(wholedataoutput[WDScooterJumpPosBase], "%.17Lg, ",HeightDiff);
                            fprintf(wholedataoutput[WDScooterJumpPosBase], "%i, ",Hat_Gracew);
                            fprintf(wholedataoutput[WDScooterJumpPosBase], "%i, ",Hat_Fw);
                            fprintf(wholedataoutput[WDScooterJumpPosBase], "%i, ",Frame);
                            fprintf(wholedataoutput[WDScooterJumpPosBase], "%.17Lg, ",PosZ);
                            fprintf(wholedataoutput[WDScooterJumpPosBase], "%.17Lg\n",PosX);

                            fprintf(wholedataoutput[WDScooterJumpPosBase+1], "%.17Lg, ",HeightDiff);
                            fprintf(wholedataoutput[WDScooterJumpPosBase+1], "%i, ",Hat_Gracew);
                            fprintf(wholedataoutput[WDScooterJumpPosBase+1], "%i, ",Hat_Fw);
                            fprintf(wholedataoutput[WDScooterJumpPosBase+1], "%i, ",Frame);
                            fprintf(wholedataoutput[WDScooterJumpPosBase+1], "%.17Lg, ",PosZ);
                            fprintf(wholedataoutput[WDScooterJumpPosBase+1], "%.17Lg\n",PosX);

                            int WDGracewPos = WDScooterJumpPosBase + 1 + 1 + Hat_Gracew;
                            fprintf(wholedataoutput[WDGracewPos], "%.17Lg, ",HeightDiff);
                            fprintf(wholedataoutput[WDGracewPos], "%i, ",Hat_Gracew);
                            fprintf(wholedataoutput[WDGracewPos], "%i, ",Hat_Fw);
                            fprintf(wholedataoutput[WDGracewPos], "%i, ",Frame);
                            fprintf(wholedataoutput[WDGracewPos], "%.17Lg, ",PosZ);
                            fprintf(wholedataoutput[WDGracewPos], "%.17Lg\n",PosX);
                        #endif
                        #if PMF
                                SJPGMFData[*pSJPGMFi].Frame = Frame;
                                SJPGMFData[*pSJPGMFi].PosZ = PosZ;
                                SJPGMFData[*pSJPGMFi].PosX = PosX;
                                // Don't use *pSJPGMFi++. The ++ operation PRECEDES the dereference operation(*),
                                // doing nothing but ruining the pointer and returning... something unexpected.
                                *pSJPGMFi += 1;
                        #endif
                    }
                    #if PMF
                        else {
                            SJPGMFData[*pSJPGMFi].Frame = 0;
                            *pSJPGMFi += 1;
                        }
                    #endif
                }
                #if TEHGW
                if ( GraceW_High_Frame[Hat_Gracew] != 0) {
                #endif
                    #if PGWHS
                        printf("Gracew %i High ( HeightDiff: %.17Lg ), ", Hat_Gracew, HeightDiff);
                        printf("%i, ",GraceW_High_Hat_Fw[Hat_Gracew]);
                        printf("%i, ",GraceW_High_Frame[Hat_Gracew]);
                        printf("%.17Lg, ",GraceW_High_PosZ[Hat_Gracew]);
                        printf("%.17Lg\n",GraceW_High_PosX[Hat_Gracew]);
                    #endif
                    #if PGWHF
                        fprintf(PGWHF_output,"%.17Lg, ",HeightDiff);
                        fprintf(PGWHF_output,"%i, ",Hat_Gracew);
                        fprintf(PGWHF_output,"%i, ",GraceW_High_Hat_Fw[Hat_Gracew]);
                        fprintf(PGWHF_output,"%i, ",GraceW_High_Frame[Hat_Gracew]);
                        fprintf(PGWHF_output,"%.17Lg, ",GraceW_High_PosZ[Hat_Gracew]);
                        fprintf(PGWHF_output,"%.17Lg\n",GraceW_High_PosX[Hat_Gracew]);
                    #endif
                #if TEHGW
                }
                #endif
            }
            #if TEHHD
            if ( High_Frame != 0) {
            #endif
                #if PHF
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", HeightDiff);
                    //fprintf(output,"%s, ",buf);
                    fprintf(output,"%.17Lg, ",HeightDiff);
                    fprintf(output,"%i, ",High_Hat_Gracew);
                    fprintf(output,"%i, ",High_Hat_Fw);
                    fprintf(output,"%i, ",High_Frame);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosZ);
                    //printf("%s, ",buf);
                    fprintf(output,"%.17Lg, ",High_PosZ);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosX);
                    //printf("%s\n",buf);
                    fprintf(output,"%.17Lg\n",High_PosX);
                #endif
                #if PHS
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", HeightDiff);
                    //printf("%s, ",buf);
                    printf("%.17Lg, ",HeightDiff);
                    printf("%i, ",High_Hat_Gracew);
                    printf("%i, ",High_Hat_Fw);
                    printf("%i, ",High_Frame);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosZ);
                    //printf("%s, ",buf);
                    printf("%.17Lg, ",High_PosZ);
                    //quadmath_snprintf(buf, sizeof buf, "%.36Qg", High_PosX);
                    //printf("%s\n",buf);
                    printf("%.17Lg\n",High_PosX);
                #endif
            #if TEHHD
            }
            #endif
			
			// WHY DID I MAKE THIS?! THIS IS MADNESS WTF
            #if PMF
                #if HTT
                    TestRange_SJ_F_Amount = TestRangeHi_SJ_Fw - TestRangeLo_SJ_Fw + 1;
                    SJPGMFi = 0;
                    for (Hat_Gracew = 0; Hat_Gracew <= TestRangeHi_Gracew; Hat_Gracew++ ) {
                        for (Hat_Fw = TestRangeLo_SJ_Fw; Hat_Fw <= TestRangeHi_SJ_Fw; Hat_Fw++ ) {
                            if ( SJPGMFData[SJPGMFi].Frame != 0 ) {
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", HeightDiff );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", Hat_Gracew );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", Hat_Gracew - High_Hat_Gracew );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", (float)Hat_Gracew / (float)High_Hat_Gracew );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", Hat_Fw + TestRangeLo_SJ_Fw );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", Hat_Fw + TestRangeLo_SJ_Fw - High_Hat_Fw );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", ( (float)Hat_Fw + (float)TestRangeLo_SJ_Fw ) / (float)High_Hat_Fw );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", Hat_Fw + TestRangeLo_SJ_Fw - GraceW_High_Hat_Fw[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", ( (float)Hat_Fw + (float)TestRangeLo_SJ_Fw ) / (float)GraceW_High_Hat_Fw[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", SJPGMFData[SJPGMFi].Frame );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", ( SJPGMFData[SJPGMFi].Frame - High_Frame ) );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi].Frame / (float)High_Frame );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", ( SJPGMFData[SJPGMFi].Frame - GraceW_High_Frame[Hat_Gracew] ) );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi].Frame / (float)GraceW_High_Frame[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosZ );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosX );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosX - High_PosX );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg,", SJPGMFData[SJPGMFi].PosX / High_PosX );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosX - GraceW_High_PosX[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg\n", SJPGMFData[SJPGMFi].PosX / GraceW_High_PosX[Hat_Gracew] );

                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", HeightDiff );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", Hat_Gracew );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", Hat_Gracew - High_Hat_Gracew );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", (float)Hat_Gracew / (float)High_Hat_Gracew );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", Hat_Fw + TestRangeLo_SJ_Fw );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", Hat_Fw + TestRangeLo_SJ_Fw - High_Hat_Fw );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", ( (float)Hat_Fw + (float)TestRangeLo_SJ_Fw ) / (float)High_Hat_Fw );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", Hat_Fw + TestRangeLo_SJ_Fw - GraceW_High_Hat_Fw[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", ( (float)Hat_Fw + (float)TestRangeLo_SJ_Fw ) / (float)GraceW_High_Hat_Fw[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", SJPGMFData[SJPGMFi].Frame );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", ( SJPGMFData[SJPGMFi].Frame - High_Frame ) );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi].Frame / (float)High_Frame );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", ( SJPGMFData[SJPGMFi].Frame - GraceW_High_Frame[Hat_Gracew] ) );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi].Frame / (float)GraceW_High_Frame[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosZ );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosX );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosX - High_PosX );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg,", SJPGMFData[SJPGMFi].PosX / High_PosX );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi].PosX - GraceW_High_PosX[Hat_Gracew] );
                                fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg\n", SJPGMFData[SJPGMFi].PosX / GraceW_High_PosX[Hat_Gracew] );
                            }
                            SJPGMFi++;
                        }
                    }
                #else
                    for (SJPGMFi.a = 0; SJPGMFi.a < ( TestRangeHi_Gracew + 1 ) * TestRange_I_SJ_F_Amount; SJPGMFi.a++ ) {
                        if ( SJPGMFData[SJPGMFi.a].Frame != 0 ) {
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", HeightDiff );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", SJPGMFi.b[1] );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", SJPGMFi.b[1] - High_Hat_Gracew );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", (float)SJPGMFi.b[1] / (float)High_Hat_Gracew );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", SJPGMFi.b[0] + TestRangeLo_SJ_Fw );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", SJPGMFi.b[0] + TestRangeLo_SJ_Fw - High_Hat_Fw );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", ( (float)SJPGMFi.b[0] + (float)TestRangeLo_SJ_Fw ) / (float)High_Hat_Fw );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", SJPGMFi.b[0] + TestRangeLo_SJ_Fw - GraceW_High_Hat_Fw[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", ( (float)SJPGMFi.b[0] + (float)TestRangeLo_SJ_Fw ) / (float)GraceW_High_Hat_Fw[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", SJPGMFData[SJPGMFi.a].Frame );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", ( SJPGMFData[SJPGMFi.a].Frame - High_Frame ) );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi.a].Frame / (float)High_Frame );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%i, ", ( SJPGMFData[SJPGMFi.a].Frame - GraceW_High_Frame[SJPGMFi.b[1]] ) );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi.a].Frame / (float)GraceW_High_Frame[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosZ );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX - High_PosX );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX / High_PosX );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX - GraceW_High_PosX[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedGrandMasterFile, "%.17Lg\n", SJPGMFData[SJPGMFi.a].PosX / GraceW_High_PosX[SJPGMFi.b[1]] );

                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", HeightDiff );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", SJPGMFi.b[1] );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", SJPGMFi.b[1] - High_Hat_Gracew );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", (float)SJPGMFi.b[1] / (float)High_Hat_Gracew );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", SJPGMFi.b[0] + TestRangeLo_SJ_Fw );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", SJPGMFi.b[0] + TestRangeLo_SJ_Fw - High_Hat_Fw );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", ( (float)SJPGMFi.b[0] + (float)TestRangeLo_SJ_Fw ) / (float)High_Hat_Fw );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", SJPGMFi.b[0] + TestRangeLo_SJ_Fw - GraceW_High_Hat_Fw[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", ( (float)SJPGMFi.b[0] + (float)TestRangeLo_SJ_Fw ) / (float)GraceW_High_Hat_Fw[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", SJPGMFData[SJPGMFi.a].Frame );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", ( SJPGMFData[SJPGMFi.a].Frame - High_Frame ) );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi.a].Frame / (float)High_Frame );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%i, ", ( SJPGMFData[SJPGMFi.a].Frame - GraceW_High_Frame[SJPGMFi.b[1]] ) );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.6g, ", (float)SJPGMFData[SJPGMFi.a].Frame / (float)GraceW_High_Frame[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosZ );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX - High_PosX );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX / High_PosX );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg, ", SJPGMFData[SJPGMFi.a].PosX - GraceW_High_PosX[SJPGMFi.b[1]] );
                            fprintf(ScooterJumpProcessedHeightDiffMasterFile, "%.17Lg\n", SJPGMFData[SJPGMFi.a].PosX / GraceW_High_PosX[SJPGMFi.b[1]] );
                        }
                    }
                #endif
                fclose(ScooterJumpProcessedHeightDiffMasterFile);
            #endif
			
            #if HTT
                TestRangeLo_SJ_Fw = High_Hat_Fw - HeuristicTestrangeTrimmingWidth;
                TestRangeHi_SJ_Fw = High_Hat_Fw + HeuristicTestrangeTrimmingWidth;
                if (TestRangeLo_SJ_Fw < 9) {
                    TestRangeLo_SJ_Fw = 9;
                }
                // Absoulte Jump Failure
                if (High_Hat_Fw == 0) {
                    TestRangeLo_SJ_Fw = 9;
                    TestRangeHi_SJ_Fw = 50;
                }
            #endif
            #if WD
                fclose(wholedataoutput[WDScooterJumpPosBase+1]);
                for (Hat_Gracew = 0; Hat_Gracew <= TestRangeHi_Gracew; Hat_Gracew++ ) {
                    fclose(wholedataoutput[WDScooterJumpPosBase+1+1+Hat_Gracew]);
                }
            #endif
        }
        #if WD
            fclose(wholedataoutput[WDScooterJumpPosBase]);
        #endif
        #if PMF
            fclose(ScooterJumpProcessedGrandMasterFile);
        #endif
    #endif
    #if PHF
        fclose(output);
    #endif
    #if PGWHF
        fclose(PGWHF_output);
    #endif
}