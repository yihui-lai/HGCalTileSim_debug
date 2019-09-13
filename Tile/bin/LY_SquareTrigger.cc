#include "HGCalTileSim/Tile/interface/LYSimAnalysis.hh"
#include "HGCalTileSim/Tile/interface/LYSimDetectorConstruction.hh"
#include "HGCalTileSim/Tile/interface/LYSimPhysicsList.hh"
#include "HGCalTileSim/Tile/interface/LYSimPrimaryGeneratorAction.hh"
#include "HGCalTileSim/Tile/interface/LYSimSteppingAction.hh"
#include "HGCalTileSim/Tile/interface/LYSimTrackingAction.hh"

#include "UserUtils/Common/interface/ArgumentExtender.hpp"

#include "G4RunManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"

int
main( int argc, char** argv )
{
  usr::po::options_description desc( "Running a run with a certain geometry" );
  desc.add_options()
    ( "beamx,x", usr::po::defvalue<double>( 0 ), "x center of beam [mm]" )
    ( "beamy,y", usr::po::defvalue<double>( 0 ), "y center of beam [mm]" )
    ( "beamwidth,w", usr::po::defvalue<double>( 0 ), "width of beam [mm]" )
    ( "dimplerad,r", usr::po::defvalue<double>( 3.0 ), "Dimple radius [mm]" )
    ( "dimpleind,d", usr::po::defvalue<double>( 1.5 ), "Dimple indent [mm]" )
    ( "absmult,a",   usr::po::defvalue<double>( 10 ),
    "Multple of inbuilt absorption length" )
    ( "NEvents,N", usr::po::defvalue<unsigned>( 1 ), "Number of events to run" )
    ( "output,o", usr::po::defvalue<std::string>( "test.root" ), "output file" )
  ;

  usr::ArgumentExtender args;
  args.AddOptions( desc );
  args.ParseOptions( argc, argv );

  const double x_center      = args.Arg<double>( "beamx" );
  const double y_center      = args.Arg<double>( "beamy" );
  const double width         = args.Arg<double>( "beamwidth" );
  const double dimplerad     = args.Arg<double>( "dimplerad" );
  const double dimpleind     = args.Arg<double>( "dimpleind" );
  const unsigned N           = args.Arg<unsigned>( "NEvents" );
  const std::string filename = args.Arg<std::string>( "output" );

  // Must initialize Run Manager first
  G4RunManager* runManager   = new G4RunManager;
  LYSimPhysicsList* physlist = new LYSimPhysicsList();
  // Overriding the detector parameters
  LYSimDetectorConstruction* detector = new LYSimDetectorConstruction();
  detector->SetDimpleRadius( dimplerad );
  detector->SetDimpleIndent( dimpleind );

  runManager->SetUserInitialization( detector );
  runManager->SetUserInitialization( physlist );

  // Overriding the generator parameters
  LYSimPrimaryGeneratorAction* genaction
    = new LYSimPrimaryGeneratorAction( detector );
  genaction->SetBeamX( x_center );
  genaction->SetBeamY( y_center );
  genaction->SetWidth( width );

  // Construct LYSimAnalysis class
  LYSimAnalysis::GetInstance()->SetDetector( detector );
  LYSimAnalysis::GetInstance()->SetOutputFile( filename );
  LYSimAnalysis::GetInstance()->SetGeneratorAction( genaction );


  runManager->SetUserAction( genaction );
  runManager->SetUserAction( new LYSimAnalysis::RunAction() );
  runManager->SetUserAction( new LYSimAnalysis::EventAction() );
  runManager->SetUserAction( new LYSimTrackingAction() );
  runManager->SetUserAction( new LYSimSteppingAction() );

  // Preparing the LYSimAnalysis
  LYSimAnalysis::GetInstance()->PrepareExperiment();

  // Initialize G4 kernel
  runManager->Initialize();
  G4VisManager* visManager = new G4VisExecutive( "Quiet" );
  visManager->Initialize();
  G4UImanager* UIManager = G4UImanager::GetUIpointer();

  char cmd[1024];
  UIManager->ApplyCommand( "/control/verbose 1" );
  UIManager->ApplyCommand( "/run/verbose 0" );
  UIManager->ApplyCommand( "/process/setVerbose 0" );
  UIManager->ApplyCommand( "/tracking/verbose 0" );

  srand( time( NULL ) );

  sprintf( cmd, "/random/setSeeds %d %d", rand(), rand() );
  UIManager->ApplyCommand( cmd );

  runManager->BeamOn( N );

  LYSimAnalysis::GetInstance()->EndOfExperiment();

  // Job termination Free the store: user actions, physics_list and
  // detector_description are owned and deleted by the run manager, so they
  // should not be deleted in the main() program !
  delete visManager;
  delete runManager;

  return 0;
}
