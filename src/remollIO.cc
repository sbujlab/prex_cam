#include "remollIO.hh"

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>

#include "G4ParticleDefinition.hh"
#include "G4GenericMessenger.hh"

#include "remollGenericDetectorHit.hh"
#include "remollGenericDetectorSum.hh"
#include "remollEvent.hh"
#include "remollRun.hh"
#include "remollRunData.hh"
#include "remollBeamTarget.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMNode.hpp>

// Singleton
remollIO* remollIO::gInstance = 0;
remollIO* remollIO::GetInstance() {
  if (gInstance == 0) {
    gInstance = new remollIO();
  }
  return gInstance;
}

remollIO::remollIO()
: fFile(0),fTree(0),fFilename("remollout.root")
{
    //  Set arrays to 0
    fNGenDetHit = 0;
    fNGenDetSum = 0;

    InitializeTree();

    // Create generic messenger
    fMessenger = new G4GenericMessenger(this,"/remoll/","Remoll properties");
    fMessenger->DeclareProperty("filename",fFilename,"Output filename");
}

remollIO::~remollIO()
{
    // Delete tree
    if (fTree) {
        delete fTree;
        fTree = NULL;
    }
    // Delete file
    if (fFile) {
        delete fFile;
        fFile = NULL;
    }

    delete fMessenger;
}

