#include "remollGenPrex.hh"

#include "Randomize.hh"

#include "G4Material.hh"
#include "G4PhysicalConstants.hh"

#include "remollEvent.hh"
#include "remollVertex.hh"
#include "remolltypes.hh"

remollGenPrex::remollGenPrex()
: remollVEventGen("prex") {
    
  fApplyMultScatt = true;

}

remollGenPrex::~remollGenPrex(){
}

void remollGenPrex::SamplePhysics(remollVertex *vert, remollEvent *evt){
  
  // Generate Prex event
  //For 5 by 5 mm raster
  G4double vertex_x = (-0.25*cm + (0.5*cm*G4RandFlat::shoot(0.0,1.0)));
  G4double vertex_y = (-0.25*cm + (0.5*cm*G4RandFlat::shoot(0.0,1.0)));
  G4double z_0 = 1666.7*cm;

  G4double vertex_z = -105.3*cm - z_0;

  G4double R, unitX,unitY,unitZ;

  R = sqrt(vertex_x*vertex_x + vertex_y*vertex_y + z_0*z_0);
  unitX = vertex_x/R;
  unitY = vertex_y/R;
  unitZ = z_0/R;

  G4double beamE = vert->GetBeamEnergy();
  G4double me    = electron_mass_c2;
  G4double beamP = sqrt(beamE*beamE - me*me);
  G4double beamPx = unitX*beamP;
  G4double beamPy = unitY*beamP;
  G4double beamPz = unitZ*beamP;

  evt->fBeamE = beamE;
  evt->fBeamMomentum = evt->fBeamMomentum.unit()*beamP;
  evt->ProduceNewParticle( G4ThreeVector(0.0, 0.0, vertex_z),G4ThreeVector(beamPx,beamPy,beamPz),"e-");
  evt->SetEffCrossSection(0.0);
  evt->SetAsymmetry(0.0);
  evt->SetQ2(0.0);
  evt->SetW2(0.0);


  return;

}
