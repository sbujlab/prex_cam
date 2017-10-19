#include "remollPrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4GenericMessenger.hh"

#include "remollIO.hh"
#include "remollVEventGen.hh"
#include "remollEvent.hh"
#include "remollRun.hh"
#include "remollRunData.hh"
#include "remolltypes.hh"
#include "globals.hh"

#include "remollGenMoller.hh"
#include "remollGenpElastic.hh"
#include "remollGenpInelastic.hh"
#include "remollGenPion.hh"
#include "remollGenBeam.hh"
#include "remollGenFlat.hh"
#include "remollGenAl.hh"
#include "remollGenLUND.hh"
#include "remollGenPrex.hh"

remollPrimaryGeneratorAction::remollPrimaryGeneratorAction()
: fParticleGun(0),fEventGen(0),fEvent(0),fMessenger(0)
{
    // Default generator
    G4String default_generator = "moller";
    SetGenerator(default_generator);

    // Get the particle gun
    fParticleGun = fEventGen->GetParticleGun();

    // Create generic messenger
    fMessenger = new G4GenericMessenger(this,"/remoll/","Remoll properties");
    fMessenger->DeclareMethod("gen",&remollPrimaryGeneratorAction::SetGenerator,"Select physics generator");
}

remollPrimaryGeneratorAction::~remollPrimaryGeneratorAction()
{
    if (fMessenger) delete fMessenger;
    if (fEventGen)  delete fEventGen;
}

void remollPrimaryGeneratorAction::SetGenerator(G4String& genname)
{
    // Delete previous generator
    if (fEventGen) {
      delete fEventGen;
      fEventGen = 0;
    }

    // Create new generator
    if( genname == "moller" ) {
        fEventGen = new remollGenMoller();
    }else if( genname == "elastic" ) {
        fEventGen = new remollGenpElastic();
    }else if( genname == "inelastic" ) {
        fEventGen = new remollGenpInelastic();
    }else if( genname == "pion" ) {
        fEventGen = new remollGenPion();
    }else if( genname == "beam" ) {
        fEventGen = new remollGenBeam();
    }else if( genname == "flat" ) {
        fEventGen = new remollGenFlat();
    }else if( genname == "inelasticAl" ) {
        fEventGen = new remollGenAl(2);
    }else if( genname == "quasielasticAl" ) {
        fEventGen = new remollGenAl(1);
    }else if( genname == "elasticAl" ) {
        fEventGen = new remollGenAl(0);
    }else if( genname == "pion_LUND" ) {
        fEventGen = new remollGenLUND();  
    }else if( genname == "prex" ) {
        fEventGen = new remollGenPrex();
    }

    if( !fEventGen ) {
        G4cerr << __FILE__ << " line " << __LINE__ << " - ERROR generator " << genname << " invalid" << G4endl;
        exit(1);
    } else {
        G4cout << "Setting generator to " << genname << G4endl;
    }

    // Get the particle gun
    fParticleGun = fEventGen->GetParticleGun();

    remollRun::GetRunData()->SetGenName(genname.data());
}

void remollPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    if (!fEventGen) {
      G4cerr << __FILE__ << " line " << __LINE__ << " - No event generator found." << G4endl;
      exit(1);
    }

    // Delete old primary event
    if (fEvent) {
      delete fEvent;
      fEvent = 0;
    }

    // Create new primary event
    fEvent = fEventGen->GenerateEvent();
    for (unsigned int pidx = 0; pidx < fEvent->fPartType.size(); pidx++) {

        double p = fEvent->fPartMom[pidx].mag();
        double m = fEvent->fPartType[pidx]->GetPDGMass();
        double kinE = sqrt(p*p + m*m) - m;

        fParticleGun->SetParticleDefinition(fEvent->fPartType[pidx]);
        fParticleGun->SetParticleEnergy(kinE);
        fParticleGun->SetParticlePosition(fEvent->fPartPos[pidx]);
        fParticleGun->SetParticleMomentumDirection(fEvent->fPartMom[pidx].unit());

        fParticleGun->GeneratePrimaryVertex(anEvent);
    }
}