void remollIO::InitializeTree()
{
    if (fFile) {
        fFile->Close();
        delete fFile;
        fFile = NULL;
        fTree = NULL;
    }

    if (fTree) {
        delete fTree;
        fTree = NULL;
    }

    fFile = new TFile(fFilename, "RECREATE");

    fTree = new TTree("T", "Geant4 Moller Simulation");

    //fTree->SetMaxTreeSize(1900000000); // 1.9GB

    // Event information
    fTree->Branch("rate",     &fEvRate,   "rate/D");
    fTree->Branch("ev.A",     &fEvAsym,   "ev.A/D");
    fTree->Branch("ev.Am",    &fEvmAsym,  "ev.Am/D");
    fTree->Branch("ev.xs",    &fEvEffXS,  "ev.xs/D");
    fTree->Branch("ev.Q2",    &fEvQ2,     "ev.Q2/D");
    fTree->Branch("ev.W2",    &fEvW2,     "ev.W2/D");
    fTree->Branch("ev.thcom", &fEvThCoM,  "ev.thcom/D");
    fTree->Branch("ev.beamp", &fEvBeamP,  "ev.beamp/D");
    fTree->Branch("ev.seed",  &fEvSeed,   "ev.seed/C");

    fTree->Branch("ev.npart", &fNEvPart   ,     "ev.npart/I");
    fTree->Branch("ev.pid",   &fEvPID,      "ev.pid[ev.npart]/I");
    fTree->Branch("ev.vx",    &fEvPart_X,   "ev.vx[ev.npart]/D");
    fTree->Branch("ev.vy",    &fEvPart_Y,   "ev.vy[ev.npart]/D");
    fTree->Branch("ev.vz",    &fEvPart_Z,   "ev.vz[ev.npart]/D");
    fTree->Branch("ev.p",     &fEvPart_P,   "ev.p[ev.npart]/D");
    fTree->Branch("ev.px",    &fEvPart_Px,  "ev.px[ev.npart]/D");
    fTree->Branch("ev.py",    &fEvPart_Py,  "ev.py[ev.npart]/D");
    fTree->Branch("ev.pz",    &fEvPart_Pz,  "ev.pz[ev.npart]/D");
    fTree->Branch("ev.th",    &fEvPart_Th,     "ev.th[ev.npart]/D");
    fTree->Branch("ev.ph",    &fEvPart_Ph,     "ev.ph[ev.npart]/D");
    fTree->Branch("ev.tpx",    &fEvPart_tPx,  "ev.tpx[ev.npart]/D");
    fTree->Branch("ev.tpy",    &fEvPart_tPy,  "ev.tpy[ev.npart]/D");
    fTree->Branch("ev.tpz",    &fEvPart_tPz,  "ev.tpz[ev.npart]/D");

    fTree->Branch("bm.x",    &fBmX,  "bm.x/D");
    fTree->Branch("bm.y",    &fBmY,  "bm.y/D");
    fTree->Branch("bm.z",    &fBmZ,  "bm.z/D");
    fTree->Branch("bm.dx",    &fBmdX,  "bm.dx/D");
    fTree->Branch("bm.dy",    &fBmdY,  "bm.dy/D");
    fTree->Branch("bm.dz",    &fBmdZ,  "bm.dz/D");
    fTree->Branch("bm.th",    &fBmth,  "bm.th/D");
    fTree->Branch("bm.ph",    &fBmph,  "bm.ph/D");

    // GenericDetectorHit
    fTree->Branch("hit.n",    &fNGenDetHit,     "hit.n/I");
    fTree->Branch("hit.det",  &fGenDetHit_det,  "hit.det[hit.n]/I");
    fTree->Branch("hit.vid",  &fGenDetHit_id,   "hit.vid[hit.n]/I");

    fTree->Branch("hit.pid",  &fGenDetHit_pid,   "hit.pid[hit.n]/I");
    fTree->Branch("hit.trid", &fGenDetHit_trid,  "hit.trid[hit.n]/I");
    fTree->Branch("hit.mtrid",&fGenDetHit_mtrid, "hit.mtrid[hit.n]/I");
    fTree->Branch("hit.gen",  &fGenDetHit_gen,   "hit.gen[hit.n]/I");

    fTree->Branch("hit.x",    &fGenDetHit_X,   "hit.x[hit.n]/D");
    fTree->Branch("hit.y",    &fGenDetHit_Y,   "hit.y[hit.n]/D");
    fTree->Branch("hit.z",    &fGenDetHit_Z,   "hit.z[hit.n]/D");
    fTree->Branch("hit.r",    &fGenDetHit_R,   "hit.r[hit.n]/D");
    fTree->Branch("hit.ph",   &fGenDetHit_Ph,  "hit.ph[hit.n]/D");

    fTree->Branch("hit.px",   &fGenDetHit_Px,   "hit.px[hit.n]/D");
    fTree->Branch("hit.py",   &fGenDetHit_Py,   "hit.py[hit.n]/D");
    fTree->Branch("hit.pz",   &fGenDetHit_Pz,   "hit.pz[hit.n]/D");

    fTree->Branch("hit.vx",   &fGenDetHit_Vx,   "hit.vx[hit.n]/D");
    fTree->Branch("hit.vy",   &fGenDetHit_Vy,   "hit.vy[hit.n]/D");
    fTree->Branch("hit.vz",   &fGenDetHit_Vz,   "hit.vz[hit.n]/D");

    fTree->Branch("hit.p",    &fGenDetHit_P,   "hit.p[hit.n]/D");
    fTree->Branch("hit.e",    &fGenDetHit_E,   "hit.e[hit.n]/D");
    fTree->Branch("hit.m",    &fGenDetHit_M,   "hit.m[hit.n]/D");

    fTree->Branch("hit.colCut",    &fCollCut,     "hit.colCut/I");

    // GenericDetectorSum
    fTree->Branch("sum.n",    &fNGenDetSum,     "sum.n/I");
    fTree->Branch("sum.det",  &fGenDetSum_det,  "sum.det[sum.n]/I");
    fTree->Branch("sum.vid",  &fGenDetSum_id,   "sum.vid[sum.n]/I");
    fTree->Branch("sum.edep", &fGenDetSum_edep, "sum.edep[sum.n]/D");

    G4cout << "Initialized tree." << G4endl;
}

void remollIO::FillTree()
{
    if( !fTree ){
        G4cerr << "Error " << __PRETTY_FUNCTION__ << ": "
            << __FILE__ <<  " line " << __LINE__
            << " - Trying to fill non-existent tree" << G4endl;
        return;
    }

    fTree->Fill();
    fTree->GetCurrentFile();
}

void remollIO::Flush()
{
    //  Set arrays to 0
    fNGenDetHit = 0;
    fNGenDetSum = 0;
    fCollCut = 1; // default
}

void remollIO::WriteTree()
{
    if (!fFile)
      return;

    if (!fFile->IsOpen()) {
        G4cerr << "ERROR: " << __FILE__ << " line " << __LINE__ << ": TFile not open" << G4endl;
        exit(1);
    }

    G4cout << "Writing output to " << fFile->GetName() << "... ";

    fFile->cd();

    fTree->Write("T", TObject::kOverwrite);
    remollRun::GetRunData()->Write("run_data", TObject::kOverwrite);

    fTree->ResetBranchAddresses();
    delete fTree;
    fTree = NULL;

    fFile->Close();

    delete fFile;
    fFile = NULL;

    G4cout << "written" << G4endl;
}

///////////////////////////////////////////////////////////////////////////////
// Interfaces to output section ///////////////////////////////////////////////

// Event seed
void remollIO::SetEventSeed(const G4String& seed)
{
  fEvSeed = seed;
}


// Event Data

void remollIO::SetEventData(const remollEvent *ev)
{
    int n = ev->fPartType.size();
    if( n > __IO_MAXHIT ){
        G4cerr << "WARNING: " << __PRETTY_FUNCTION__ << " line " << __LINE__ << ":  Buffer size exceeded!" << G4endl;
        return;
    }

    fNEvPart = n;

    fEvRate   = ev->fRate*s;
    fEvEffXS  = ev->fEffXs/microbarn;
    fEvAsym   = ev->fAsym/__ASYMM_SCALE;
    fEvmAsym  = ev->fmAsym/__ASYMM_SCALE;
    fEvBeamP  = ev->fBeamMomentum.mag()/__E_UNIT;

    fEvQ2     = ev->fQ2/__E_UNIT/__E_UNIT;
    fEvW2     = ev->fW2/__E_UNIT/__E_UNIT;
    fEvThCoM  = ev->fThCoM/deg; // specify this in degrees over anything else

    int idx;
    for( idx = 0; idx < n; idx++ ){
        fEvPID[idx] = ev->fPartType[idx]->GetPDGEncoding();

        fEvPart_X[idx] = ev->fPartPos[idx].x()/__L_UNIT;
        fEvPart_Y[idx] = ev->fPartPos[idx].y()/__L_UNIT;
        fEvPart_Z[idx] = ev->fPartPos[idx].z()/__L_UNIT;

        fEvPart_Px[idx] = ev->fPartRealMom[idx].x()/__E_UNIT;
        fEvPart_Py[idx] = ev->fPartRealMom[idx].y()/__E_UNIT;
        fEvPart_Pz[idx] = ev->fPartRealMom[idx].z()/__E_UNIT;
        fEvPart_Th[idx] = ev->fPartRealMom[idx].theta();
        fEvPart_Ph[idx] = ev->fPartRealMom[idx].phi();

        fEvPart_P[idx] = ev->fPartRealMom[idx].mag()/__E_UNIT;

        fEvPart_tPx[idx] = ev->fPartMom[idx].x()/__E_UNIT;
        fEvPart_tPy[idx] = ev->fPartMom[idx].y()/__E_UNIT;
        fEvPart_tPz[idx] = ev->fPartMom[idx].z()/__E_UNIT;
    }

    /////////////////////////////////////////////////
    //  Set beam data as well

    const remollBeamTarget* bt = ev->GetBeamTarget();

    fBmX = bt->fVer.x()/__L_UNIT;
    fBmY = bt->fVer.y()/__L_UNIT;
    fBmZ = bt->fVer.z()/__L_UNIT;

    fBmdX = bt->fDir.x();
    fBmdY = bt->fDir.y();
    fBmdZ = bt->fDir.z();
    fBmth = bt->fDir.theta();
    fBmph = bt->fDir.phi()/deg;
}

// GenericDetectorHit

void remollIO::AddGenericDetectorHit(remollGenericDetectorHit *hit)
{
    int n = fNGenDetHit;
    if( n > __IO_MAXHIT ){
        G4cerr << "WARNING: " << __PRETTY_FUNCTION__ << " line " << __LINE__ << ":  Buffer size exceeded!" << G4endl;
        return;
    }

    fGenDetHit_det[n]  = hit->fDetID;
    fGenDetHit_id[n]   = hit->fCopyID;

    fGenDetHit_trid[n] = hit->fTrID;
    fGenDetHit_mtrid[n]= hit->fmTrID;
    fGenDetHit_pid[n]  = hit->fPID;
    fGenDetHit_gen[n]  = hit->fGen;

    fGenDetHit_X[n]  = hit->f3X.x()/__L_UNIT;
    fGenDetHit_Y[n]  = hit->f3X.y()/__L_UNIT;
    fGenDetHit_Z[n]  = hit->f3X.z()/__L_UNIT;
    fGenDetHit_R[n]  = sqrt(hit->f3X.x()*hit->f3X.x()+hit->f3X.y()*hit->f3X.y())/__L_UNIT;
    fGenDetHit_Ph[n] = hit->f3X.phi()/deg;

    fGenDetHit_Px[n]  = hit->f3P.x()/__E_UNIT;
    fGenDetHit_Py[n]  = hit->f3P.y()/__E_UNIT;
    fGenDetHit_Pz[n]  = hit->f3P.z()/__E_UNIT;

    fGenDetHit_Vx[n]  = hit->f3V.x()/__L_UNIT;
    fGenDetHit_Vy[n]  = hit->f3V.y()/__L_UNIT;
    fGenDetHit_Vz[n]  = hit->f3V.z()/__L_UNIT;

    fGenDetHit_P[n]  = hit->fP/__E_UNIT;
    fGenDetHit_E[n]  = hit->fE/__E_UNIT;
    fGenDetHit_M[n]  = hit->fM/__E_UNIT;

    fNGenDetHit++;

    // for collimator cut
    if( (hit->fDetID==200 && hit->f3X.perp()/__L_UNIT < 0.03) ||
        (hit->fDetID==201 && hit->f3X.perp()/__L_UNIT < 0.05) )
        fCollCut=0;
}


// GenericDetectorSum

void remollIO::AddGenericDetectorSum(remollGenericDetectorSum *hit)
{
    int n = fNGenDetSum;
    if( n > __IO_MAXHIT ){
        G4cerr << "WARNING: " << __PRETTY_FUNCTION__ << " line " << __LINE__ << ":  Buffer size exceeded!" << G4endl;
        return;
    }

    fGenDetSum_edep[n] = hit->fEdep/__E_UNIT;
    fGenDetSum_det[n]  = hit->fDetID;
    fGenDetSum_id[n]   = hit->fCopyID;

    fNGenDetSum++;
}

/*---------------------------------------------------------------------------------*/

void remollIO::GrabGDMLFiles(G4String fn)
{
    // Reset list
    fGDMLFileNames.clear();

    remollRunData *rundata = remollRun::GetRunData();
    rundata->ClearGDMLFiles();

    SearchGDMLforFiles(fn);


    // Store filename

    // Copy into buffers
    for(unsigned int idx = 0; idx < fGDMLFileNames.size(); idx++ ){
        G4cout << "Found GDML file " << fGDMLFileNames[idx] << G4endl;
        rundata->AddGDMLFile(fGDMLFileNames[idx]);
    }
}

void remollIO::SearchGDMLforFiles(G4String fn)
{
    /*!  Chase down files to be included by GDML.
     *   Mainly look for file tags and perform recursively */

    struct stat thisfile;

    int ret = stat(fn.data(), &thisfile);
    if( ret != 0 ){
        G4cerr << "ERROR opening file " << fn <<  " in " << __PRETTY_FUNCTION__ << ": " << strerror(errno) << G4endl;
        exit(1);
    }

    fGDMLFileNames.push_back(fn);


    xercesc::XMLPlatformUtils::Initialize();

    xercesc::XercesDOMParser *xmlParser = new xercesc::XercesDOMParser();
    xmlParser->parse(fn.data());
    xercesc::DOMDocument* xmlDoc = xmlParser->getDocument();
    xercesc::DOMElement* elementRoot = xmlDoc->getDocumentElement();

    TraverseChildren( elementRoot );

    xercesc::XMLPlatformUtils::Terminate();

    delete xmlParser;
}

void remollIO::TraverseChildren( xercesc::DOMElement *thisel )
{
    xercesc::DOMNodeList* children = thisel->getChildNodes();
    const XMLSize_t nodeCount = children->getLength();

    for( XMLSize_t xx = 0; xx < nodeCount; ++xx ){
        xercesc::DOMNode* currentNode = children->item(xx);
        if( currentNode->getNodeType() ){   // true is not NULL

            if (currentNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) { // is element
                xercesc::DOMElement* currentElement
                  = dynamic_cast< xercesc::DOMElement* >( currentNode );
                // transcode
                XMLCh* str_file = xercesc::XMLString::transcode("file");
                if( xercesc::XMLString::equals(currentElement->getTagName(), str_file)){
                    // transcode
                    XMLCh* str_name = xercesc::XMLString::transcode("name");
                    const XMLCh* attr = currentElement->getAttribute(str_name);
                    char* str_attr = xercesc::XMLString::transcode(attr);
                    // search files
                    SearchGDMLforFiles(G4String(str_attr));
                    // release
                    xercesc::XMLString::release(&str_name);
                    xercesc::XMLString::release(&str_attr);
                }
                // release
                xercesc::XMLString::release(&str_file);

                if( currentElement->getChildNodes()->getLength() > 0 ){
                    TraverseChildren( currentElement );
                }
            }
        }
    }
}
